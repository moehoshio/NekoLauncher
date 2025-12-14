#include <neko/log/nlog.hpp>
#include <neko/schema/exception.hpp>

#include <neko/system/platform.hpp>

#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/core/crashReporter.hpp"
#include "neko/core/feedback.hpp"

#include "neko/bus/configBus.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

namespace fs = std::filesystem;

namespace neko::core::crash {

    namespace {
        constexpr std::size_t kMaxLogBytesPerFile = 512 * 1024; // cap per file to avoid giant payloads

        std::string readFileLimited(const fs::path &path, bool &truncated) {
            std::ifstream ifs(path, std::ios::binary);
            if (!ifs.is_open()) {
                throw ex::FileError("Failed to open log file: " + path.string());
            }
            std::string data;
            data.reserve(kMaxLogBytesPerFile);

            char buf[4096];
            std::size_t readTotal = 0;
            truncated = false;
            while (ifs.good() && readTotal < kMaxLogBytesPerFile) {
                const std::size_t toRead = std::min<std::size_t>(sizeof(buf), kMaxLogBytesPerFile - readTotal);
                ifs.read(buf, static_cast<std::streamsize>(toRead));
                auto got = static_cast<std::size_t>(ifs.gcount());
                data.append(buf, got);
                readTotal += got;
                if (got == 0) {
                    break;
                }
            }

            truncated = (readTotal >= kMaxLogBytesPerFile) && ifs.good();
            return data;
        }
    } // namespace

    bool markRunStart() {
        auto cfg = bus::config::getClientConfig();
        const bool previous = cfg.other.lastRunUnclean;
        bus::config::updateClientConfig([](neko::ClientConfig &c) {
            c.other.lastRunUnclean = true;
        });
        bus::config::save(app::getConfigFileName());
        return previous;
    }

    void markCleanShutdown() {
        bus::config::updateClientConfig([](neko::ClientConfig &c) {
            c.other.lastRunUnclean = false;
        });
        bus::config::save(app::getConfigFileName());
    }

    void uploadLogsIfNeeded(bool previousRunUnclean) {
        if (!previousRunUnclean) {
            return;
        }

        fs::path logDir = system::workPath() + "/logs";
        if (!fs::exists(logDir) || !fs::is_directory(logDir)) {
            log::warn("Last run was unclean but log directory missing: {}", {}, logDir.string());
            return;
        }

        std::string feedbackBody;
        for (const auto &entry : fs::directory_iterator(logDir)) {
            if (!entry.is_regular_file() || entry.path().extension() != ".log") {
                continue;
            }
            try {
                bool truncated = false;
                const auto content = readFileLimited(entry.path(), truncated);
                feedbackBody += "===== " + entry.path().filename().string() + " (" + std::to_string(content.size()) + " bytes";
                if (truncated) {
                    feedbackBody += ", truncated to " + std::to_string(kMaxLogBytesPerFile) + " bytes";
                }
                feedbackBody += ") =====\n";
                feedbackBody += content;
                feedbackBody += "\n\n";
            } catch (const std::exception &e) {
                log::warn("Failed to read log {}: {}", {}, entry.path().string(), e.what());
            }
        }

        if (feedbackBody.empty()) {
            log::warn("Crash feedback skipped: no log files found");
            return;
        }

        // Prevent oversized payloads: cap final body to ~256 KB
        constexpr std::size_t kMaxPayloadBytes = 256 * 1024;
        if (feedbackBody.size() > kMaxPayloadBytes) {
            feedbackBody.resize(kMaxPayloadBytes);
            feedbackBody += "\n[truncated]\n";
        }

        feedbackBody = std::string{"Previous run did not exit cleanly.\n"} + feedbackBody;

        try {
            core::feedbackLog(feedbackBody);
            log::info("Crash feedback submitted (logs attached)");
        } catch (const std::exception &e) {
            log::error("Crash feedback failed: {}", {}, e.what());
        }
    }

} // namespace neko::core::crash
