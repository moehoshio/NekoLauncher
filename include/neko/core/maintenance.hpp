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
#include "neko/app/appinfo.hpp"
#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"

#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/downloadPoster.hpp"

namespace neko::core {

    struct MaintenanceInfo {
        bool isMaintenance = false;
        std::string message;
        std::string posterPath;
        std::string openLinkCmd; // command to open the link, e.g., "open https://example.com"
    };

    /**
     * @brief Checks if the application is under maintenance.
     * @return MaintenanceInfo containing maintenance status and details
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     * @throws ex::OutOfRange if a required key is missing in the response
     * @note This function publishes events to update the UI about the maintenance check process.
     */
    inline MaintenanceInfo checkMaintenance() {
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
            .maxRetries = 5,
            .retryDelay = std::chrono::milliseconds(150),
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
        bus::event::publish(event::UpdateLoadingStatusEvent{.process = process});
        log::info("maintenance response : " + response);

        try {

            auto jsonData = nlohmann::json::parse(response).at("maintenanceResponse");
            api::MaintenanceResponse maintenanceInfo = jsonData.get<api::MaintenanceResponse>();

            maintenanceInfo.message = lang::trWithReplaced(
                lang::keys::maintenance::category,
                lang::keys::maintenance::message,
                {{"{startTime}", maintenanceInfo.startTime},
                 {"{exEndTime}", maintenanceInfo.exEndTime},
                 {"{description}", maintenanceInfo.message}});

            // Update process to downloading poster
            process = lang::tr(lang::keys::maintenance::category, lang::keys::maintenance::downloadPoster);
            bus::event::publish(event::UpdateLoadingStatusEvent{.process = process});
            auto filePath = downloadPoster(maintenanceInfo.posterUrl);

            std::string command;
            if constexpr (system::isWindows()) {
                command = "start " + maintenanceInfo.link;
            } else if constexpr (system::isLinux()) {
                command = "xdg-open " + maintenanceInfo.link;
            } else if constexpr (system::isMacOS()) {
                command = "open " + maintenanceInfo.link;
            }

            return {
                .isMaintenance = true,
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
