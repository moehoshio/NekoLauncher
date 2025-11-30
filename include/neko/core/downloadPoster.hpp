#pragma once

// Neko Modules
#include <neko/network/network.hpp>
#include <neko/network/networkTypes.hpp>

#include <neko/system/platform.hpp>
#include <neko/function/utilities.hpp>

#include <optional>
#include <string>

namespace neko::core {
    
    // if successful, return file name
    inline std::optional<std::string> downloadPoster(const std::string &url) noexcept {
        
        if (!url.empty() && util::check::isUrl(url)) {
            network::Network net;
            auto fileName = system::tempFolder() + "/poster_" + util::random::generateRandomString(12) + ".png";
            network::RequestConfig reqConfig{
                .url = url,
                .method = network::RequestType::DownloadFile,
                .requestId = "download-poster-" + util::random::generateRandomString(6),
                .fileName = fileName
            };

            auto res = net.execute(reqConfig);

            if (res.isSuccess()) {
                return fileName;
            }
        }
        return std::nullopt;
    }
} // namespace neko::core
