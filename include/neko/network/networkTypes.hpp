#pragma once

#include "neko/schema/types.hpp"

#include <string>
#include <vector>
#include <chrono>
#include <functional>

namespace neko::network {

    enum class RequestType {
        Get,
        Head,
        Post,
        DownloadFile,
        UploadFile
    };
    /**
     * @brief This structure holds the result of a network request, including status code, content, and error messages.
     * @struct NetworkResult
     */
    template <typename T>
    struct NetworkResult {

        // The HTTP status code of the response.
        int statusCode = 0;
        // Indicates whether the request encountered an error.
        bool hasError = false;

        // The content of the response, if any.
        // If this is a downloadFile request, then content will be empty.
        T content;

        // A brief error message, if an error occurred.
        std::string errorMessage;

        // A more detailed error message, if available.
        std::string detailedErrorMessage;

        /**
         * @brief Check if the request was successful.
         * @return Returns true if the request was successful (status code is between 200 and 299) and no error occurred (hasError is false), otherwise returns false.
         * @note A successful request does not necessarily mean that the content is valid or as expected.
         * @note Use hasContent() to check if the response contains any content.
         */
        bool isSuccess() const { return !hasError && (statusCode >= 200 && statusCode < 300); }

        /**
         * @brief Check if the response contains any content.
         * @return Returns true if the content is not empty, otherwise returns false.
         */
        bool hasContent() const { return !content.empty(); }

        /**
         * @brief Set an error message for the request result.
         * @note This function is used internally to Network set the error message.
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
        RequestType method = RequestType::Get;
        std::string userAgent = config::globalConfig.getUserAgent();

        // "true" uses system proxy, or specify a proxy address, leave empty or other values to not use proxy
        std::string proxy = config::globalConfig.getProxy();

        std::string requestId;

        /**
         * @brief HTTP header for the request.
         * @note This field is used to set custom headers for the request.
         * @note The header should be formatted as "Key: Value" pairs, separated by newlines.
         * @note Example: "Content-Type: application/json \nAuthorization: Bearer token"
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

        /**
         * @brief Callback function invoked each time data is received. Can be used to calculate download progress. The parameter is usually in bytes.
         */
        std::function<void(neko::uint64)> progressCallback = nullptr;
    };

    /**
     * @brief This structure holds the configuration for retrying failed network requests.
     * @struct RetryConfig
     * @ingroup network
     */
    struct RetryConfig {

        RequestConfig config;
        /**
         * @brief Maximum number of retry attempts for the request.
         * @note Default is 3 retries.
         */
        int maxRetries = 3;
        /**
         * @brief Delay between retry attempts.
         * @note Default is 150 milliseconds.
         */
        std::chrono::milliseconds retryDelay{150};
        /**
         * @brief List of HTTP status codes considered successful for the request.
         * @note These status codes will be treated as successful requests, even if there are errors.
         * @note The default value is {200, 204}, which represents HTTP 200 OK and HTTP 204 No Content.
         */
        std::vector<int> successCodes = {200, 204};
    };

    /**
     * @brief This structure holds the configuration for multi-threaded downloads.
     * @struct MultiDownloadConfig
     * @ingroup network
     */
    struct MultiDownloadConfig {
        RequestConfig config;

        /**
         * @brief The approach to splitting the file for multi-threaded download.
         * @note This field determines how the file will be split into segments for downloading.
         * @enum Approach
         * @ingroup network
         */
        enum Approach {
            /**
             * @note Auto: Automatically determine the best approach based on file size and system capabilities.
             */
            Auto = 0,
            /**
             * @note Thread: Split the file based on the number of threads, where segmentParam indicates the number of tasks. 0 means default value (100 tasks).
             */
            Thread = 1,
            /**
             * @note Size: Split the file based on the specified size, where segmentParam indicates the size of each segment in bytes. 0 means default value (5MB).
             */
            Size = 2,
            /**
             * @note Quantity: Split the file based on the specified number of segments, where segmentParam indicates the number of segments. 0 means default value (100 segments).
             */
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
        neko::uint64 segmentParam = 0;

        /**
         * @brief List of HTTP status codes considered successful for the request.
         * @note These status codes will be treated as successful requests, even if there are errors.
         * @note The default value is {200, 206}, which represents HTTP 200 OK and HTTP 206 Partial Content.
         */
        std::vector<int> successCodes = {200, 206};
    };

} // namespace neko::network
