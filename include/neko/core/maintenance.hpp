#pragma once

// Neko Module
#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>
#include <neko/log/nlog.hpp>
#include <neko/event/event.hpp>
#include <neko/function/utilities.hpp>
#include <neko/system/platform.hpp>
#include <neko/network/network.hpp>
#include <neko/network/networkCommon.hpp>

// NekoLc project
#include "neko/function/lang.hpp"
#include "neko/function/info.hpp"
#include "neko/schema/eventTypes.hpp"

namespace neko::core {

    struct MaintenanceInfo {
        bool isMaintenance = false;
        std::string message;
        std::string poster;      // file path
        std::string openLinkCmd; // command to open the link, e.g., "open https://example.com"
    };

    /**
     * @brief Checks if the application is under maintenance.
     * @return MaintenanceInfo containing maintenance status and details
     * @throws ex::NetworkError if the network request fails
     * @throws ex::Parse if the response cannot be parsed
     * @throws ex::OutOfRange if a required key is missing in the response
     * @note This function publishes loading events to the event bus to update the UI.
     */
    inline MaintenanceInfo checkMaintenance() {
        log::autoLog log;
        network::Network net;

        bus::event::publish(event::ShowLoadEvent(ui::LoadMsg{
            .type = ui::LoadMsg::Type::OnlyRaw,
            .process = lang::tr(lang::keys::maintenance::infoRequest)}));

        nlohmann::json maintenanceRequest = info::getRequestJson("maintenanceRequest");
        auto url = network::buildUrl(network::NetworkBase::Api::maintenance);

        network::RequestConfig reqConfig{
            .url = url,
            .method = network::RequestType::Post,
            .data = maintenanceRequest.dump(),
            .requestId = "Maintenance-" + util::random::generateRandomString(6),
            .header = network::header::jsonContentHeader};
        network::RetryConfig retryConfig{
            .config = reqConfig,
            .maxRetries = 5,
            .retryDelay = {150},
            .successCodes = {200, 204}};
        auto result = net.executeWithRetry(retryConfig);

        if (!result.hasError && result.statusCode == 204) {
            return {};
        }

        if (!result.isSuccess() || !result.hasContent()) {
            throw ex::NetworkError("Failed to check maintenance status: " + result.errorMessage);
        }

        std::string response = result.content;
        bus::event::publish(event::UpdateLoadingNowEvent{
            .process = lang::tr(lang::keys::Loading::maintenanceInfoParse)});
        log::info({}, "maintenance response : {}", response);

        try {

            auto jsonData = nlohmann::json::parse(response).at("maintenanceResponse");
            schema::MaintenanceResponse maintenanceInfo = jsonData.get<schema::MaintenanceResponse>();
            maintenanceInfo.message = lang::withPlaceholdersReplaced(
                lang::tr(lang::keys::Maintenance::message),
                {{"{startTime}", maintenanceInfo.startTime},
                 {"{exEndTime}", maintenanceInfo.endTime},
                 {"{msg}", maintenanceInfo.message}});

            bus::event::publish(event::UpdateLoadingNowEvent{
                .process = lang::tr(lang::keys::Loading::downloadMaintenancePoster)});
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
                .poster = filePath.value_or(""),
                .openLinkCmd = command};
        } catch (nlohmann::json::parse_error &e) {
            log::error({}, "Failed to parse json: %s", e.what());
            throw ex::Parse("Failed to parse json: " + std::string(e.what()));
        } catch (const nlohmann::json::out_of_range &e) {
            log::error({}, "Json key not found: {}", e.what());
            throw ex::OutOfRange("Json key not found: " + std::string(e.what()));
        }
        return {};
    }

} // namespace neko::core
