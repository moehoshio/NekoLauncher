# Server

## Definition

1. "coreVersion": This represents the version of NekoLc.
2. "resourceVersion": This represents the version of any resources managed, maintained, or upgraded by NekoLc.

## Protocol

1. In most cases, we use JSON for data interaction.
2. The client and server must include the header: "Content-Type: application/json".
3. If authentication is required, include the header Authorization: Bearer {token} in the request.
4. Standard error response format:

    ```json
    {
        "errors": {
            "error": "ForClientError", // e.g, ForClientError, ForServerError or ForUnknownError
            "errorType": "string", // e.g., "InvalidRequest", "NotFound", "InternalError"
            "errorMessage": "Error message describing the issue"
        }
    }
    ```

5. HTTP status codes used:

    - 200: Success, the request was processed successfully
    - 204: Request successful, no content to return
    - 206: Partial content returned successfully
    - 400: Client error, invalid request or format error, etc.
    - 401: Unauthorized, valid authentication credentials required
    - 429: Too many requests, try again later
    - 500: Server error, internal error
    - 501: Method not supported, should be treated as a client error

## Api

1. testing

    - `/v0/testing/ping`
        - Tests connectivity, generally should not have restrictions
        - Custom return content, typically HTTP code 200 indicates success

    - `/v0/testing/echo` : post
        - Post any content, and the server will return the same content.

2. api

    1. Account : optional

    - Account authentication is an optional API implementation. If account authentication is not used, it is recommended that checkUpdates automatically generates temporary links with tokens.
    - If account authentication is used, the Authorization: Bearer {token} header must be included in every request, otherwise a 401 response is returned.

        - `/v0/api/auth/login` : post , optional
            - Obtain access token
                - post:
                    - Authentication with account and password (requires account system)

                        ```json
                        {
                            "auth": {
                                "username": "string",
                                "password": "string"
                            }
                        }
                        ```

                    - Authentication using a unique identifier to generate a hash value

                        ```json
                        {
                            "auth": {
                                "identifier": "string",
                                "timestamp": "UTCZ Timestamp",
                                "signature": "hash"
                            }
                        }
                        ```

                    - If the account system is not implemented, return HTTP 501
                    - If the account system is implemented but authentication fails, return HTTP 401

                - response:

                    ```json
                    {
                        "accessToken": "string",
                        "refreshToken": "string"
                    }
                    ```

                    - **About refreshToken validity:**
                    - When a new refreshToken is obtained, it is recommended to immediately invalidate the previous refreshToken to enhance security and prevent reuse of old tokens.
                    - If multi-device login is required, consider allowing multiple refreshTokens to exist in parallel, but each refreshToken should have its own validity period. This can be a fixed time (e.g., 30 days), or based on the last usage timestamp (e.g., expires 15 days after last use).

        - `/v0/api/auth/refresh` : post , optional
            - Obtain a new accessToken using refreshToken
            - post:

                ```json
                {
                    "refreshToken": "string"
                }
                ```

                - If the account system is not implemented, return HTTP 501
                - If the refreshToken is invalid/expired, return HTTP 401

            - response:

                ```json
                {
                    "accessToken": "string"
                }
                ```

        - `/v0/api/auth/logout` : post, optional
            - Immediately invalidate accessToken and refreshToken
            - post:

                ```json
                {
                    "logout": {
                        "accessToken": "string",
                        "refreshToken": "string"
                    }
                }
                ```

            - response: 204 (No Content) for success, 500 for server error
            - If the account system is not implemented, return HTTP 501

    2.Api

    - `/v0/api/maintenance` : post  
        - post:

            ```json
            {
                "queryMaintenance": {
                    "os": "string",
                    "arch": "string",
                    "language": "string",
                    "coreVersion": "string (optional)",
                    "resourceVersion": "string (optional)"
                }
            }
            ```

            - os: windows, osx, linux, or custom
            - arch: system architecture (e.g., x64, arm64)
            - language: preferred language
            - coreVersion/resourceVersion: (optional) specify if maintenance info should be version-specific
            - If the server is not in maintenance mode, return code 204 (No Content)

        - response:

            ```json
            {
                "maintenanceInformation": {
                    "status" : "string", // scheduled , progress
                    "message": "string",
                    "startTime": "ISO 8601 format",
                    "exEndTime": "ISO 8601 format",
                    "posterUrl": "url",
                    "time": "ISO 8601 format",
                    "link": "url"
                }
            }
            ```

            - status: "scheduled", "progress"
                - "scheduled": Maintenance is planned but not yet started.
                - "progress": Maintenance is currently underway.
            - link: Link for users to view the maintenance announcement

    - `/v0/api/checkUpdates` : post

        - post :

            ```json
            {
                "checkUpdate" : {
                    "os" : "string",
                    "arch": "string",
                    "coreVersion" : "string",
                    "resourceVersion" : "string",
                    "language" : "string"
                }
            }
            ```

            - If there are no updates, return code 204.
            - If the server is in maintenance mode, return code 503 (Service Unavailable) with maintenance information. (This applies to cases where the maintenance API and update API are not strongly consistent, and the update API is under maintenance. Specifically, if the maintenance status is checked before checking for updates, there is no need to handle separate maintenance status for the update API.)
            - If parsing is successful, return code 200 along with:

        - response:

            ```json
            {
                "updateInformation" : {
                    "title": "string",
                    "description": "string",
                    "posterUrl": "url",
                    "publishTime": "ISO 8601 format",
                    "resourceVersion": "string",  // If the resource version has been updated, this field should be present. After the update, the client should store this version.
                    "isMandatory": true,
                    "files":[
                        {
                            "url":"url",
                            "fileName":"string",
                            "checksum":"string",
                            "downloadMeta":{
                                "hashAlgorithm": "sha256",// md5 , sha1 ,sha256 ,sha512
                                "multiPart":false,// use multis thread download?
                                "isCoreFile":false,
                                "isAbsoluteUrl":false // if not absolute url , an use current host.
                            }
                        },
                        {
                            //....
                        }
                    ]
                }
            }
            ```

            - resourceVersion: If this update does not involve a resource version, this key can be absent or an empty string.
            - If the main program (i.e., Nekolc core, including libraries) needs to be updated, The main program, and main libraries should be included in the URL. The update program is then run, and main program exits.
            - The update program will update the main program and files by replacing them with the already downloaded versions, and then it will launch the main program.
            - If only resources need to be updated, the update is completed as soon as the download finishes.

    - `/v0/api/feedbackLog` : post

        - post:

            ```json
            {
                "feedbackLog" : {
                    "os": "string",
                    "arch": "string",
                    "coreVersion": "string",
                    "resourceVersion": "string",
                    "timestamp": "UTCZ Timestamp",
                    "content": "string"
                }
            }
            ```

            - Return 204 for success, 400 for client error, 500 for server error.
            - For example, if either the core or resource version is a non-existent version, return a client error.
