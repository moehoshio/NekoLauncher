# Server

The "game" version (or your own content) and the "core" version are separate.  
In most cases, we use JSON for data interaction.

1. testing
    - `/testing/ping`
        - Tests connectivity, generally should not have restrictions
        - Custom return content, typically HTTP code 200 indicates success
2. api
    - get
        - `/api/get?info=maintenance`
            - Returns maintenance information

            ```json
            {
                "enable": true,
                "msg": "",
                "poster": "url",
                "time": 1700000000, // anticipated completion time
                "annctLink": "url"
            }
            ```

        - `/api/get?token=$versionKey` : opt
            - Returns a temporary token
            - No specific format required, e.g., a-Z, 0-9, 64 characters

            ```json
            {
                "token": ""
            }
            ```

        - `/api/get?info=version`
            - Returns the version number as the lowest digit, e.g., v1.0.01 = 1001, v0.0.01 = 1

            ```json
            {
                "version": 1
            }
            ```

        - `/api/get?info=core or game`
            - Returns the latest version information

            ```json
            {
                "title": "",
                "msg": "",
                "poster": "url",
                "version": 1,
                "time": 17000000000, // the timestamp of this update
                "miniVersion": 1, // the minimum incremental update
                "update": {
                    "core": {
                        "url": "", // You are free to design the API here as you see fit.
                        "size": "",
                        "hash": ""
                    },
                    "game": {
                        "full": {
                            "url": [],
                            "size": [],
                            "hash": []
                        },
                        "diff": {
                            "url": [],
                            "size": [],
                            "hash": []
                        }
                    }
                }
            }
            ```

3. get
    - download
        - `/get/download?meta=$file`
            - Uses internal rewrite and checks authorization (if any)
