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
            "resVersion":"",//If the version has been updated
            "update":[
                {
                    "url":"url1",
                    "name":"name1",
                    "hash":"hash1",
                    "meta":{
                        "hashAlgorithm": "sha256",// md5 , sha1 ,sha256 ,sha512
                        "multis":false,// use multis download?
                        "temp":false,//in temp dir? Enabling this option will pass the file name to the update program and copy it to the root folder. This usually indicates a core update.
                        "randName":false,//use rand name? Using this option allows the "name" key value to be an empty string.
                        "absoluteUrl":false// if not absolute url , an use current host.
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
                        "absoluteUrl":true
                    }
                }
            ],
            "mandatory": true
        }
        ```

        - If the main program (i.e., Nekolc core, including libraries) needs to be updated, a separate temporary update program will be downloaded. The main program, update program, and main libraries should be included in the URL. The temporary program is then run, and the main program exits.
        - The temporary update program will update the main program and files by replacing them with the already downloaded versions, and then it will launch the main program.
        - If only resources need to be updated, the update is completed as soon as the download finishes.

The server can be implemented in any language. Below is an example of the above API implemented using PHP:
- `/v1/api/maintenance` :

    ```php
    <?php
    $putData = array(
        "enable" => false,
        "msg" => "msg",
        "poster" => "https://example.com/img/img1.png",
        "time" => "2024/01/01 00:00(UTC+01) - 2024/12/01 23:59(UTC+01)",
        "annctLink" => "https://example.com"
    );
    $putJsonData = json_encode($putData);
    header('Content-Type: application/json');
    if (json_last_error() === JSON_ERROR_NONE) {
        echo $putJsonData;
    } else {
        http_response_code(500);
    }
    ?>
    ```

- `/v1/api/checkUpdates` :

    ```php
    <?php

    $jsonData = file_get_contents('php://input');
    $data = json_decode($jsonData, true);

    $putData = array(
        "title"=>"Release v0.0.0.2",
        "msg"=>"msg",
        "poster"=>"https://example.com/img/img2.png",
        "time"=>"2024-01-01 10:00(UTC+01)",
        "resVersion"=>"v1.0.0.1",
        "mandatory"=>true,
        "update"=>array(
            array("url"=>"https://data.example.com/download/test","name"=>"temp/test.txt","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>false,"temp"=>false,"randName"=>false,"absoluteUrl"=>true)
            ),
            array("url"=>"/api/getInfo?meta=config","name"=>"config.ini","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>false,"temp"=>true,"randName"=>false,"absoluteUrl"=>false)
            ),
            array("url"=>"https://example.com/100MB.bin","name"=>"","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>true,"temp"=>false,"randName"=>true,"absoluteUrl"=>true)
            ),
        )
        );
    $putJsonData = json_encode($putData);

    header('Content-Type: application/json;');

    if (json_last_error() === JSON_ERROR_NONE) {
    $var = $data["core"];
    switch ($var) {
        case 'v0.0.0.1':
            echo $putJsonData;
            http_response_code(200);
            break;
        case 'v0.0.0.2':
            http_response_code(204);
            break;
        default:
            http_response_code(400);
            break;
    }

    } else {
        http_response_code(400);
    }
    ?>

    ```
