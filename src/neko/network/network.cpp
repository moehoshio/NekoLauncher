// Neko Module

#include "neko/network/network.hpp"

#include "neko/schema/nekodefine.hpp"

#include "neko/log/nlog.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

// libcurl
#include <curl/curl.h>

// C++ STL
#include <string>
#include <vector>

#include <chrono>
#include <future>
#include <memory>
#include <optional>

#include <algorithm>
#include <unordered_map>

#include <filesystem>
#include <fstream>

#if defined(_WIN32)
#include <winreg.h>
#endif

namespace neko {
    
    namespace network {

        //=================================================
        // NetworkBase Implementation
        //=================================================

        std::optional<std::string> NetworkBase::getSysProxy() {
            auto proxy = std::getenv("http_proxy");
            auto tlsProxy = std::getenv("https_proxy");

#if defined(_WIN32)
            if (!proxy) {
                HKEY hKey;
                LPCSTR lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
                if (RegOpenKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    DWORD dwSize = 0;
                    DWORD dwType = 0;
                    RegQueryValueExA(hKey, "ProxyServer", 0, &dwType, 0, &dwSize);
                    if (dwSize > 0) {
                        char *proxyServer = new char[dwSize];
                        RegQueryValueExA(hKey, "ProxyServer", 0, &dwType, (LPBYTE)proxyServer, &dwSize);
                        std::string res(proxyServer);
                        delete[] proxyServer;
                        if (res.find("http") == std::string::npos)
                            res.insert(0, "http://");
                        return res;
                    }
                    RegCloseKey(hKey);
                }
            }
#endif

            if (tlsProxy)
                return std::string(tlsProxy);
            else if (proxy)
                return std::string(proxy);

            return std::nullopt;
        }

        void NetworkBase::logError(const std::string &message) {
            nlog::Err(FI, LI, "Network Error: %s", message.c_str());
        }

        void NetworkBase::logInfo(const std::string &message) {
            nlog::Info(FI, LI, "Network: %s", message.c_str());
        }

        void NetworkBase::logWarn(const std::string &message) {
            nlog::Warn(FI, LI, "Network: %s", message.c_str());
        }

        void NetworkBase::logDebug(const std::string &message) {
            bool dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
                 debug = exec::getConfigObj().GetBoolValue("dev", "debug", false);
            if (dev && debug) {
                nlog::Debug(FI, LI, "Network [Debug]: %s", message.c_str());
            }
        }

        std::future<void> NetworkBase::initialize() {

            std::string proxy = exec::getConfigObj().GetValue("net", "proxy", "true");
            // "" or true or proxyAdd, otherwise set ""
            bool proxyUnexpected = exec::allTrue((proxy != ""), (proxy != "true"), !exec::isProxyAddress(proxy));
            if (proxyUnexpected)
                proxy = "";

            bool dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
                 tls = exec::getConfigObj().GetBoolValue("dev", "tls", true);

            NetworkBase::globalConfig = {
                "NekoLc/" + info::getVersion() + " (" + info::getOsNameS() + "; Build " + std::string(build_id) + ")",
                proxy,
                NetworkBase::api.hostList[0],
                (dev == true && tls == false) ? "http://" : "https://"};

            logInfo(
                "NetworkBase::initialize() : ",
                "Proxy: " + globalConfig.proxy +
                    ", Dev: " + (dev ? "true" : "false") +
                    ", TLS: " + (tls ? "true" : "false") +
                    ", Protocol: " + globalConfig.protocol +
                    ", UserAgent: " + globalConfig.userAgent);

            return exec::getThreadObj().enqueue([]() {
                logInfo(
                    "NetworkBase::initialize() : ",
                    "Starting network test...");

                for (auto it : NetworkBase::Api::hostList) {
                    Network net;
                    std::string url = NetworkBase::buildUrl(NetworkBase::api.testing, it);

                    RequestConfig config;
                    config.url = url;
                    config.method = NetworkBase::RequestType::Get;
                    config.requestId = "testing - " + std::string(it);

                    auto result = net.executeWithRetry(config, 2, std::chrono::milliseconds(50), {200});

                    if (!result.isSuccess()) {
                        logWarn(
                            "NetworkBase::initialize() : ",
                            "Failed to test host: ", it,
                            ", statusCode: ", std::to_string(result.statusCode),
                            ", errorMessage: ", result.errorMessage,
                            ". now trying the next");
                    }

                    logInfo(
                        "NetworkBase::initialize() : ",
                        "Testing okay, host: ", it, ", statusCode: ", std::to_string(result.statusCode));
                    NetworkBase::globalConfig.host = std::string(it);
                    return;
                }
                logError(
                    "NetworkBase::initialize() : ",
                    "No available hosts for network test! Please check your network settings or try again later.");
            });
        }

        //=================================================
        // Network Implementation
        //=================================================

        Network::Network() {
            curl_global_init(CURL_GLOBAL_ALL);
        }
        Network::~Network() {
            curl_global_cleanup();
        }

        void Network::logRequestInfo(const RequestConfig &config) {

            std::string methodStr;
            switch (config.method) {
                case RequestType::Get:
                    methodStr = "GET";
                    break;
                case RequestType::Head:
                    methodStr = "HEAD";
                    break;
                case RequestType::Post:
                    methodStr = "POST";
                    break;
                case RequestType::DownloadFile:
                    methodStr = "DOWNLOAD";
                    break;
                case RequestType::UploadFile:
                    methodStr = "UPLOAD";
                    break;
                default:
                    methodStr = "UNKNOWN";
                    break;
            }
            auto sysProxy = getSysProxy();

            logInfo(
                "Network::logRequestInfo() : ",
                "Request: URL: ", config.url,
                ", Method: ", methodStr,
                ", FileName: ", config.fileName,
                ", Range: ", config.range,
                ", Resumable: ", exec::boolTo(config.resumable),
                ", UserAgent: ", exec::boolTo(config.userAgent.empty(), globalConfig.userAgent, config.userAgent),
                ", Global Protocol: ", globalConfig.protocol,
                ", Proxy: ", exec::boolTo<std::string>(config.proxy.empty(), "none", config.proxy),
                ", SysProxy: ", sysProxy ? "none" : *sysProxy,
                ", ID: ", config.requestId);

            logDebug(
                "Network::logRequestInfo() : ",
                "Data: ", exec::boolTo<std::string>(config.postData.empty(), config.postData, "null"),
                ", Header: ", exec::boolTo<std::string>(config.header.empty(), config.header, "null"),
                ", ID: ", config.requestId);
        }

        std::string Network::initCurl(CURL *curl, const RequestConfig &config) {
            std::string errorMsg;
            if (!curl) {
                errorMsg = "Failed to initialize curl. ID: " + config.requestId;
                logError(
                    "Network::initCurl() : ",
                    errorMsg);
                return errorMsg;
            }

            // Set CA certificate
            std::string caPath = info::workPath() + "/cacert.pem";
            if (!std::filesystem::exists(caPath)) {
                errorMsg = "The cacert.pem file does not exist at: " + caPath + ", ID: " + config.requestId;
                logError(
                    "Network::initCurl() : ",
                    errorMsg);
                return errorMsg;
            }

            curl_easy_setopt(curl, CURLOPT_CAINFO, caPath.c_str());
            curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);

            auto systemProxy = getSysProxy();

            // proxy = "true" means use system proxy
            if (config.proxy == "true" && systemProxy && exec::isProxyAddress(*systemProxy)) {
                curl_easy_setopt(curl, CURLOPT_PROXY, (*systemProxy).c_str());
            } else if (!config.proxy.empty() && exec::isProxyAddress(config.proxy)) {
                curl_easy_setopt(curl, CURLOPT_PROXY, config.proxy.c_str());
            } else {
                curl_easy_setopt(curl, CURLOPT_PROXY, "");
                if (!config.proxy.empty() && config.proxy != "false") {
                    // If an invalid proxy address is specified, log a warning, but don't abort the request
                    logWarn(
                        "Network::initCurl() : ",
                        "Invalid proxy address: " + config.proxy + ", ID: " + config.requestId);
                }
            }

            // Handle resumable downloads
            if (config.resumable && !config.fileName.empty()) {
                std::fstream infile(config.fileName, std::ios::in | std::ios::binary);
                if (!infile.is_open()) {
                    errorMsg = "Failed to open file for resuming: " + config.fileName + ", ID: " + config.requestId;
                    logError(
                        "Network::initCurl() : ",
                        errorMsg);
                    return errorMsg;
                }

                infile.seekg(0, std::ios::end);
                size_t fileSize = infile.tellg();
                infile.close();

                // If range download, resume from the specified range (file_size + range_start)
                if (!config.range.empty()) {
                    size_t resumeOffset = fileSize + std::stoull(config.range.substr(0, config.range.find('-')));
                    curl_easy_setopt(curl, CURLOPT_RESUME_FROM, resumeOffset);
                } else {
                    curl_easy_setopt(curl, CURLOPT_RESUME_FROM, fileSize);
                }
            }

            // Set headers and range
            if (!config.header.empty())
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, config.header.c_str()));

            if (!config.range.empty())
                curl_easy_setopt(curl, CURLOPT_RANGE, config.range.c_str());

            // Set user agent and URL
            std::string userAgent = config.userAgent.empty() ? globalConfig.userAgent : config.userAgent;
            curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
            curl_easy_setopt(curl, CURLOPT_URL, config.url.c_str());
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

            return std::string();
        }

        int Network::getHttpStatusCode(CURL *curl) {
            long statusCode = 0;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
            return static_cast<int>(statusCode);
        }

        // This callback function is used to handle libcurl debug messages, called only in Network::performRequest function.
        static size_t debugCallback(CURL *handle, curl_infotype type, char *data, size_t size, void *userptr) {
            if (type == CURLINFO_TEXT || type == CURLINFO_SSL_DATA_IN || type == CURLINFO_SSL_DATA_OUT) {
                if (userptr) {
                    static_cast<std::vector<std::string> *>(userptr)->emplace_back(std::string(data, size));
                }
            }
            return 0;
        }

        NetworkResult Network::performRequest(CURL *curl, const RequestConfig &config) {
            NetworkResult result;
            logInfo(
                "Network::performRequest() : ",
                "Starting request execution, ID: " + config.requestId);

            // Set up debug messages vector
            std::vector<std::string> curl_debug_msgs;
            curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

            curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, &debugCallback);
            curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &curl_debug_msgs);

            // Output libcurl version information
            curl_version_info_data *ver = curl_version_info(CURLVERSION_NOW);
            logDebug(
                "Network::performRequest() : ",
                "libcurl version: " + std::string(ver->version) + ", ssl version: " + std::string(ver->ssl_version));

            // Execute request
            CURLcode res = curl_easy_perform(curl);

            // Get connection IP
            char *ip = nullptr;
            curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);

            // Set status code (whether success or failure)
            result.statusCode = getHttpStatusCode(curl);

            if (res != CURLE_OK) {
                // Handle error
                std::string basicMsg = "Failed to get network req: " +
                                       std::string(curl_easy_strerror(res)) +
                                       ", ID: " + config.requestId +
                                       ", IP: " + (ip ? ip : "N/A");

                // Build detailed error message
                std::string detailMsg = basicMsg;

                // Add debug messages
                for (const auto &dbg : curl_debug_msgs) {
                    detailMsg += "\n[Debug] " + dbg;
                }

                // Add certificate information
                struct curl_certinfo *ci = nullptr;
                if (curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ci) == CURLE_OK && ci) {
                    for (int i = 0; i < ci->num_of_certs; i++) {
                        struct curl_slist *slist = ci->certinfo[i];
                        for (; slist; slist = slist->next) {
                            detailMsg += "\n[CERT] ";
                            detailMsg += slist->data;
                        }
                    }
                }

                logError(
                    "Network::performRequest() : ",
                    basicMsg);
                logDebug(
                    "Network::performRequest() : ",
                    detailMsg);
                result.setError(basicMsg, detailMsg);
                return result;
            }

            logInfo(
                "Network::performRequest() : ",
                "Request executed successfully, ID: " + config.requestId);
            return result;
        }

        NetworkResult Network::handleGetRequest(CURL *curl, const RequestConfig &config) {
            std::string content;
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToStringCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

            // Execute request and get result
            NetworkResult result = performRequest(curl, config);

            // If request is successful, set content
            if (!result.hasError) {
                result.content = content;
            }

            return result;
        }

        NetworkResult Network::handleHeadRequest(CURL *curl, const RequestConfig &config) {
            std::string headerContent;
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &writeToStringCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerContent);

            // Execute request and get result
            NetworkResult result = performRequest(curl, config);

            // If request is successful, set content
            if (!result.hasError) {
                result.content = headerContent;
            }

            return result;
        }

        NetworkResult Network::handlePostRequest(CURL *curl, const RequestConfig &config) {
            std::string content;
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, config.postData.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToStringCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &content);

            // Execute request and get result
            NetworkResult result = performRequest(curl, config);

            // If request is successful, set content
            if (!result.hasError) {
                result.content = content;
            }

            return result;
        }

        NetworkResult Network::handleDownloadRequest(CURL *curl, const RequestConfig &config) {
            NetworkResult result;

            // Open file for writing
            std::fstream file(config.fileName, std::ios::out | std::ios::binary | (config.resumable ? std::ios::app : std::ios::trunc));
            if (!file.is_open()) {
                std::string errorMsg = "Failed to open file for writing: " + config.fileName + ", ID: " + config.requestId;
                logError(
                    "Network::handleDownloadRequest() : ",
                    errorMsg);
                result.setError("File operation error", errorMsg);
                return result;
            }

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeToFileCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

            // Execute request and get result
            result = performRequest(curl, config);

            return result;
        }

        NetworkResult Network::doExecute(const RequestConfig &config) {
            logRequestInfo(config);

            CURL *curl = curl_easy_init();
            NetworkResult result;

            std::string initError = initCurl(curl, config);
            if (!initError.empty()) {
                result.setError("Failed to initialize curl", initError);
                if (curl)
                    curl_easy_cleanup(curl);
                return result;
            }

            // Handle different request types
            switch (config.method) {
                case RequestType::Get:
                    result = handleGetRequest(curl, config);
                    break;

                case RequestType::Head:
                    result = handleHeadRequest(curl, config);
                    break;

                case RequestType::Post:
                    result = handlePostRequest(curl, config);
                    break;

                case RequestType::DownloadFile:
                    result = handleDownloadRequest(curl, config);
                    break;

                case RequestType::UploadFile:
                    // Upload feature not implemented yet
                    result.setError("Upload not implemented");
                    break;

                default:
                    result.setError("Unknown request type");
                    break;
            }

            curl_easy_cleanup(curl);
            return result;
        }

        NetworkResult Network::execute(const RequestConfig &config) {
            return doExecute(config);
        }

        std::future<NetworkResult> Network::executeAsync(const RequestConfig &config) {
            return exec::getThreadObj().enqueue([this, config]() {
                return this->execute(config);
            });
        }

        NetworkResult Network::executeWithRetry(const RequestConfig &config,
                                                int maxRetries,
                                                std::chrono::milliseconds retryDelay,
                                                const std::vector<int> &successCodes) {
            // Build expected status code string for logging
            std::string expectCodes;
            for (auto code : successCodes) {
                expectCodes.append(std::to_string(code) + ",");
            }

            logInfo(
                "Network::executeWithRetry() : ",
                "Retry config: Expected codes: " + expectCodes +
                    ", Delay: " + std::to_string(retryDelay.count()) +
                    ", Max attempts: " + std::to_string(maxRetries) +
                    ", ID: " + config.requestId);

            for (int attempt = 0; attempt < maxRetries; ++attempt) {
                auto result = execute(config);

                logInfo(
                    "Network::executeWithRetry() : ",
                    "Attempt " + std::to_string(attempt + 1) +
                        " status code: " + std::to_string(result.statusCode) +
                        ", ID: " + config.requestId);

                // Check for success
                for (auto code : successCodes) {
                    if (result.statusCode == code) {
                        return result;
                    }
                }

                // Wait and retry if not the last attempt
                if (attempt < maxRetries - 1) {
                    std::this_thread::sleep_for(retryDelay);
                }
            }

            // All attempts failed
            NetworkResult failResult;
            failResult.setError("All retry attempts failed",
                                "All retry attempts failed for request ID: " + config.requestId +
                                    ", Expected codes: " + expectCodes +
                                    ", Last status code: " + std::to_string(failResult.statusCode));
            return failResult;
        }

        std::optional<size_t> Network::getContentSize(const std::string &url) {
            RequestConfig config;
            config.url = url;
            config.method = RequestType::Head;

            auto result = execute(config);
            if (result.hasError || result.statusCode != 200) {
                return std::nullopt;
            }

            // Find Content-Length in headers
            std::string headers = result.content;
            std::transform(headers.begin(), headers.end(), headers.begin(), ::tolower);

            std::size_t pos = headers.find("content-length:");
            if (pos == std::string::npos) {
                return std::nullopt;
            }

            std::size_t start = pos + 15; // Skip "content-length:"
            std::size_t end = headers.find_first_of("\r\n", start);
            std::string sizeStr = headers.substr(start, end - start);

            // Trim whitespace
            sizeStr.erase(0, sizeStr.find_first_not_of(" \t"));
            sizeStr.erase(sizeStr.find_last_not_of(" \t") + 1);

            try {
                return std::stoull(sizeStr);
            } catch (const std::invalid_argument &e) {
                logError(
                    "Network::getContentSize() : ",
                    "Invalid Content-Length value: " + sizeStr + " - " + e.what());
            } catch (const std::out_of_range &e) {
                logError(
                    "Network::getContentSize() : ",
                    "Content-Length value out of range: " + sizeStr + " - " + e.what());
            }

            return std::nullopt;
        }

        std::optional<std::string> Network::getContentType(const std::string &url) {
            RequestConfig config;
            config.url = url;
            config.method = RequestType::Head;

            auto result = execute(config);
            if (result.hasError || result.statusCode != 200) {
                return std::nullopt;
            }

            // Find Content-Type in headers
            std::string headers = result.content;
            std::transform(headers.begin(), headers.end(), headers.begin(), ::tolower);

            std::size_t pos = headers.find("content-type:");
            if (pos == std::string::npos) {
                return std::nullopt;
            }

            std::size_t start = pos + 13; // Skip "content-type:"
            std::size_t end = headers.find_first_of("\r\n", start);
            std::string contentType = headers.substr(start, end - start);

            // Trim whitespace
            contentType.erase(0, contentType.find_first_not_of(" \t"));
            contentType.erase(contentType.find_last_not_of(" \t") + 1);

            return contentType;
        }

        bool Network::multiThreadedDownload(const MultiDownloadConfig &config) {
            constexpr const size_t defaultChunkSize = 5 * 1024 * 1024; // 5MB default segment size
            constexpr const size_t defaultSegments = 100;              // Default segment count
            constexpr const size_t minSegmentSize = 256 * 1024;        // Minimum segment size (256KB)
            constexpr const size_t maxSegments = 200;                  // Maximum segment count limit

            logInfo(
                "Network::multiThreadedDownload() : ",
                "Starting multi-threaded download, ID: " + config.config.requestId);

            // Get file size first
            auto fileSize = getContentSize(config.config.url);
            if (fileSize == std::nullopt || *fileSize == 0) {
                logError(
                    "Network::multiThreadedDownload() : ",
                    "Failed to get file size or file is empty, ID: " + config.config.requestId);
                return false;
            }

            // Calculate segmentation info
            size_t numSegments = 0; // Number of segments
            size_t chunkSize = 0;   // Size of each segment

            // Determine segmentation method based on strategy
            switch (config.approach) {
                case MultiDownloadConfig::Approach::Thread:
                    // Specify task count
                    numSegments = (config.segmentParam > 0) ? config.segmentParam : defaultSegments;
                    chunkSize = *fileSize / numSegments;
                    // Ensure segments have at least minimum size
                    if (chunkSize < minSegmentSize) {
                        numSegments = std::max<>(size_t(1), *fileSize / minSegmentSize);
                        chunkSize = *fileSize / numSegments;
                    }
                    break;

                case MultiDownloadConfig::Approach::Size:
                    // Specify segment size
                    chunkSize = (config.segmentParam > 0) ? config.segmentParam : defaultChunkSize;
                    // Ensure segment size is reasonable
                    chunkSize = std::max<>(minSegmentSize, chunkSize);
                    numSegments = (*fileSize + chunkSize - 1) / chunkSize; // Round up
                    break;

                case MultiDownloadConfig::Approach::Quantity:
                    // Specify segment count
                    numSegments = (config.segmentParam > 0) ? config.segmentParam : defaultSegments;
                    chunkSize = (*fileSize + numSegments - 1) / numSegments; // Round up
                    break;

                case MultiDownloadConfig::Approach::Auto:
                default:
                    // Automatically determine best segmentation method
                    if (*fileSize < 10 * defaultChunkSize) {
                        // Small file: segment by quantity, but ensure each segment is at least minSegmentSize
                        numSegments = std::min<>(defaultSegments, *fileSize / minSegmentSize);
                        if (numSegments < 1)
                            numSegments = 1;
                        chunkSize = (*fileSize + numSegments - 1) / numSegments;
                    } else {
                        // Large file: segment by size
                        chunkSize = defaultChunkSize;
                        numSegments = (*fileSize + chunkSize - 1) / chunkSize;
                    }
                    break;
            }

            // Limit segment count to avoid creating too many tasks
            if (numSegments > maxSegments) {
                logWarn(
                    "Network::multiThreadedDownload() : ",
                    "Reducing segment count from " + std::to_string(numSegments) +
                        " to " + std::to_string(maxSegments) + " to avoid overloading the system");
                numSegments = maxSegments;
                chunkSize = (*fileSize + numSegments - 1) / numSegments;
            }

            // Ensure at least one segment
            if (numSegments < 1)
                numSegments = 1;

            // Success code list string
            std::string successCodesStr;
            for (auto code : config.successCodes) {
                successCodesStr += std::to_string(code) + ",";
            }

            logInfo(
                "Network::multiThreadedDownload() : ",
                "Download strategy: " + std::string(config.approach == MultiDownloadConfig::Approach::Auto ? "Auto" : config.approach == MultiDownloadConfig::Approach::Thread ? "Thread"
                                                                                                                  : config.approach == MultiDownloadConfig::Approach::Size     ? "Size"
                                                                                                                                                                               : "Quantity") +
                    ", Segments: " + std::to_string(numSegments) +
                    ", Chunk size: " + std::to_string(chunkSize) + " bytes" +
                    ", Expected success codes: " + successCodesStr +
                    ", ID: " + config.config.requestId);

            logInfo(
                "Network::multiThreadedDownload() : ",
                "File size: " + std::to_string(*fileSize) + " bytes" +
                    ", Download URL: " + config.config.url +
                    ", Output file: " + config.config.fileName +
                    ", ID: " + config.config.requestId);

            // Store download information for each segment
            struct DownloadSegment {
                std::string range;
                std::string tempFile;
                std::string segmentId;
                std::future<NetworkResult> result;
                bool success;
            };

            std::vector<DownloadSegment> segments;
            segments.reserve(numSegments);

            // Create and submit download tasks
            for (size_t i = 0; i < numSegments; ++i) {
                // Calculate start and end positions for current segment
                size_t startByte = i * chunkSize;
                size_t endByte = (i == numSegments - 1) ? *fileSize - 1 : (i + 1) * chunkSize - 1;

                // Ensure not exceeding file size
                if (startByte >= *fileSize) {
                    logWarn(
                        "Network::multiThreadedDownload() : ",
                        "Segment " + std::to_string(i) + " starts beyond file size, skipping");
                    continue;
                }

                if (endByte >= *fileSize) {
                    endByte = *fileSize - 1;
                }

                std::string range = std::to_string(startByte) + "-" + std::to_string(endByte);
                // Use more identifiable temporary filename, including part of original filename
                std::string baseName = std::filesystem::path(config.config.fileName).filename().string();
                std::string tempFileName = info::tempDir() + "/" + baseName + "." +
                                           config.config.requestId.substr(0, 8) + "." +
                                           std::to_string(i);
                std::string segmentId = config.config.requestId + "-" + std::to_string(i);

                // Configure request for current segment
                RequestConfig segmentConfig = config.config;
                segmentConfig.range = range;
                segmentConfig.fileName = tempFileName;
                segmentConfig.requestId = segmentId;
                segmentConfig.method = RequestType::DownloadFile;

                logDebug(
                    "Network::multiThreadedDownload() : ",
                    "Creating segment " + std::to_string(i) +
                        ", range: " + range +
                        ", temp file: " + tempFileName);

                // Submit download task to thread pool
                segments.push_back({
                    range,
                    tempFileName,
                    segmentId,
                    executeAsync(segmentConfig),
                    false // Initialize as not successful
                });
            }

            // Wait for all segments to complete and process results
            logInfo(
                "Network::multiThreadedDownload() : ",
                "Waiting for " + std::to_string(segments.size()) + " segments to complete");

            // First round: wait for all tasks to complete and record results
            for (size_t i = 0; i < segments.size(); ++i) {
                auto result = segments[i].result.get();

                // Check if successful
                segments[i].success = false;
                for (auto code : config.successCodes) {
                    if (result.statusCode == code) {
                        segments[i].success = true;
                        break;
                    }
                }

                if (!segments[i].success) {
                    logWarn(
                        "Network::multiThreadedDownload() : ",
                        "Segment " + std::to_string(i) + " failed, status code: " +
                            std::to_string(result.statusCode) +
                            ", ID: " + segments[i].segmentId);
                }
            }

            // Second round: retry failed segments
            std::vector<std::future<NetworkResult>> retryResults;
            bool anyFailed = false;

            for (size_t i = 0; i < segments.size(); ++i) {
                if (!segments[i].success) {
                    logInfo(
                        "Network::multiThreadedDownload() : ",
                        "Retrying segment " + std::to_string(i) +
                            ", range: " + segments[i].range +
                            ", ID: " + segments[i].segmentId);

                    RequestConfig retryConfig = config.config;
                    retryConfig.range = segments[i].range;
                    retryConfig.fileName = segments[i].tempFile;
                    retryConfig.requestId = segments[i].segmentId + "-retry";
                    retryConfig.method = RequestType::DownloadFile;

                    // Submit retry task
                    retryResults.push_back(executeAsync(retryConfig));
                }
            }

            // Process retry results
            if (!retryResults.empty()) {
                logInfo(
                    "Network::multiThreadedDownload() : ",
                    "Waiting for " + std::to_string(retryResults.size()) + " retry operations");

                size_t retryIndex = 0;
                for (size_t i = 0; i < segments.size(); ++i) {
                    if (!segments[i].success) {
                        auto result = retryResults[retryIndex++].get();

                        // Re-check if successful
                        segments[i].success = false;
                        for (auto code : config.successCodes) {
                            if (result.statusCode == code) {
                                segments[i].success = true;
                                break;
                            }
                        }

                        if (!segments[i].success) {
                            logError(
                                "Network::multiThreadedDownload() : ",
                                "Segment " + std::to_string(i) + " failed after retry, status code: " +
                                    std::to_string(result.statusCode) +
                                    ", range: " + segments[i].range);
                            anyFailed = true;
                        }
                    }
                }
            }

            // Check if all segments downloaded successfully
            if (anyFailed) {
                logError(
                    "Network::multiThreadedDownload() : ",
                    "Some segments failed to download, cannot complete the file");

                // Clean up temporary files
                for (const auto &segment : segments) {
                    try {
                        std::filesystem::remove(segment.tempFile);
                    } catch (const std::exception &e) {
                        logWarn(
                            "Network::multiThreadedDownload() : ",
                            "Failed to remove temp file: " + segment.tempFile +
                                ", error: " + e.what());
                    }
                }

                return false;
            }

            // Merge all file segments
            logInfo(
                "Network::multiThreadedDownload() : ",
                "Merging " + std::to_string(segments.size()) + " segments into final file: " +
                    config.config.fileName);

            try {
                // Create output file
                std::fstream outputFile(config.config.fileName,
                                        std::ios::out | std::ios::binary | std::ios::trunc);

                if (!outputFile.is_open()) {
                    logError(
                        "Network::multiThreadedDownload() : ",
                        "Failed to open output file: " + config.config.fileName);
                    return false;
                }

                // Counter for checking merged file size
                size_t totalBytesWritten = 0;
                constexpr size_t bufferSize = 262144; // 256KB buffer
                std::vector<char> buffer(bufferSize);

                // Merge each segment in order
                for (size_t i = 0; i < segments.size(); ++i) {
                    std::ifstream segmentFile(segments[i].tempFile, std::ios::binary);

                    if (!segmentFile.is_open()) {
                        logError(
                            "Network::multiThreadedDownload() : ",
                            "Failed to open segment file: " + segments[i].tempFile);
                        outputFile.close();
                        return false;
                    }

                    // Use buffer to copy data for better efficiency
                    while (!segmentFile.eof()) {
                        segmentFile.read(buffer.data(), bufferSize);
                        std::streamsize bytesRead = segmentFile.gcount();

                        if (bytesRead > 0) {
                            outputFile.write(buffer.data(), bytesRead);
                            totalBytesWritten += bytesRead;
                        }
                    }

                    segmentFile.close();

                    // Delete temporary file
                    try {
                        std::filesystem::remove(segments[i].tempFile);
                    } catch (const std::exception &e) {
                        logWarn(
                            "Network::multiThreadedDownload() : ",
                            "Failed to remove temp file: " + segments[i].tempFile +
                                ", error: " + e.what());
                    }
                }

                outputFile.close();

                // Check merged file size
                if (totalBytesWritten != *fileSize) {
                    logWarn(
                        "Network::multiThreadedDownload() : ",
                        "Merged file size (" + std::to_string(totalBytesWritten) +
                            " bytes) doesn't match expected size (" + std::to_string(*fileSize) +
                            " bytes)");
                }

                logInfo(
                    "Network::multiThreadedDownload() : ",
                    "Download completed successfully, total size: " +
                        std::to_string(totalBytesWritten) + " bytes, ID: " +
                        config.config.requestId);

                return true;
            } catch (const std::exception &e) {
                logError(
                    "Network::multiThreadedDownload() : ",
                    "Error merging file segments: " + std::string(e.what()) +
                        ", ID: " + config.config.requestId);
                return false;
            }
        }

    } // namespace network
} // namespace neko
