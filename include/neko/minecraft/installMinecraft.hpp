/**
 * @file installMinecraft.hpp
 * @brief Minecraft download and installation
 * @author moehoshio
 * @copyright Copyright (c) 2025 Hoshi
 * @license MIT OR Apache-2.0
 */

#pragma once
#include <neko/schema/types.hpp>

#include "neko/minecraft/downloadSource.hpp"

#include <nlohmann/json.hpp>

namespace neko::minecraft {

    // Should not be called from the main thread, as it will block the incoming thread until completion.
    void setupMinecraftDownloads(
        DownloadSource downloadSource,
        neko::strview versionId,
        const nlohmann::json &versionJson,
        neko::strview installPath = "./.minecraft");

    // Should not be called from the main thread, as it will block the incoming thread until completion.
    void installMinecraft(
        neko::strview installPath = "./.minecraft",
        neko::strview targetVersion = "1.16.5",
        DownloadSource downloadSource = DownloadSource::Official);

} // namespace neko::minecraft
