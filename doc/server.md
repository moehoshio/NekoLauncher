# Server

The "res" version and the "core" version are separate. In most cases, we use JSON for data interaction.

1. testing

   - `/testing/ping`
     - Tests connectivity, generally should not have restrictions
     - Custom return content, typically HTTP code 200 indicates success
2. api

   - get
     - `/api/maintenance` : get

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
     - `/api/checkUpdates` : post

       - post :

       ```json
       {
           "core":"v0.0.1",
           "res":"v0.0.0.1",
           "token":"" // opt
       }
       ```
       - If there are no updates, return code 204. If an error occurs, return code 400. Otherwise, return code 200 along with:

       ```json
       {
           "title":"",
           "msg":"",
           "poster":"url",
           "time":"",
           "url":[]
       }
       ```
3. get

   - download
     - `/download?meta=$file`
       - Uses internal rewrite and checks authorization (if any)
