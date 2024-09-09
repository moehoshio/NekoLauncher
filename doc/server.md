# Server

The "res" version and the "core" version are separate. In most cases, we use JSON for data interaction.

standard error message:

```json
{
    "errType":"",
    "msg":"",
    "details": "",
    "traceId": ""
}
```

1. testing
    - `/v1/testing/ping`
        - Tests connectivity, generally should not have restrictions
        - Custom return content, typically HTTP code 200 indicates success
2. api

    - `/v1/api/maintenance` : get

        - Returns maintenance information

        ```json
        {
            "enable": true,
            "msg": "",
            "poster": "url",
            "time": "",
            "annctLink": "url"
        }
        ```

    - `/v1/api/checkUpdates` : post

        - post :

        ```json
        {
            "core":"v0.0.0.1",
            "res":"v0.0.0.1",
            "token":"" // opt
        }
        ```

        - If there are no updates, return code 204  
        - If a client error occurs (e.g., the incoming JSON cannot be parsed, or there are key-value errors), return HTTP code 400 and standard error message  
        - If a server error occurs, return HTTP code 500 and standard error message  
        - If parsing is successful, return code 200 along with:

        ```json
        {
            "title":"",
            "msg":"",
            "poster":"url",
            "time":"",
            "update":[
                {
                    "url":"url1",
                    "name":"name1",
                    "hash":"hash1",
                    "meta":{
                        "hashAlgorithm": "sha256",
                        "multis":false,// use multis download?
                        "temp":false,//in temp dir?
                        "randName":false,//use rand name?
                        "absoluteUrl":false,// if not absolute url , an use current host.
                        "isUpdateProgram":false
                    }
                },
                {
                    "url":"url2",
                    "name":"",
                    "hash":"hash2",
                    "meta":{
                        "hashAlgorithm": "sha256",
                        "multis":true,
                        "temp":false,
                        "randName":true,
                        "absoluteUrl":true,
                        "isUpdateProgram":true
                    }
                }
            ],
            "mandatory": true
        }
        ```

        - If the main program (i.e., Nekolc core, including libraries) needs to be updated, a separate temporary update program will be downloaded. The main program, update program, and main libraries should be included in the URL. The temporary program is then run, and the main program exits.
        - The temporary update program will update the main program and files by replacing them with the already downloaded versions, and then it will launch the main program.
        - If only resources need to be updated, the update is completed as soon as the download finishes.
