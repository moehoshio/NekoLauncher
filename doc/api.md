# Api

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
        "errors": [
            {
                "error": "ForClientError", // e.g., ForClientError, ForServerError, or ForUnknownError...
                "errorType": "string", // e.g., "InvalidRequest", "NotFound", "InternalError"...
                "errorMessage": "A human-readable error message describing the issue"
            },
            {
                // ...
            }
        ]
        // other content ... if any
    }
    ```

5. HTTP status codes:  
    These represent the HTTP status codes that should be used in NekoLc, but not all APIs are required to use only these codes, nor are these the only possible codes that may be returned. For example, reverse proxy servers or CDNs may return other status codes. However, within NekoLc, these codes should be considered standard.

    - 200: Success, the request was processed successfully
    - 204: Request successful, no content to return
    - 206: Partial content returned successfully
    - 400: Client error, invalid request or format error, etc.
    - 401: Unauthorized, valid authentication credentials required
    - 404: Not found
    - 429: Too many requests, try again later
    - 500: Server error, internal error
    - 501: Method not supported, should be treated as a client error
    - 503: Service unavailable, the service is currently unavailable, such as during maintenance

## Meta

API meta information should be included in every API response, with the following structure:

```json
{
    "meta": {
        "apiVersion": "string",
        "minApiVersion": "string", // minimum required API version
        "buildVersion": "string",
        "timestamp": "UTCZ Timestamp", // server time
        "releaseDate": "ISO 8601 format",
        "deprecated": false,
        "deprecatedMessage": "string" // if deprecated, provide deprecation info
    }
}
```

## preferences

The `preferences` field is used to pass user preferences, such as language settings. It should be included in the request body of APIs that support it. The structure is as follows:

```json
{
    "preferences": {
        "language": "string",
        "telemetry": true
    }
}
```

For example, if supported, error messages can also be returned according to the preferred language.

## Apis

1. testing

    - `/v0/testing/ping`
        - Tests connectivity, generally should not have restrictions
        - Custom return content, typically HTTP code 200 indicates success

    - `/v0/testing/echo` : post , only debug
        - Post any content, and the server will return the same content.
        - Optional: Require verification of whether the authentication token header is correct and whether the format (such as JSON) is valid.
        - Note: This API should only be used in debug mode and must not be available in production environments.

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
                            },
                            "preferences": {
                                // Preferences for the user
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
                            },
                            "preferences": {
                                // Preferences for the user
                            }
                        }
                        ```

                    - If the account system is not implemented, return HTTP 501
                    - If the account system is implemented but authentication fails, return HTTP 401

                - response:

                    ```json
                    {
                        "accessToken": "string",
                        "refreshToken": "string",
                        "meta": {
                            // Api meta information
                        }
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
                    "accessToken": "string",
                    "meta": {
                        // Api meta information
                    }
                }
                ```

        - `/v0/api/auth/validate` : post , optional
            - Validate the accessToken
            - post:

                ```json
                {
                    "accessToken": "string"
                }
                ```

            - response
                - 204 (No Content) for valid accessToken, 401 for invalid/expired accessToken

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

    2.Launcher

    - `/v0/api/launcherConfig` : post
        - Obtain the configuration of the launcher
        - post:

            ```json
            {
                "config": {
                    "os": "string",
                    "arch": "string",
                    "coreVersion": "string (optional)",
                    "resourceVersion": "string (optional)"
                },
                "preferences": {
                    // Preferences for the user
                }
            }
            ```

        - response:

            ```json
            {
                "launcherConfig": {
                    "host": [
                        "string"
                    ],
                    // If WebSocket support is available
                    "webSocket": {
                        "enable": true,
                        "socketHost": "string", 
                        "heartbeatIntervalSec": "number",
                    },
                    "retryIntervalSec": "number",
                    "maxRetryCount": "number",
                    // If authentication is required
                    "security": {
                        "enableAuthentication": true,
                        "tokenExpirationSec": "number",
                        "refreshTokenExpirationDays": "number",
                        "loginUrl": "string",
                        "logoutUrl": "string",
                        "refreshUrl": "string"
                    },
                    // Extension...
                    "featuresFlags": {
                        "ui": {
                            "enableDevHint": true,
                        },
                        "enableFeatureA": true,
                        "enableFeatureB": false
                    }
                },
                "meta": {
                    // Api meta information
                }
            }
            ```

            - The `featuresFlags` field can be extended as needed. These fields can be used to control the enabling or disabling of client features.

    - `/v0/api/maintenance` : post
        - Check if the `service` is in maintenance mode
        - post:

            ```json
            {
                "checkMaintenance": {
                    "os": "string",
                    "arch": "string",
                    "coreVersion": "string (optional)",
                    "resourceVersion": "string (optional)"
                },
                "preferences": {
                    // Preferences for the user
                }
            }
            ```

            - os: windows, osx, linux, or custom
            - arch: system architecture (e.g., x64, arm64)
            - language: preferred language
            - coreVersion/resourceVersion: (optional) specify if maintenance info should be version-specific
            - If the service is not in maintenance mode, return code 204 (No Content)

        - response:

            ```json
            {
                "maintenanceInformation": {
                    "status" : "string", // scheduled , progress
                    "message": "string",
                    "startTime": "ISO 8601 format",
                    "exEndTime": "ISO 8601 format",
                    "posterUrl": "url",
                    "link": "url"
                },
                "meta": {
                    // Api meta information
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
                    "coreVersion": "string",
                    "resourceVersion": "string"
                },
                "preferences": {
                    // Preferences for the user
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
                    "files": [
                        {
                            "url": "url",
                            "fileName": "string",
                            "checksum": "string",
                            "downloadMeta": {
                                "hashAlgorithm": "sha256", // md5 , sha1 ,sha256 ,sha512
                                "suggestMultiThread": false,
                                "isCoreFile": false,
                                "isAbsoluteUrl": false // if not absolute url , an use current host.
                            }
                        },
                        {
                            //....
                        }
                    ]
                },
                "meta": {
                    // Api meta information
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
                },
                "preferences": {
                    // Preferences for the user
                }
            }
            ```

            - Return 204 for success, 400 for client error, 500 for server error.
            - For example, if either the core or resource version is a non-existent version, return a client error.

### WebSocket

In the API, the use of WebSocket is optional.
Whether it is enabled, the connection host, and other configurations are returned by the configuration API `/v0/api/launcherConfig`.
The server can send update and maintenance notifications, and the client can report feedback information.
This ensures real-time communication, preventing situations where a new version is released right after an update check.

We only define the protocol and format for the WebSocket API, and the server can implement it as needed.

Server-side WebSocket API should follow the following protocol:

```json
{
    "action": "string", // "ping", "pong", "notify"    
    "messageId": "string", // optional, used for message history compensation
    "notifyChanged": {
        "type": "string", // "update", "maintenance"
        "os": "string",
        "arch": "string",
        "coreVersion": "string",
        "resourceVersion": "string",
        "message": "string"
    },
    // If there is an error, the server returns this field
    "errors": [
        // Standard error response format
    ],
    "meta": {
        // Api meta information
    }
}
```

Client-side WebSocket API should follow the following protocol:

```json
{
    "action": "string", // "ping", "pong"
    "accessToken": "string", // optional, if authentication is enabled
    "lastMessageId": "string", // optional, used for message history compensation
    "clientInfo": {
        "os": "string",
        "arch": "string",
        "coreVersion": "string",
        "resourceVersion": "string"
    },
    "preferences": {
        // Preferences for the user
    }
}
```

If the WebSocket connection is disconnected, the client should be fully restarted. Otherwise, the server should implement a message queue or history compensation mechanism, such as using fields like `lastMessageId`.

If authorization fails, a 401 should be returned immediately during the handshake phase.  
If authorization expires after the connection is established, the connection should be closed.

### Static

Some features support static deployment on the server side, but there are certain limitations:

- No authentication is possible.
- Incremental updates are not supported (each update requires a full download).
- No multilingual message support (including update notifications, maintenance information, etc.)
- No feedback mechanism.
- No differentiated adjustments; it is not possible to dynamically adjust based on client status, region, version, etc.
- For example, it is not possible to maintain only specific versions or specific clients.

For static deployment, we only define the API protocol format.

- Remote configuration URL: GET
- response:

    ```json
    {
        "launcherConfig": {
            "checkUpdateUrls": {
                "<os>-<arch>": "url" // e.g., "windows-x64": "https://example.com/update/windows-x64.json"
            },

            // Same format as the launcherConfig in the API
        },
        "maintenanceInformation": {
            "status": "string", // scheduled, progress, completed
            // Other fields are the same as in the maintenance check format
        }
    }
    ```

- Check update URL: GET
- response:

    ```json
    {
        "coreVersion": "string",
        "resourceVersion": "string",
        "updateInformation": {
            // Same format as the update check
        }
    }
    ```
