/**
 * @file network.hpp
 * @brief Network module header file
 * @author moehoshio
 * @date 2025-06-03
 */

#pragma once

// Neko Module
#include "neko/schema/nekodefine.hpp"

// C++ STL
#include <string>
#include <vector>

#include <chrono>
#include <future>
#include <optional>

#include <fstream>

using CURL = void;

namespace neko {


    /**
     * @defgroup network Network Module
     * @brief This module provides network request handling functionality.
     * @details The Network module provides classes and functions for making network requests, handling responses, and managing network configurations.
     */
    namespace network {

        /**
         * @class NetworkBase
         * @brief Base class for network operations
         * @see Network for a class that implements network requests using this base class.
         * @see Api for the struct that defines API endpoints.
         * @see NetConfig for the struct that stores global network configuration, which provides default values in RequestConfig.
         * @see RequestType for the enum that defines different request types.
         * @ingroup network
         */
        class NetworkBase {
        public:
            // API Definition
            struct Api {
                constexpr static const char *hostList[] = {NetWorkHostListDefine};

                // NekoLc API endpoints
                constexpr static const char *mainenance = "/v1/api/maintenance";
                constexpr static const char *checkUpdates = "/v1/api/checkUpdates";
                constexpr static const char *feedback = "/v1/api/feedbacklog";
                constexpr static const char *testing = "/v1/testing/ping";

                // Minecraft Authlib Injector API endpoints
                struct Authlib {
                    constexpr static const char *host = NetWorkAuthlibHostDefine;
                    constexpr static const char *root = "/api/yggdrasil";
                    constexpr static const char *authenticate = "/api/yggdrasil/authserver/authenticate";
                    constexpr static const char *refresh = "/api/yggdrasil/authserver/refresh";
                    constexpr static const char *validate = "/api/yggdrasil/authserver/validate";
                    constexpr static const char *invalidate = "/api/yggdrasil/authserver/invalidate";
                    constexpr static const char *signout = "/api/yggdrasil/authserver/signout";

                    struct Injector {
                        constexpr static const char *downloadHost = "authlib-injector.yushi.moe";
                        constexpr static const char *getVersionsList = "/artifacts.json";
                        constexpr static const char *latest = "/artifact/latest.json";
                        constexpr static const char *baseRoot = "/artifact"; // + /{build_number}.json
                    };
                    Injector injector;
                };
                Authlib authlib;
            };

            struct NetConfig {
                std::string userAgent;
                std::string proxy;
                std::string host;
                std::string protocol;
            };

            enum class RequestType {
                Get,
                Head,
                Post,
                DownloadFile,
                UploadFile
            };

            inline static NetConfig globalConfig;
            constexpr inline static Api api;

            /**
             * @brief Initialize the network module
             * @return std::future<void> - A future that will be ready when initialization is complete.
             * This function initializes the network module, including setting up global configurations and testing the host.
             * It should be called before any network operations are performed.
             **/
            inline static std::future<void> initialize();

            /**
             * @brief Get system proxy settings
             * @return std::string - The system proxy settings, or an empty string if no proxy is set.
             * **/
            static std::optional<std::string> getSysProxy();

            /**
             * @brief Build a complete URL from the given path, host, and protocol.
             * @param path The path to append to the host.
             * @param host The host to use (default is globalConfig.host).
             * @param protocol The protocol to use (default is globalConfig.protocol).
             * @return std::string - The complete URL.
             */
            static std::string buildUrl(const std::string &path, const std::string &host = globalConfig.host, const std::string &protocol = globalConfig.protocol) {
                return (protocol + host + path);
            }

        protected:
            // default callback functions
            static size_t writeToStringCallback(char *ptr, size_t size, size_t nmemb, void *userdata) {
                std::string *buffer = static_cast<std::string *>(userdata);
                buffer->append(ptr, size * nmemb);
                return size * nmemb;
            }
            static size_t writeToFileCallback(char *contents, size_t size, size_t nmemb, void *userp) {
                std::fstream *file = static_cast<std::fstream *>(userp);
                file->write(contents, size * nmemb);
                return size * nmemb;
            }

            // logging functions
            template <typename... Args>
            static void logError(const std::string &message, Args &&...args) {
                logError(message + " " + ((std::string(args) + " ") + ...));
            }
            template <typename... Args>
            static void logInfo(const std::string &message, Args &&...args) {
                logInfo(message + " " + ((std::string(args) + " ") + ...));
            }
            template <typename... Args>
            static void logWarn(const std::string &message, Args &&...args) {
                logWarn(message + " " + ((std::string(args) + " ") + ...));
            }
            template <typename... Args>
            static void logDebug(const std::string &message, Args &&...args) {
                logDebug(message + " " + ((std::string(args) + " ") + ...));
            }
            // Overloaded logging functions for single string messages
            static void logError(const std::string &message);
            static void logInfo(const std::string &message);
            static void logWarn(const std::string &message);
            static void logDebug(const std::string &message);
        };

        /**
         * @brief This structure holds the result of a network request, including status code, content, and error messages.
         * @struct NetworkResult
         */
        struct NetworkResult {

            // The HTTP status code of the response.
            int statusCode = 0;
            // Indicates whether the request encountered an error.
            bool hasError = false;

            // The content of the response, if any.
            std::string content;

            // A brief error message, if an error occurred.
            std::string errorMessage;

            // A more detailed error message, if available.
            std::string detailedErrorMessage;

            /**
             * @brief Check if the request was successful.
             * @return Returns true if the request was successful (status code is between 200 and 299) and no error occurred (hasError is false), otherwise returns false.
             * @note This function checks if the request was successful based on the HTTP status code.
             * @note A successful request does not necessarily mean that the content is valid or as expected.
             * @note Use hasContent() to check if the response contains any content.
             * @see hasContent() for checking if the response contains any content.
             */
            bool isSuccess() const { return !hasError && (statusCode >= 200 && statusCode < 300); }
            /**
             * @brief Check if the response contains any content.
             * @return Returns true if the content is not empty, otherwise returns false.
             * @note This function checks if the response has any content, regardless of whether the request was successful or not.
             * @see isSuccess() for checking if the request was successful.
             */
            bool hasContent() const { return !content.empty(); }

            /**
             * @brief Set an error message for the request result.
             * @note This function is used internally to Network set the error message.
             * @note This function sets the hasError flag to true and updates the errorMessage and detailedErrorMessage fields.
             */
            void setError(const std::string &message, const std::string &detailsMessage = "") {
                hasError = true;
                errorMessage = message;
                if (!detailsMessage.empty()) {
                    detailedErrorMessage = detailsMessage;
                }
            }
        };

        /**
         * @brief This structure holds the configuration for network requests, used to pass parameters to Network.
         * @struct RequestConfig
         * @ingroup network
         */
        struct RequestConfig {

            std::string url;
            NetworkBase::RequestType method = NetworkBase::RequestType::Get;
            std::string userAgent = NetworkBase::globalConfig.userAgent;

            // "true" uses system proxy, or specify a proxy address, leave empty or other values to not use proxy
            std::string proxy = NetworkBase::globalConfig.proxy;

            std::string requestId;

            /**
             * @brief HTTP header for the request.
             * @note This field is used to set custom headers for the request.
             * @note The header should be formatted as "Key: Value" pairs, separated by newlines.
             * @note Example: "Content-Type: application/json\nAuthorization: Bearer token"
             */
            std::string header;

            /**
             * @brief Data to be sent in the body of the request.
             * @note only used for POST requests. otherwise ignored.
             */
            std::string postData;

            /**
             * @brief The fileName field is used to specify the name of the file to be uploaded or downloaded.
             * @note only used for UploadFile and DownloadFile request types.
             * @note For DownloadFile, this field specifies where to save the downloaded file.
             * @note For UploadFile, this field specifies the file to be uploaded.
             */
            std::string fileName;

            /**
             * @brief Whether the download is resumable.
             * @note If true, the request will attempt to resume a previous download if the server supports it.
             * @note This is useful for large files or unreliable connections.
             */
            bool resumable = false;

            /**
             * @brief Range for downloading a file.
             * @note This field is used to specify a byte range for downloading a file.
             * @note The format is "start-end", where start is the starting byte and end is the ending byte.
             * @note Example: "0-1000" means download the first 1000 bytes of the file.
             * @note This feature requires server support for range requests.
             */
            std::string range;

            RequestConfig &setUrl(const std::string &u) {
                url = u;
                return *this;
            }
            RequestConfig &setMethod(NetworkBase::RequestType m) {
                method = m;
                return *this;
            }
            RequestConfig &setUserAgent(const std::string &ua) {
                userAgent = ua;
                return *this;
            }
            RequestConfig &setProxy(const std::string &p) {
                proxy = p;
                return *this;
            }
            RequestConfig &setRequestId(const std::string &id) {
                requestId = id;
                return *this;
            }
            RequestConfig &setHeader(const std::string &h) {
                header = h;
                return *this;
            }
            RequestConfig &setData(const std::string &d) {
                postData = d;
                return *this;
            }
            RequestConfig &setOutputFile(const std::string &f) {
                fileName = f;
                return *this;
            }
            RequestConfig &setResumable(bool r) {
                resumable = r;
                return *this;
            }
            RequestConfig &setRange(const std::string &r) {
                range = r;
                return *this;
            }
        };

        /**
         * @brief This structure holds the configuration for multi-threaded downloads.
         * @struct MultiDownloadConfig
         * @ingroup network
         */
        struct MultiDownloadConfig {
            RequestConfig config;

            /**
             * @brief This enum defines the approach for splitting files during multi-threaded downloads.
             * @enum Approach
             * @ingroup network
             * @note Auto: Automatically determine the best approach based on file size and system capabilities.
             * @note Thread: Split the file based on the number of threads, where segmentParam indicates the number of tasks. 0 means default value (100 tasks).
             * @note Size: Split the file based on the specified size, where segmentParam indicates the size of each segment in bytes. 0 means default value (5MB).
             * @note Quantity: Split the file based on the specified number of segments, where segmentParam indicates the number of segments. 0 means default value (100 segments).
             */
            enum Approach {
                Auto = 0,
                Thread = 1,
                Size = 2,
                Quantity = 3
            };
            Approach approach = Approach::Auto;

            /**
             * @brief The parameter for the splitting approach.
             * @note For Thread approach, this indicates the number of tasks (default is 100).
             * @note For Size approach, this indicates the size of each segment in bytes (default is 5MB).
             * @note For Quantity approach, this indicates the number of segments (default is 100).
             * @note If set to 0, the default value will be used based on the approach.
             */
            size_t segmentParam = 0;

            /**
             * @brief List of HTTP status codes considered successful for the request.
             * @note These status codes will be treated as successful requests, even if there are errors.
             * @note The default value is {200, 206}, which represents HTTP 200 OK and HTTP 206 Partial Content.
             */
            std::vector<int> successCodes = {200, 206};
        };

        /**
         * @class Network
         * @brief Network request handling class that provides various network request methods.
         * @ingroup network
         * @see NetworkBase for the base class that provides common functionality.
         * @see RequestConfig for the struct that defines request parameters.
         * @see NetworkResult for the struct that holds the result of a network request.
         * @note This class uses libcurl for network requests and provides methods for synchronous and asynchronous execution.
         * @note It supports various request types such as GET, HEAD, POST, and file downloads.
         * @note It also provides methods for automatic retries and multi-threaded downloads.
         */
        class Network : public NetworkBase {
        public:
            Network();
            ~Network();

            /**
             * @brief Execute a network request synchronously.
             * @param config The configuration for the request, including URL, method, headers, etc.
             * @return NetworkResult - The result of the network request, including status code, content, and error messages.
             * @note This method performs a network request based on the provided configuration and returns the result.
             * @note If the request fails, the result will contain an error message and status code.
             * @note This method blocks until the request is complete.
             * @see RequestConfig for the struct that defines request parameters.
             * @see NetworkResult for the struct that holds the result of a network request.
             */
            NetworkResult execute(const RequestConfig &config);

            /**
             * @brief Execute a network request asynchronously.
             * @param config The configuration for the request, including URL, method, headers, etc.
             * @return std::future<NetworkResult> - A future that will contain the result of the network request when it completes.
             * @note This method performs a network request based on the provided configuration and returns a future that will be ready when the request is complete.
             * @note The future can be used to retrieve the result of the request without blocking the current thread.
             * @see RequestConfig for the struct that defines request parameters.
             * @see NetworkResult for the struct that holds the result of a network request.
             * @note Example usage:
             * @code
             * RequestConfig config;
             * config.setUrl("https://example.com/api/data")
             *       .setMethod(NetworkBase::RequestType::Get)
             *       .setRequestId("request-123");
             * std::future<NetworkResult> future = network.executeAsync(config);
             * // Do other work while the request is being processed
             * NetworkResult result = future.get(); // This will block until the request is complete
             * if (result.isSuccess()) {
             *     // Process the result
             * } else {
             *     // Handle the error
             *     std::cerr << "Error: " << result.errorMessage << std::endl;
             * }
             * @endcode
             */
            std::future<NetworkResult> executeAsync(const RequestConfig &config);

            /**
             * @brief Execute a network request with retry logic.
             * @param config The configuration for the request, including URL, method, headers, etc.
             * @param maxRetries The maximum number of retry attempts (default is 3).
             * @param retryDelay The delay between retries (default is 150 milliseconds).
             * @param successCodes A list of HTTP status codes that are considered successful (default is {200}).
             * @return NetworkResult - The result of the network request after retries.
             * @note This method will retry the request if it fails, up to the specified number of retries.
             * @note If the request is successful, it will return the result immediately.
             * @note If all retries fail, it will return a result with an error message and status code.
             */
            NetworkResult executeWithRetry(const RequestConfig &config,
                                           int maxRetries = 3,
                                           std::chrono::milliseconds retryDelay = std::chrono::milliseconds(150),
                                           const std::vector<int> &successCodes = {200});

            /**
             * @brief Perform a multi-threaded download of a file.
             * @param config The configuration for the multi-threaded download, including URL, file name, and splitting approach.
             * @return bool - Returns true if the download was successful, false otherwise.
             * @note This method will split the file into segments and download each segment in a separate thread.
             * @note The segments will be merged into a single file after all downloads are complete.
             * @note If any segment fails to download, the method will attempt to retry the failed segments.
             * @note The splitting approach can be specified as Auto, Thread, Size, or Quantity.
             * @note The segmentParam field in the config can be used to specify the number of segments, size of each segment, or number of tasks, depending on the approach.
             * @see MultiDownloadConfig for the struct that defines multi-threaded download parameters.
             */
            bool multiThreadedDownload(const MultiDownloadConfig &config);

            std::optional<size_t> getContentSize(const std::string &url);

            /**
             * @returns the content type of the resource at the given URL.
             * @returns Example: "application/json", "text/html", "image/png", etc.
             */
            std::optional<std::string> getContentType(const std::string &url);

        private:
            NetworkResult doExecute(const RequestConfig &config);

            /**
             * @return If there is an error, return the error message, otherwise return an empty string.
             */
            std::string initCurl(CURL *curl, const RequestConfig &config);

            NetworkResult performRequest(CURL *curl, const RequestConfig &config);
            int getHttpStatusCode(CURL *curl);

            NetworkResult handleGetRequest(CURL *curl, const RequestConfig &config);
            NetworkResult handleHeadRequest(CURL *curl, const RequestConfig &config);
            NetworkResult handlePostRequest(CURL *curl, const RequestConfig &config);
            NetworkResult handleDownloadRequest(CURL *curl, const RequestConfig &config);

            // logging RequestConfig information
            void logRequestInfo(const RequestConfig &config);
        };

    } // namespace network
} // namespace neko