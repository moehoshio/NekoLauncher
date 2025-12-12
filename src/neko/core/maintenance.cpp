#pragma once

// Neko Module
#include <neko/schema/exception.hpp>
#include <neko/schema/types.hpp>

#include <neko/event/event.hpp>
#include <neko/log/nlog.hpp>

#include <neko/function/utilities.hpp>
#include <neko/system/platform.hpp>

#include <neko/network/network.hpp>

// NekoLc project
#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"

#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/maintenance.hpp"
#include "neko/core/remoteConfig.hpp"
#include "neko/core/downloadPoster.hpp"
#include "neko/core/launcherProcess.hpp"

namespace neko::core {

    MaintenanceInfo checkMaintenance(api::LauncherConfigResponse config) {
        log::autoLog log;
        network::Network net;

        // Update process to checking maintenance status
        std::string process = lang::tr(lang::keys::maintenance::category, lang::keys::maintenance::checkingStatus);
        bus::event::publish(event::ShowLoadingEvent(ui::LoadingMsg{
            .type = ui::LoadingMsg::Type::OnlyRaw,
            .process = process}));

        nlohmann::json maintenanceRequest = app::getRequestJson("maintenanceRequest");
        auto url = network::buildUrl(lc::api::maintenance);

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .postData = maintenanceRequest.dump()};
        network::RetryConfig retryConfig{
            .config = reqConfig,
            .maxRetries = config.maxRetryCount,
            .retryDelay = std::chrono::seconds(config.retryIntervalSec),
            .successCodes = {200, 204}};
        auto result = net.executeWithRetry(retryConfig);

        if (!result.hasError && result.statusCode == 204) {
            return {.isMaintenance = false};
        }

        if (!result.isSuccess() || !result.hasContent()) {
            throw ex::NetworkError("Failed to check maintenance status: " + result.errorMessage);
        }

        // if has content , then parse it
        std::string response = result.content;

        // Update process to parsing json
        process = lang::tr(lang::keys::maintenance::category, lang::keys::maintenance::parseIng);
        bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = process});
        log::info("maintenance response : " + response);

        try {

            auto root = nlohmann::json::parse(response);
            auto jsonData = root.at("maintenanceResponse");
            if (root.contains("meta") && root.at("meta").is_object()) {
                jsonData["meta"] = root.at("meta");
            }
            api::MaintenanceResponse maintenanceInfo = jsonData.get<api::MaintenanceResponse>();

            maintenanceInfo.message = lang::trWithReplaced(
                lang::keys::maintenance::category,
                lang::keys::maintenance::message,
                {{"{startTime}", maintenanceInfo.startTime},
                 {"{exEndTime}", maintenanceInfo.exEndTime},
                 {"{description}", maintenanceInfo.message}});

            // Update process to downloading poster
            process = lang::tr(lang::keys::maintenance::category, lang::keys::maintenance::downloadPoster);
            bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = process});
            auto filePath = downloadPoster(maintenanceInfo.posterUrl);

            std::string command;
            if (!maintenanceInfo.link.empty()) {
                if constexpr (system::isWindows()) {
                    command = "start \"\" \"" + maintenanceInfo.link + "\""; // empty title then URL
                } else if constexpr (system::isLinux()) {
                    command = "xdg-open \"" + maintenanceInfo.link + "\"";
                } else if constexpr (system::isMacOS()) {
                    command = "open \"" + maintenanceInfo.link + "\"";
                }
            }

            neko::ui::NoticeMsg notice{
                .title = lang::tr(lang::keys::maintenance::category, lang::keys::maintenance::title, "Maintenance"),
                .message = maintenanceInfo.message,
                .posterPath = filePath.value_or("")};

            const bool inProgress = maintenanceInfo.isMaintenance();
            const bool scheduled = maintenanceInfo.isScheduled();

            if (inProgress) {
                if (!command.empty()) {
                    notice.buttonText = {
                        lang::tr(lang::keys::button::category, lang::keys::button::open, "Open"),
                        lang::tr(lang::keys::button::category, lang::keys::button::quit, "Quit")};
                    notice.callback = [command](neko::uint32 index) {
                        if (index == 0) {
                            try {
                                launcherNewProcess(command);
                            } catch (const std::exception &e) {
                                log::error(std::string("Failed to open maintenance link: ") + e.what());
                            }
                        }
                        // Always quit on progress maintenance
                        app::quit();
                    };
                } else {
                    // No link: force quit only
                    notice.buttonText = {
                        lang::tr(lang::keys::button::category, lang::keys::button::quit, "Quit")};
                    notice.callback = [](neko::uint32 /*index*/) {
                        app::quit();
                    };
                }
                notice.defaultButtonIndex = static_cast<neko::uint32>(notice.buttonText.size() > 1 ? 1 : 0);
            } else {
                // Scheduled or other statuses: do not force quit; keep explicit Quit option
                if (!command.empty()) {
                    notice.buttonText = {
                        lang::tr(lang::keys::button::category, lang::keys::button::open, "Open"),
                        lang::tr(lang::keys::button::category, lang::keys::button::quit, "Quit")};
                    notice.callback = [command](neko::uint32 index) {
                        if (index == 0) {
                            try {
                                launcherNewProcess(command);
                            } catch (const std::exception &e) {
                                log::error(std::string("Failed to open maintenance link: ") + e.what());
                            }
                        } else if (index == 1) {
                            app::quit();
                        }
                    };
                } else {
                    notice.buttonText = {
                        lang::tr(lang::keys::button::category, lang::keys::button::close, "Close"),
                        lang::tr(lang::keys::button::category, lang::keys::button::quit, "Quit")};
                    notice.callback = [](neko::uint32 index) {
                        if (index == 1) {
                            app::quit();
                        }
                    };
                }
            }

            // Notify listeners about maintenance state; subscribers can choose how to display
            bus::event::publish(event::MaintenanceEvent(notice));

            return {
                .isMaintenance = inProgress || scheduled,
                .message = maintenanceInfo.message,
                .posterPath = filePath.value_or(""),
                .openLinkCmd = command};
        } catch (nlohmann::json::parse_error &e) {
            throw ex::Parse("Failed to parse json: " + std::string(e.what()));
        } catch (const nlohmann::json::out_of_range &e) {
            throw ex::OutOfRange("Json key not found: " + std::string(e.what()));
        }
        
        return {.isMaintenance = false};
    }

} // namespace neko::core
