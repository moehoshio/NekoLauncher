# Server

## Definition

1. "coreVersion": This represents the version of NekoLc.
2. "resourceVersion": This represents the version of any resources managed, maintained, or upgraded by NekoLc.

## Protocol

1. In most cases, we use JSON for data interaction.
2. The client and server need the header: "Content-Type: application/json"


## Api

1. testing
    - `/v1/testing/ping`
        - Tests connectivity, generally should not have restrictions
        - Custom return content, typically HTTP code 200 indicates success
2. api

    - `/v1/api/maintenance` : post
        - post:
        
        ```json
        {
            "queryMaintenance":{
                "os" : "string",
                "language" : "string",
            }
        }
        ```

        - os: windows, osx, linux or custom
        - lang: preferred language
        - Returns maintenance information :

        ```json
        {
            "maintenanceInformation": {
                "enable": true,
                "message": "string",
                "poster": "url",
                "time": "string",
                "link": "url"
            }
        }
        ```

    - `/v1/api/checkUpdates` : post

        - post :

        ```json
        {
            "checkUpdate" : {
                "coreVersion" : "string",
                "resourceVersion" : "string",
                "os" : "string",
                "language" : "string"
            }
        }
        ```

        - If there are no updates, return code 204  
        - If a client error occurs (e.g., the incoming JSON cannot be parsed, or there are key-value errors), return HTTP code 400 and standard error message  
        - If a server error occurs, return HTTP code 500 and standard error message  
        - If parsing is successful, return code 200 along with:

        ```json
        {
            "updates" : {
                "title": "string",
                "message": "string",
                "poster": "url",
                "time": "string",
                "resourceVersion": "string",//If the version has been updated
                "mandatory": true,
                "download":[
                    {
                        "url":"url",
                        "name":"string",
                        "hash":"string",
                        "meta":{
                            "hashAlgorithm": "sha256",// md5 , sha1 ,sha256 ,sha512
                            "multis":false,// use multis thread download?
                            "temp":false,//in temp dir? Enabling this option will pass the file name to the update program and copy it to the root folder. This usually indicates a core update.
                            "randName":false,//use rand name? Using this option allows the "name" key value to be an empty string.
                            "absoluteUrl":false// if not absolute url , an use current host.
                        }
                    },
                    {
                        "url":"",
                        "name":"",
                        "hash":"",
                        "meta":{
                            "hashAlgorithm": "sha256",
                            "multis":true,
                            "temp":false,
                            "randName":true,
                            "absoluteUrl":true
                        }
                    }
                ]
            }
        }
        ```

        - resourceVersion: If this update does not involve a resource version, this key can be absent or an empty string.
        - If the main program (i.e., Nekolc core, including libraries) needs to be updated, a separate temporary update program will be downloaded. The main program, update program, and main libraries should be included in the URL. The temporary program is then run, and the main program exits.
        - The temporary update program will update the main program and files by replacing them with the already downloaded versions, and then it will launch the main program.
        - If only resources need to be updated, the update is completed as soon as the download finishes.

    - `/v1/api/feedbacklog` : post

        - post:

        ```json
        {
            "feedbacklog" : {
                "coreVersion": "string",
                "resourceVersion": "string",
                "os": "string",
                "timestamp": int,
                "log": "string"
            }
        }
        ```

        - Return 204 for success, 400 for client error, 500 for server error.
        - For example, if either the core or resource version is a non-existent version, return a client error.

### Implementation Example

The server can be implemented in any language. Below is an example of the above API implemented using PHP:

Note: This should not be used in production environments.

- `/v1/api/maintenance` :

    ```php
    <?php

    $os = $_GET["os"];
    $lang = $_GET["lang"];

    $putData = array(
        "enable" => false,
        "msg" => "",
        "poster" => "https://example.com/img/img1.png",
        "time" => "2024/01/01 00:00(UTC+01) - 2024/12/01 23:59(UTC+01)",
        "link" => "https://example.com/news"
    );

    switch ($lang) {
        case 'zh_tw':
            $putData["msg"] = "訊息\n訊息2";
            break;
        case 'en':
        default:
            $putData["msg"] = "msg\nmsg2";
            break;
    }

    header('Content-Type: application/json');

    switch ($os) {
        case 'osx':
            $putData["enable"] = true;
        case 'windows':
        case 'linux':
            echo json_encode($putData);
            break;
        default:
            http_response_code(400);
            break;
    }
    ?>
    ```

- `/v1/api/checkUpdates` :

    ```php
    <?php

    $jsonData = file_get_contents('php://input');
    $data = json_decode($jsonData, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
        http_response_code(400);
        exit;
    }
    $putData = array(
        "title"=>"",
        "msg"=>"",
        "poster"=>"https://example.com/img/img2.png",
        "time"=>"2024-01-01 10:00(UTC+01)",
        "resVersion"=>"v1.0.1",
        "mandatory"=>true,
        "update"=>array()
    );
    
    $lang = $data["lang"];
    switch ($lang) {
        case 'zh_tw':
            $putData["title"] = "版本發佈 v0.0.2";
            $putData["msg"] = "訊息";
            break;
        case 'en' :
        default:
            $putData["title"] = "Release v0.0.2";
            $putData["msg"] = "msg";
            break;
    }

    header('Content-Type: application/json;');

    $core = $data["core"];
    $res = $data["res"];
    $os = $data["os"];
    $urls = array();

    if($os ==="windows"){
    switch ($core) {
        case 'v0.0.1':
            $urls[] = array("url"=>"https://data.example.com/download/test","name"=>"temp/test.txt","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>false,"temp"=>false,"randName"=>false,"absoluteUrl"=>true)
            );
            $urls[]  = array("url"=>"/api/getInfo?meta=config","name"=>"config.ini","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>false,"temp"=>true,"randName"=>false,"absoluteUrl"=>false)
            );
            //...
            break;
        case 'v0.0.2':
            break;
        default:
            $urls[] = array("url"=>"https://data.example.com/download/test","name"=>"temp/test.txt","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>false,"temp"=>false,"randName"=>false,"absoluteUrl"=>true)
            );
            //You can download it completely here or trigger a client error.
            break;
    }

    switch ($res) {
        case 'v1.0.0':
            $urls[] = array("url"=>"https://example.com/100MB.bin","name"=>"","hash"=>"","meta"=>
                array("hashAlgorithm"=>"sha256","multis"=>true,"temp"=>false,"randName"=>true,"absoluteUrl"=>true)
            );
            break;
        case 'v1.0.1':
            break;
        default:
            break;
    }
    }
    if($os ==="osx"){}
    if($os ==="linux"){}
    //...

    if (empty($urls)) {
        http_response_code(204);
        exit;
    }

    $putData["update"] = $urls;
    $putJsonData = json_encode($putData);
    if (json_last_error() !== JSON_ERROR_NONE) {
        http_response_code(500);
        exit;
    }
    http_response_code(200);
    echo $putJsonData;

    ?>

    ```

- `/v1/api/feedbacklog` : 

```php
<?php
$userIP = $_SERVER['REMOTE_ADDR'];
$rateLimitFile = "logs/ratelimit/$userIP.json";
$rateLimitDuration = 10; // second

createDirectoryIfNotExists("logs/ratelimit/");


function createDirectoryIfNotExists($directory) {
    if (!file_exists($directory)) {
        mkdir($directory, 0777, true);
    }
}

function checkCoreVersion($version){
    switch ($version) {
        case 'v0.0.1':
        case 'v0.0.2':
            return true;
        default:
            return false;
    }
}

function checkResVersion($version){
    switch ($version) {
        case 'v0.0.1':
        case 'v0.0.2':
            return true;
        default:
            return false;
    }
}

function checkOs($osName){
    switch ($osName) {
        case 'osx':
        case 'windows':
        case 'linux':
            return true;
        default:
            return false;
    }
}

function handleRateLimiting($rateLimitFile, $rateLimitDuration) {
    if (file_exists($rateLimitFile)) {
        $rateLimitData = json_decode(file_get_contents($rateLimitFile), true);
        if (time() - $rateLimitData['lastRequest'] < $rateLimitDuration) {
            return true;
        }
    }
    file_put_contents($rateLimitFile, json_encode(['lastRequest' => time()]));
    return false;
}


if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    if (handleRateLimiting($rateLimitFile, $rateLimitDuration)){
        http_response_code(429); // Too Many Requests
        exit;
    }

    $data = json_decode(file_get_contents('php://input'), true);
    $core = $data['core'];
    $resVersion = $data['res'];
    $os = $data['os'];
    $time = $data['time'];
    $log = $data['log'];
    
    if (!checkOs($os) || !checkCoreVersion($core) || !checkResVersion($resVersion)) {
        http_response_code(400);
        exit;
    }
    
    $date = date('Y-m-d', $time);
    $logDirectory = "logs/nekolc/$core-res_$resVersion/$date";
    
    createDirectoryIfNotExists($logDirectory);
    
    $logFile = "$logDirectory/$userIP.json"; 
    if (file_exists($logFile)) { 
        $logData = json_decode(file_get_contents($logFile), true); 
    } else { 
        $logData = []; 
    } 
    $logData[$time] = $log;
    
    file_put_contents($logFile, json_encode($logData));
    http_response_code(204);
    }
    ?>

```
