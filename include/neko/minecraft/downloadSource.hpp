#pragma once

#include <neko/log/nlog.hpp>

#include <string>
#include <string_view>
#include <map>
#include <vector>

namespace neko::minecraft {

    enum class DownloadSource {
        Official,
        BMCLAPI
    };
    inline std::map<DownloadSource, std::string_view> downloadSourceMap = {
        {DownloadSource::Official, "Official"},
        {DownloadSource::BMCLAPI, "BMCLAPI"}};

    inline std::string getMinecraftListUrl(DownloadSource downloadSource = DownloadSource::Official) {
        nlog::autoLog log;

        static const std::map<DownloadSource, std::string_view> urlMap = {
            {DownloadSource::Official, "https://piston-meta.mojang.com/mc/game/version_manifest.json"},
            {DownloadSource::BMCLAPI, "https://bmclapi2.bangbang93.com/mc/game/version_manifest.json"}};

        auto it = urlMap.find(downloadSource);
        if (it != urlMap.end()) {
            return std::string(it->second);
        }
        return std::string(urlMap.at(DownloadSource::Official));
    }

    inline std::string replaceWithBMCLAPI(const std::string &url) {
        std::string newUrl = url;
        const std::vector<std::pair<std::string, std::string>> mojangUrls = {
            {"https://piston-meta.mojang.com", "https://bmclapi2.bangbang93.com"},
            {"https://launchermeta.mojang.com", "https://bmclapi2.bangbang93.com"},
            {"https://launcher.mojang.com", "https://bmclapi2.bangbang93.com"},
            {"https://libraries.minecraft.net", "https://bmclapi2.bangbang93.com/maven"},
            {"https://resources.download.minecraft.net", "https://bmclapi2.bangbang93.com/assets"},
            {"https://files.minecraftforge.net", "https://bmclapi2.bangbang93.com/maven"},
            {"https://maven.minecraftforge.net", "https://bmclapi2.bangbang93.com/maven"},
            {"https://launchermeta.mojang.com/v1/products/java-runtime", "https://bmclapi2.bangbang93.com/v1/products/java-runtime"},
            {"http://dl.liteloader.com/versions/versions.json", "https://bmclapi.bangbang93.com/maven/com/mumfrey/liteloader/versions.json"}};

        for (const auto &oldUrl : mojangUrls) {
            size_t pos = newUrl.find(oldUrl.first);
            if (pos != std::string::npos) {
                newUrl.replace(pos, oldUrl.first.length(), oldUrl.second);
            }
        }

        return newUrl;
    }
} // namespace neko::minecraft
