// Neko Module
#include "neko/network/network.hpp"
#include "neko/network/networkCommon.hpp"
#include "neko/network/networkTypes.hpp"

#include "neko/function/utilities.hpp" // For utility functions bool to, isProxyAddress etc.
#include "neko/system/platform.hpp"    // For temporary folder current working directory

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
#include <sstream>

#if defined(_WIN32)
#include <winreg.h> // For Windows Proxy
#endif

namespace neko::network {

    //=================================================
    // helper Implementation
    //=================================================

    std::optional<std::string> helper::getSysProxy() {
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
                    if (res.find("http") == std::string::npos && res.find("https") == std::string::npos && res.find("socks5") == std::string::npos)
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

    //=================================================
    // Network Implementation
    //=================================================

    Network::Network(std::shared_ptr<executor::IAsyncExecutor> executor, std::shared_ptr<log::ILogger> logger)
        : executor(std::move(executor)), logger(std::move(logger)) {
        curl_global_init(CURL_GLOBAL_DEFAULT);
    }
    Network::~Network() {
        curl_global_cleanup();
    }

    void Network::logError(const std::string &msg) {
        if (logger)
            logger->error("Network Error: " + msg);
    }
    void Network::logInfo(const std::string &msg) {
        if (logger)
            logger->info("Network: " + msg);
    }
    void Network::logWarn(const std::string &msg) {
        if (logger)
            logger->warn("Network: " + msg);
    }
    void Network::logDebug(const std::string &msg) {
        if (logger)
            logger->debug("Network [Debug]: " + msg);
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
        auto sysProxy = helper::getSysProxy();

        std::stringstream ss;
        ss << "Network::logRequestInfo() : "
           << "Request: URL: " << config.url
           << ", Method: " << methodStr
           << ", FileName: " << config.fileName
           << ", Range: " << config.range
           << ", Resumable: " << util::logic::boolTo(config.resumable)
           << ", UserAgent: " << util::logic::boolTo(config.userAgent.empty(), config::globalConfig.getUserAgent(), config.userAgent)
           << ", Global Protocol: " << config::globalConfig.getProtocol()
           << ", Proxy: " << util::logic::boolTo<std::string>(config.proxy.empty(), "<none>", config.proxy)
           << ", SysProxy: " << sysProxy.value_or("<none>")
           << ", ID: " << config.requestId;

        logInfo(ss.str());

        ss.str(""); // Clear the stringstream for next use
        ss << "Network::logRequestInfo() : "
           << "Header: " << util::logic::boolTo<std::string>(config.header.empty(), "<none>", config.header)
           << ", PostData: " << util::logic::boolTo<std::string>(config.postData.empty(), "<none>", config.postData)
           << ", ProgressCallback: " << (config.progressCallback ? "set" : "not set");

        logDebug(ss.str());
    }

    std::optional<std::string> Network::initCurl(CURL *curl, const RequestConfig &config) {
        std::stringstream errorMsg;
        if (!curl) {
            errorMsg << "Failed to initialize curl. ID: " << config.requestId;
            logError("Network::initCurl() : " + errorMsg.str());
            return errorMsg.str();
        }

        // Set CA certificate
        std::string caPath = system::workPath() + "/cacert.pem";
        if (std::filesystem::exists(caPath)) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, caPath.c_str());
        } else {
            errorMsg << "The cacert.pem file does not exist at: " << caPath << ", ID: " << config.requestId << ", Use the system default CA bundle.";
            logWarn("Network::initCurl() : " + errorMsg.str());
            errorMsg.str("");
        }

        curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);
        auto systemProxy = helper::getSysProxy();

        // proxy = "true" means use system proxy
        if (config.proxy == "true" && systemProxy && util::check::isProxyAddress(*systemProxy)) {
            curl_easy_setopt(curl, CURLOPT_PROXY, systemProxy.value_or("").c_str());
        } else if (!config.proxy.empty() && util::check::isProxyAddress(config.proxy)) {
            curl_easy_setopt(curl, CURLOPT_PROXY, config.proxy.c_str());
        } else {
            curl_easy_setopt(curl, CURLOPT_PROXY, "");
            if (!config.proxy.empty() && config.proxy != "false") {
                errorMsg << "Invalid proxy address: " << config.proxy << ", ID: " << config.requestId;
                logWarn("Network::initCurl() : " + errorMsg.str());
                errorMsg.str("");
            }
        }

        // Handle resumable downloads
        if (config.resumable && !config.fileName.empty()) {
            std::fstream infile(config.fileName, std::ios::in | std::ios::binary);
            if (!infile.is_open()) {
                errorMsg << "Failed to open file for resuming: " << config.fileName << ", ID: " << config.requestId;
                logError("Network::initCurl() : " + errorMsg.str());
                return errorMsg.str();
            }

            infile.seekg(0, std::ios::end);
            neko::uint64 fileSize = infile.tellg();
            infile.close();

            // If range download, resume from the specified range (file_size + range_start)
            if (!config.range.empty()) {
                neko::uint64 resumeOffset = fileSize + std::stoull(config.range.substr(0, config.range.find('-')));
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
        std::string userAgent = config.userAgent.empty() ? config::globalConfig.getUserAgent() : config.userAgent;
        curl_easy_setopt(curl, CURLOPT_USERAGENT, userAgent.c_str());
        curl_easy_setopt(curl, CURLOPT_URL, config.url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

        return std::nullopt; // No error
    }

    int Network::getHttpStatusCode(CURL *curl) {
        long statusCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        return static_cast<int>(statusCode);
    }

    // This callback function is used to handle libcurl debug messages, called only in Network::performRequest function.
    namespace {
        neko::uint64 debugCallback(CURL *handle, curl_infotype type, char *data, neko::uint64 size, void *userptr) {
            if (type == CURLINFO_TEXT || type == CURLINFO_SSL_DATA_IN || type == CURLINFO_SSL_DATA_OUT) {
                if (userptr) {
                    static_cast<std::vector<std::string> *>(userptr)->emplace_back(std::string(data, size));
                }
            }
            return 0;
        }
    } // namespace

    template <typename T>
    NetworkResult<T> Network::performRequest(CURL *curl, const RequestConfig &config) {
        NetworkResult<T> result;
        std::stringstream ss;

        ss << "Network::performRequest() : "
           << "Performing request, URL: " << config.url
           << ", Method: " << static_cast<int>(config.method)
           << ", ID: " << config.requestId;
        logInfo(ss.str());
        ss.str("");

        // Set up debug messages vector
        std::vector<std::string> curlDebugMsgs;
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, &debugCallback);
        curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &curlDebugMsgs);

        // Output libcurl version information
        curl_version_info_data *ver = curl_version_info(CURLVERSION_NOW);
        ss << "Network::performRequest() : "
           << "libcurl version: " << ver->version
           << ", SSL version: " << ver->ssl_version
           << ", ID: " << config.requestId;
        logInfo(ss.str());
        ss.str("");

        // Execute request
        CURLcode res = curl_easy_perform(curl);

        // Get connection IP
        char *ip = nullptr;
        curl_easy_getinfo(curl, CURLINFO_PRIMARY_IP, &ip);

        // Set status code
        result.statusCode = getHttpStatusCode(curl);

        if (res != CURLE_OK) {
            // Handle error
            ss << "Failed to get network req: " << std::string(curl_easy_strerror(res)) << ", ID: " << config.requestId << ", IP: " << (ip ? ip : "N/A");

            std::string basicMsg = ss.str();
            logError(basicMsg);
            ss.str("");

            // Add debug messages
            for (const auto &dbg : curlDebugMsgs) {
                ss << "\n[Debug] " << dbg;
            }

            // Add certificate information
            struct curl_certinfo *ci = nullptr;
            if (curl_easy_getinfo(curl, CURLINFO_CERTINFO, &ci) == CURLE_OK && ci) {
                for (int i = 0; i < ci->num_of_certs; i++) {
                    struct curl_slist *slist = ci->certinfo[i];
                    for (; slist; slist = slist->next) {
                        ss << "\n[CERT] " << slist->data;
                    }
                }
            }

            logDebug(ss.str());
            result.setError(basicMsg, ss.str());
            return result;
        }
        ss << "Network::performRequest() : "
           << "Request completed successfully, ID: " << config.requestId
           << ", Status Code: " << result.statusCode;

        logInfo(ss.str());
        return result;
    }

    template <typename T>
    NetworkResult<T> Network::handleGetRequest(CURL *curl, const RequestConfig &config) {
        T content{};
        helper::WriteCallbackContext<T> writeContext;
        writeContext.buffer = &content;
        writeContext.progressCallback = const_cast<std::function<void(neko::uint64)>*>(&config.progressCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &helper::writeToCallback<T>);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeContext);

        NetworkResult<T> result = performRequest<T>(curl, config);

        if (!result.hasError) {
            result.content = std::move(content);
        }

        return result;
    }

    template <typename T>
    NetworkResult<T> Network::handleHeadRequest(CURL *curl, const RequestConfig &config) {
        T headerContent{};
        helper::WriteCallbackContext<T> writeContext;
        writeContext.buffer = &headerContent;
        writeContext.progressCallback = const_cast<std::function<void(neko::uint64)>*>(&config.progressCallback);

        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &helper::writeToCallback<T>);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeContext);

        NetworkResult<T> result = performRequest<T>(curl, config);

        if (!result.hasError) {
            result.content = std::move(headerContent);
        }

        return result;
    }

    template <typename T>
    NetworkResult<T> Network::handlePostRequest(CURL *curl, const RequestConfig &config) {
        T content{};
        helper::WriteCallbackContext<T> writeContext;
        writeContext.buffer = &content;
        writeContext.progressCallback = const_cast<std::function<void(neko::uint64)>*>(&config.progressCallback);

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, config.postData.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &helper::writeToCallback<T>);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeContext);

        NetworkResult<T> result = performRequest<T>(curl, config);

        if (!result.hasError) {
            result.content = std::move(content);
        }

        return result;
    }

    template <typename T>
    NetworkResult<T> Network::handleDownloadRequest(CURL *curl, const RequestConfig &config) {
        NetworkResult<T> result;

        std::fstream file(config.fileName, std::ios::out | std::ios::binary | (config.resumable ? std::ios::app : std::ios::trunc));
        if (!file.is_open()) {
            std::string errorMsg = "Failed to open file for writing: " + config.fileName + ", ID: " + config.requestId;
            logError("Network::handleDownloadRequest() : " + errorMsg);
            result.setError("File operation error : ", errorMsg);
            return result;
        }

        helper::WriteCallbackContext<std::fstream> writeContext;
        writeContext.buffer = &file;
        writeContext.progressCallback = const_cast<std::function<void(neko::uint64)>*>(&config.progressCallback);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &helper::writeToCallback<std::fstream>);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &writeContext);

        result = performRequest<T>(curl, config);
        return result;
    }

    template <typename T>
    NetworkResult<T> Network::doExecute(const RequestConfig &config) {
        logRequestInfo(config);

        CURL *curl = curl_easy_init();
        NetworkResult<T> result;

        auto initError = initCurl(curl, config);
        if (initError.has_value()) {
            result.setError("Failed to initialize libcurl", initError.value());
            if (curl)
                curl_easy_cleanup(curl);
            return result;
        }

        switch (config.method) {
            case RequestType::Get:
                result = handleGetRequest<T>(curl, config);
                break;
            case RequestType::Head:
                result = handleHeadRequest<T>(curl, config);
                break;
            case RequestType::Post:
                result = handlePostRequest<T>(curl, config);
                break;
            case RequestType::DownloadFile:
                result = handleDownloadRequest<T>(curl, config);
                break;
            case RequestType::UploadFile:
                result.setError("Upload not implemented");
                break;
            default:
                result.setError("Unknown request type");
                break;
        }

        curl_easy_cleanup(curl);
        return result;
    }

    template <typename T>
    NetworkResult<T> Network::execute(const RequestConfig &config) {
        return doExecute<T>(config);
    }

    template <typename T>
    std::future<NetworkResult<T>> Network::executeAsync(const RequestConfig &config) {
        if (executor) {
            return executor->submit([this, config]() {
                return this->execute<T>(config);
            });
        } else {
            // fallback: std::async
            return std::async(std::launch::async, [this, config]() {
                return this->execute<T>(config);
            });
        }
    }

    template <typename T>
    NetworkResult<T> Network::executeWithRetry(const RetryConfig &retryConfig) {
        std::string expectCodes;
        for (auto code : retryConfig.successCodes) {
            expectCodes.append(std::to_string(code) + std::string(","));
        }
        if (!expectCodes.empty()) {
            expectCodes.pop_back(); // Remove the trailing comma
        }
        std::stringstream ss;
        ss << "Network::executeWithRetry() : "
           << "Executing request with retry, URL: " << retryConfig.config.url
           << ", Expected codes: " << expectCodes
           << ", Delay: " << retryConfig.retryDelay.count()
           << ", Max attempts: " << retryConfig.maxRetries
           << ", ID: " << retryConfig.config.requestId;

        logInfo(ss.str());
        ss.str("");

        NetworkResult<T> result;

        for (int attempt = 0; attempt < retryConfig.maxRetries; ++attempt) {
            result = execute<T>(retryConfig.config);

            ss << "Network::executeWithRetry() : "
               << "Attempt " << (attempt + 1) << " status code: " << result.statusCode
               << ", ID: " << retryConfig.config.requestId;
            logInfo(ss.str());
            ss.str("");

            for (auto code : retryConfig.successCodes) {
                if (result.statusCode == code) {
                    return result;
                }
            }
            ss << "Network::executeWithRetry() : "
               << "Attempt " << (attempt + 1) << " failed, status code: " << result.statusCode
               << ", ID: " << retryConfig.config.requestId;
            logWarn(ss.str());
            ss.str("");

            if (attempt < retryConfig.maxRetries - 1) {
                std::this_thread::sleep_for(retryConfig.retryDelay);
            }
        }

        ss << "Network::executeWithRetry() : "
           << "All retry attempts failed, ID: " << retryConfig.config.requestId
           << ", Expected codes: " << expectCodes
           << ", Last status code: " << result.statusCode;
        logError(ss.str());
        result.setError("All retry attempts failed", ss.str());
        return result;
    }

    template <typename T>
    std::optional<T> Network::findUrlHeader(const std::string &url, const std::string &headerName) {
        RequestConfig config;
        config.url = url;
        config.method = RequestType::Head;

        auto result = execute<std::string>(config);
        if (!result.isSuccess()) {
            return std::nullopt;
        }

        // Find the specified header in the response
        std::string headers = result.content;
        std::transform(headers.begin(), headers.end(), headers.begin(), ::tolower);

        std::size_t pos = headers.find(headerName + ":");
        if (pos == std::string::npos) {
            return std::nullopt;
        }

        std::size_t start = pos + headerName.length() + 1; // Skip "headerName: "
        std::size_t end = headers.find_first_of("\r\n", start);
        std::string headerValue = headers.substr(start, end - start);

        // Trim whitespace
        headerValue.erase(0, headerValue.find_first_not_of(" \t"));
        headerValue.erase(headerValue.find_last_not_of(" \t") + 1);

        return headerValue;
    }

    std::optional<neko::uint64> Network::getContentSize(const std::string &url) {
        auto contentLength = findUrlHeader<std::string>(url, "Content-Length");
        std::stringstream ss;
        if (!contentLength.has_value()) {
            ss << "Network::getContentSize() : "
               << "Failed to find Content-Length header, URL: " << url;
            logError(ss.str());
            return std::nullopt;
        }
        try {
            return std::stoull(*contentLength);
        } catch (const std::invalid_argument &e) {
            ss << "Network::getContentSize() : "
               << "Invalid Content-Length value: " << *contentLength << ", URL: " << url
               << ", Error: " << e.what();
            logError(ss.str());
        } catch (const std::out_of_range &e) {
            ss << "Network::getContentSize() : "
               << "Content-Length value out of range: " << *contentLength << ", URL: " << url
               << ", Error: " << e.what();
            logError(ss.str());
        }

        return std::nullopt;
    }

    std::optional<std::string> Network::getContentType(const std::string &url) {
        return findUrlHeader<std::string>(url, "Content-Type");
    }

    bool Network::multiThreadedDownload(const MultiDownloadConfig &config) {
        constexpr const neko::uint64 defaultChunkSize = 5 * 1024 * 1024; // 5MB default segment size
        constexpr const neko::uint64 defaultSegments = 100;              // Default segment count
        constexpr const neko::uint64 minSegmentSize = 256 * 1024;        // Minimum segment size (256KB)
        constexpr const neko::uint64 maxSegments = 255;                  // Maximum segment count limit

        std::stringstream ss;
        ss << "Network::multiThreadedDownload() : "
           << "Starting multi-threaded download, ID: " << config.config.requestId;

        logInfo(ss.str());
        ss.str("");

        // Get file size first
        auto fileSize = getContentSize(config.config.url);
        if (fileSize.value_or(0) == 0) {
            ss << "Network::multiThreadedDownload() : "
               << "Failed to get file size or file is empty, URL: " << config.config.url
               << ", ID: " << config.config.requestId;
            logError(ss.str());
            return false;
        }

        // Create output file
        std::fstream outputFile(config.config.fileName,
                                std::ios::out | std::ios::binary | std::ios::trunc);

        if (!outputFile.is_open()) {
            ss << "Network::multiThreadedDownload() : "
               << "Failed to open output file for merging: " << config.config.fileName
               << ", ID: " << config.config.requestId;
            logError(ss.str());
            ss.str("");
            return false;
        }

        // Calculate segmentation info
        neko::uint64 numSegments = 0; // Number of segments
        neko::uint64 chunkSize = 0;   // Size of each segment

        // Determine segmentation method based on strategy
        switch (config.approach) {
            case MultiDownloadConfig::Approach::Thread:
                numSegments = (config.segmentParam > 0) ? config.segmentParam : defaultSegments;
                chunkSize = *fileSize / numSegments;
                // Ensure segments have at least minimum size
                if (chunkSize < minSegmentSize) {
                    numSegments = std::max<>(neko::uint64(1), *fileSize / minSegmentSize);
                    chunkSize = *fileSize / numSegments;
                }
                break;

            case MultiDownloadConfig::Approach::Size:
                chunkSize = (config.segmentParam > 0) ? config.segmentParam : defaultChunkSize;
                // Ensure segment size is reasonable
                chunkSize = std::max<>(minSegmentSize, chunkSize);
                numSegments = (*fileSize + chunkSize - 1) / chunkSize; // Round up
                break;

            case MultiDownloadConfig::Approach::Quantity:
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

            ss << "Network::multiThreadedDownload() : "
               << "Reducing segment count from " << numSegments
               << " to " << maxSegments << " to avoid overloading the system";
            logWarn(ss.str());
            ss.str("");

            // Adjust segment count and chunk size
            numSegments = maxSegments;
            chunkSize = (*fileSize + numSegments - 1) / numSegments;
        }

        if (numSegments < 1)
            numSegments = 1;

        // Success code list string to print
        std::string successCodesStr;
        for (auto code : config.successCodes) {
            successCodesStr += std::to_string(code) + ",";
        }
        if (!successCodesStr.empty())
            successCodesStr.pop_back();

        std::string approachStr;
        switch (config.approach) {
            case MultiDownloadConfig::Approach::Auto:
                approachStr = "Auto";
                break;
            case MultiDownloadConfig::Approach::Thread:
                approachStr = "Thread";
                break;
            case MultiDownloadConfig::Approach::Size:
                approachStr = "Size";
                break;
            case MultiDownloadConfig::Approach::Quantity:
                approachStr = "Quantity";
                break;
            default:
                approachStr = "Unknown";
                break;
        }

        ss << "Network::multiThreadedDownload() : "
           << "Download strategy: " << approachStr
           << ", Segments: " << numSegments
           << ", Chunk size: " << chunkSize << " bytes"
           << ", Expected success codes: " << successCodesStr
           << ", ID: " << config.config.requestId;
        logInfo(ss.str());
        ss.str("");

        ss << "Network::multiThreadedDownload() : "
           << "File size: " << *fileSize << " bytes"
           << ", Download URL: " << config.config.url
           << ", Output file: " << config.config.fileName;
        logInfo(ss.str());
        ss.str("");

        // Store download information for each segment
        struct DownloadSegment {
            std::string range;
            std::string tempFile;
            std::string segmentId;
            std::future<NetworkResult<std::string>> result;
            bool success;
        };

        std::vector<DownloadSegment> segments;
        segments.reserve(numSegments);

        // Create and submit download tasks
        for (neko::uint64 i = 0; i < numSegments; ++i) {
            // Calculate start and end positions for current segment
            neko::uint64 startByte = i * chunkSize;
            neko::uint64 endByte = (i == numSegments - 1) ? *fileSize - 1 : (i + 1) * chunkSize - 1;

            // Ensure not exceeding file size
            if (startByte >= fileSize.value()) {
                ss << "Network::multiThreadedDownload() : "
                   << "Segment " << i << " starts beyond file size, skipping";
                logWarn(ss.str());
                ss.str("");
                continue;
            }

            if (endByte >= fileSize.value()) {
                endByte = fileSize.value() - 1;
            }

            std::string range = std::to_string(startByte) + "-" + std::to_string(endByte);
            // Use more identifiable temporary filename, including part of original filename
            std::string baseName = std::filesystem::path(config.config.fileName).filename().string();
            std::string tempFileName = system::tempFolder() + baseName + "." +
                                       config.config.requestId.substr(0, 8) + "." +
                                       std::to_string(i);
            std::string segmentId = config.config.requestId + "-" + std::to_string(i);

            // Configure request for current segment
            RequestConfig segmentConfig = config.config;
            segmentConfig.range = range;
            segmentConfig.fileName = tempFileName;
            segmentConfig.requestId = segmentId;
            segmentConfig.method = RequestType::DownloadFile;

            ss << "Network::multiThreadedDownload() : "
               << "Creating segment " << i
               << ", Range: " << range
               << ", Temp file: " << tempFileName
               << ", ID: " << segmentId;
            logDebug(ss.str());
            ss.str("");

            // Submit download task to async executor
            segments.push_back({
                range,
                tempFileName,
                segmentId,
                executeAsync(segmentConfig),
                false // Initialize as not successful
            });
        }

        // Wait for all segments to complete and process results

        ss << "Network::multiThreadedDownload() : "
           << "Waiting for " << segments.size() << " segments to complete";
        logInfo(ss.str());
        ss.str("");

        // First round: wait for all tasks to complete and record results
        for (neko::uint64 i = 0; i < segments.size(); ++i) {
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
                ss << "Network::multiThreadedDownload() : "
                   << "Segment " << i << " failed, status code: " << result.statusCode
                   << ", ID: " << segments[i].segmentId;
                logError(ss.str());
                ss.str("");
            }
        }

        // Second round: retry failed segments
        std::vector<std::future<NetworkResult<std::string>>> retryResults;
        bool anyFailed = false;

        for (size_t i = 0; i < segments.size(); ++i) {
            if (!segments[i].success) {
                ss << "Network::multiThreadedDownload() : "
                   << "Segment " << i << " failed, retrying"
                   << ", Range: " << segments[i].range
                   << ", ID: " << segments[i].segmentId;
                logError(ss.str());
                ss.str("");

                RequestConfig retryConfig = config.config;
                retryConfig.range = segments[i].range;
                retryConfig.fileName = segments[i].tempFile;
                retryConfig.requestId = segments[i].segmentId + "-retry";
                retryConfig.method = RequestType::DownloadFile;

                retryResults.push_back(executeAsync(retryConfig));
            }
        }

        // Process retry results
        if (!retryResults.empty()) {
            ss << "Network::multiThreadedDownload() : "
               << "Retrying " << retryResults.size() << " failed segments";
            logInfo(ss.str());
            ss.str("");

            neko::uint64 retryIndex = 0;
            for (neko::uint64 i = 0; i < segments.size(); ++i) {
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
                        ss << "Network::multiThreadedDownload() : "
                           << "Segment " << i << " failed after retry, status code: " << result.statusCode
                           << ", Range: " << segments[i].range
                           << ", ID: " << segments[i].segmentId;
                        logError(ss.str());
                        ss.str("");
                        anyFailed = true;
                    }
                }
            }
        }

        // Check if all segments downloaded successfully
        if (anyFailed) {
            ss << "Network::multiThreadedDownload() : "
               << "Some segments failed to download, cannot complete the file, ID: " << config.config.requestId;
            logError(ss.str());
            ss.str("");

            for (const auto &segment : segments) {
                try {
                    std::filesystem::remove(segment.tempFile);
                } catch (const std::exception &e) {
                    ss << "Network::multiThreadedDownload() : "
                       << "Failed to remove temp file: " << segment.tempFile
                       << ", error: " << e.what()
                       << ", ID: " << segment.segmentId;
                    logWarn(ss.str());
                    ss.str("");
                }
            }

            return false;
        }

        // Merge all file segments
        ss << "Network::multiThreadedDownload() : "
           << "All segments downloaded successfully, merging files, ID: " << config.config.requestId;
        logInfo(ss.str());
        ss.str("");

        try {

            neko::uint64 totalBytesWritten = 0;
            constexpr neko::uint64 bufferSize = 256 * 1024;
            std::vector<char> buffer(bufferSize);

            // Merge each segment in order
            for (neko::uint64 i = 0; i < segments.size(); ++i) {
                std::ifstream segmentFile(segments[i].tempFile, std::ios::binary);

                if (!segmentFile.is_open()) {
                    ss << "Network::multiThreadedDownload() : "
                       << "Failed to open segment file: " << segments[i].tempFile
                       << ", ID: " << segments[i].segmentId;
                    logError(ss.str());
                    ss.str("");
                    outputFile.close();
                    return false;
                }

                while (!segmentFile.eof()) {
                    segmentFile.read(buffer.data(), bufferSize);
                    std::streamsize bytesRead = segmentFile.gcount();

                    if (bytesRead > 0) {
                        outputFile.write(buffer.data(), bytesRead);
                        totalBytesWritten += bytesRead;
                    }
                }

                segmentFile.close();

                // Remove temporary file
                try {
                    std::filesystem::remove(segments[i].tempFile);
                } catch (const std::exception &e) {
                    ss << "Network::multiThreadedDownload() : "
                       << "Failed to remove temp file: " << segments[i].tempFile
                       << ", ID: " << segments[i].segmentId
                       << ", error: " << e.what();
                    logWarn(ss.str());
                    ss.str("");
                }
            }

            outputFile.close();

            // Check merged file size
            if (totalBytesWritten != fileSize.value()) {
                ss << "Network::multiThreadedDownload() : "
                   << "Merged file size (" << totalBytesWritten << " bytes) "
                   << "doesn't match expected size (" << *fileSize << " bytes)"
                   << ", ID: " << config.config.requestId;
                logError(ss.str());
                ss.str("");
            }

            ss << "Network::multiThreadedDownload() : "
               << "File segments merged successfully, total size: " << totalBytesWritten
               << " bytes, ID: " << config.config.requestId;
            logInfo(ss.str());
            ss.str("");

            return true;
        } catch (const std::exception &e) {
            ss << "Network::multiThreadedDownload() : "
               << "Error merging file segments: " << e.what()
               << ", ID: " << config.config.requestId;
            logError(ss.str());
            return false;
        }
    }
    // Explicit template instantiation for the types we want to use

    // Sync template instantiations
    template NetworkResult<std::string> Network::execute(const RequestConfig &);
    template NetworkResult<std::vector<char>> Network::execute(const RequestConfig &);
    template NetworkResult<std::fstream> Network::execute(const RequestConfig &);

    // Async template instantiations
    template std::future<NetworkResult<std::string>> Network::executeAsync(const RequestConfig &);
    template std::future<NetworkResult<std::vector<char>>> Network::executeAsync(const RequestConfig &);
    template std::future<NetworkResult<std::fstream>> Network::executeAsync(const RequestConfig &);

    // Retry template instantiations
    template NetworkResult<std::string> Network::executeWithRetry(const RetryConfig &);
    template NetworkResult<std::vector<char>> Network::executeWithRetry(const RetryConfig &);
    template NetworkResult<std::fstream> Network::executeWithRetry(const RetryConfig &);

    // IAsyncExecutor template instantiations
    template std::future<NetworkResult<std::string>> executor::IAsyncExecutor::submitImpl(std::function<NetworkResult<std::string>()> f);
    template std::future<NetworkResult<std::vector<char>>> executor::IAsyncExecutor::submitImpl(std::function<NetworkResult<std::vector<char>>()> f);
    template std::future<NetworkResult<std::fstream>> executor::IAsyncExecutor::submitImpl(std::function<NetworkResult<std::fstream>()> f);

    // StdAsyncExecutor template instantiations
    template std::future<NetworkResult<std::string>> executor::StdAsyncExecutor::submitImpl(std::function<NetworkResult<std::string>()> f);
    template std::future<NetworkResult<std::vector<char>>> executor::StdAsyncExecutor::submitImpl(std::function<NetworkResult<std::vector<char>>()> f);
    template std::future<NetworkResult<std::fstream>> executor::StdAsyncExecutor::submitImpl(std::function<NetworkResult<std::fstream>()> f);

#if defined(NEKO_IMPORT_THREAD_BUS)
    // BusThreadExecutor template instantiations
    template std::future<NetworkResult<std::string>> executor::BusThreadExecutor::submitImpl(std::function<NetworkResult<std::string>()> f);
    template std::future<NetworkResult<std::vector<char>>> executor::BusThreadExecutor::submitImpl(std::function<NetworkResult<std::vector<char>>()> f);
    template std::future<NetworkResult<std::fstream>> executor::BusThreadExecutor::submitImpl(std::function<NetworkResult<std::fstream>()> f);
#endif

} // namespace neko::network
