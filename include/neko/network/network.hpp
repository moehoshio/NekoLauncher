/**
 * @file network.hpp
 * @brief Network module header file
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once

// Neko Module
#include "neko/schema/nekodefine.hpp"
#include "neko/schema/types.hpp"

#include "neko/network/networkCommon.hpp"
#include "neko/network/networkTypes.hpp"

// C++ STL
#include <string>
#include <vector>

#include <future>
#include <memory>
#include <optional>

#include <functional>
#include <optional>

/**
 * @defgroup network Network Module
 * @namespace neko::network
 */
namespace neko::network {

    using CURL = void; // Placeholder for the actual CURL type, which is defined in the libcurl library.

    /**
     * @class Network
     * @brief Network request handling class that provides various network request methods.
     * @ingroup network
     */
    class Network {
    public:
        Network(
            std::shared_ptr<executor::IAsyncExecutor> executor = executor::createExecutor(),
            std::shared_ptr<log::ILogger> logger = log::createLogger());

        ~Network();

        /**
         * @brief Execute a network request synchronously.
         * @param config The configuration for the request
         * @return NetworkResult<T> - The result of the network request
         * @note This method blocks until the request is complete.
         */
        template <typename T = std::string>
        NetworkResult<T> execute(const RequestConfig &config);

        /**
         * @brief Execute a network request asynchronously.
         * @param config The configuration for the request
         * @return std::future<NetworkResult<T>> - A future that will contain the result of the network request when it completes.
         */
        template <typename T = std::string>
        std::future<NetworkResult<T>> executeAsync(const RequestConfig &config);

        /**
         * @brief Execute a network request with retry logic.
         * @param config The configuration for the request, including retry settings.
         * @return NetworkResult<T> - Returns the result of the first successful request, or the result of the last retry attempt if all retries fail.
         * @see RetryConfig for the struct that defines retry settings.
         */
        template <typename T = std::string>
        NetworkResult<T> executeWithRetry(const RetryConfig &config);

        /**
         * @brief Perform a multi-threaded download of a file.
         * @param config The configuration for the multi-threaded download
         * @return bool - Returns true if the download was successful, false otherwise.
         * @note This method will split the file into segments and download each segment in a separate thread.
         * @note If the server does not support range requests, the download will fail.
         * @see MultiDownloadConfig for the struct that defines multi-threaded download parameters.
         */
        bool multiThreadedDownload(const MultiDownloadConfig &config);

        /**
         * @brief Find the URL header for a given URL.
         * @param url The URL to check
         * @param headerName The name of the header to find (default is "Content-Type")
         * @return std::optional<T> - Returns the value of the header if found,
         *                             or std::nullopt if the header is not found.
         * @note headerName is case-insensitive.
         */
        template <typename T = std::string>
        std::optional<T> findUrlHeader(const std::string &url, const std::string &headerName = "Content-Type");

        /**
         * @returns the content type of the resource at the given URL.
         * @returns Example: "application/json", "text/html", "image/png", etc.
         */
        std::optional<std::string> getContentType(const std::string &url);
        std::optional<neko::uint64> getContentSize(const std::string &url);

    private:
        std::shared_ptr<log::ILogger> logger;
        std::shared_ptr<executor::IAsyncExecutor> executor;

        // === Internal methods ===

        template <typename T = std::string>
        NetworkResult<T> doExecute(const RequestConfig &config);

        /**
         * @return If there is an error, return the error message, otherwise return an std::nullopt.
         */
        std::optional<std::string> initCurl(CURL *curl, const RequestConfig &config);

        template <typename T = std::string>
        NetworkResult<T> performRequest(CURL *curl, const RequestConfig &config);
        int getHttpStatusCode(CURL *curl);

        template <typename T = std::string>
        NetworkResult<T> handleGetRequest(CURL *curl, const RequestConfig &config);
        template <typename T = std::string>
        NetworkResult<T> handleHeadRequest(CURL *curl, const RequestConfig &config);
        template <typename T = std::string>
        NetworkResult<T> handlePostRequest(CURL *curl, const RequestConfig &config);
        template <typename T = std::string>
        NetworkResult<T> handleDownloadRequest(CURL *curl, const RequestConfig &config);

        void logError(const std::string &message);
        void logInfo(const std::string &message);
        void logWarn(const std::string &message);
        void logDebug(const std::string &message);

        // logging RequestConfig information
        void logRequestInfo(const RequestConfig &config);
    };

} // namespace neko::network