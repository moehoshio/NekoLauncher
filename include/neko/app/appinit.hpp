/**
 * @file appinit.hpp
 * @brief Application initialization functions for NekoLauncher.
 */

#pragma once

#include <neko/log/nlog.hpp>
#include <neko/schema/types.hpp>

#include <neko/function/utilities.hpp>
#include <neko/function/uuid.hpp>
#include <neko/network/network.hpp>
#include <neko/network/networkCommon.hpp>
#include <neko/system/platform.hpp>

#include "neko/app/appinfo.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/app/lang.hpp"
#include "neko/app/appSubscribe.hpp"

#include "neko/core/coreSubscribe.hpp"

#include "neko/minecraft/minecraftSubscribe.hpp"

#include "neko/ui/uiSubscribe.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/threadBus.hpp"

#include <sstream>

namespace neko::app::init {

    inline void initDeviceID() {
        auto cfg = bus::config::getClientConfig();
        if (!cfg.main.deviceID.empty()) {
            return;
        }

        bus::config::updateClientConfig([](neko::ClientConfig &c) {
            c.main.deviceID = util::uuid::uuidV4();
            log::info("Device ID not set, generating new one: " + c.main.deviceID);
        });
        bus::config::save(app::getConfigFileName());
    }

    inline void initLog() {

        std::string logDir = "logs";
        std::string logExtName = ".log";
        std::string debugLogName = "debug.log";
        std::string newDebugLogName = "new-debug.log";

        auto cfg = bus::config::getClientConfig();
        bool
            dev = cfg.dev.enable,
            debug = cfg.dev.debug;

        if (std::filesystem::exists(logDir) && !std::filesystem::is_directory(logDir)) {
            std::filesystem::remove(logDir);
        }

        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(logDir);
        }

        if (!dev) {
            log::setLevel(log::Level::Warn);
            auto fileName = util::math::sum(logDir, "/", util::time::getLocalTimeString("%Y-%m-%d-%H-%M-%S").value(), logExtName);
            log::addFileAppender(fileName, false);

        } else if (dev && !debug) {
            log::setLevel(log::Level::Info);
            auto fileName = util::math::sum(logDir, "/", util::time::getLocalTimeString("%Y-%m-%d-%H-%M-%S").value(), logExtName);
            log::addFileAppender(fileName, false);

        } else if (dev && debug) {
            log::setLevel(log::Level::Debug);
            log::addFileAppender(util::math::sum(logDir, "/", debugLogName), false);
            log::addFileAppender(util::math::sum(logDir, "/", newDebugLogName), true);
        }
    }

    inline void initThreads() {

        auto cfg = bus::config::getClientConfig();
        neko::uint64 threadCount = cfg.net.thread > 0 ? cfg.net.thread : std::thread::hardware_concurrency();
        bus::thread::setThreadCount(threadCount);

        log::setCurrentThreadName("Main Thread");
        log::info("Initializing thread pool with {} threads", {}, threadCount);

        auto ids = bus::thread::getWorkerIds();

        // Set thread names for each worker thread
        for (auto i : ids) {
            std::string i_str = std::to_string(i);
            std::string threadName = "Worker Thread " + i_str;
            try {
                bus::thread::submitToWorker(i, [threadName]() {
                    log::setCurrentThreadName(threadName);
                    log::info("Hello thread " + threadName);
                });
            } catch (const ex::OutOfRange &e) {
                log::error("Not Found Worker Thread {}", {}, i_str);
            }
        }
    }

    inline void initSystem() {
        using namespace neko::ops::pipe;

        ClientConfig cfg = bus::config::getClientConfig();
        
        if (!cfg.other.tempFolder.empty() && std::filesystem::is_directory(cfg.other.tempFolder)) {
            system::tempFolder(cfg.other.tempFolder | util::unifiedPath);
        }
    }

    inline void configInfoPrint(const ClientConfig &config) {

        log::info("config main : lang : {} , backgroundType : {} , background : {} , windowSize : {} , launcherMethod : {} , useSysWindowFrame: {} , headBarKeepRight : {} , deviceID : {} , resourceVersion : {}",
                  {},
                  config.main.lang,
                  config.main.backgroundType,
                  config.main.background,
                  config.main.windowSize,
                  config.main.launcherMethod,
                  util::logic::boolTo<neko::cstr>(config.main.useSysWindowFrame),
                  util::logic::boolTo<neko::cstr>(config.main.headBarKeepRight),
                  config.main.deviceID,
                  config.main.resourceVersion);

        log::info("config net : thread : {} , proxy : {}",
                  {},
                  config.net.thread,
                  config.net.proxy);

        log::info("config style : blurEffect : {} , blurRadius : {} , fontPointSize : {} , fontFamilies : {}",
                  {},
                  config.style.blurEffect,
                  config.style.blurRadius,
                  config.style.fontPointSize,
                  config.style.fontFamilies);

        log::info("config dev : enable : {} , debug : {} , server : {} , tls : {}",
                  {},
                  util::logic::boolTo<neko::cstr>(config.dev.enable),
                  util::logic::boolTo<neko::cstr>(config.dev.debug),
                  config.dev.server,
                  util::logic::boolTo<neko::cstr>(config.dev.tls));

        std::string accToken = config.minecraft.accessToken;
        std::string maskToken = accToken.empty() ? "null" : "**********" + accToken.substr(accToken.size() - 4); // Hide accessToken for security
        log::info("config minecraft : folder : {} , javaPath : {} , account : {} , name : {} , uuid : {} , accessToken : {} , targetVersion : {} , maxMemory : {} , minMemory : {} , needMemory : {} , authlibName : {} , authlibPrefetched : {} , authlibSha256 : {} , tolerantMode : {} , customResolution : {} , joinServerAddress : {} , joinServerPort : {}",
                  {},
                  config.minecraft.minecraftFolder,
                  config.minecraft.javaPath,
                  config.minecraft.account,
                  config.minecraft.playerName,
                  config.minecraft.uuid,
                  maskToken.c_str(),
                  config.minecraft.targetVersion,
                  config.minecraft.maxMemoryLimit,
                  config.minecraft.minMemoryLimit,
                  config.minecraft.needMemoryLimit,
                  config.minecraft.authlibName,
                  config.minecraft.authlibPrefetched,
                  config.minecraft.authlibSha256,
                  util::logic::boolTo<neko::cstr>(config.minecraft.tolerantMode),
                  config.minecraft.customResolution,
                  config.minecraft.joinServerAddress,
                  config.minecraft.joinServerPort);

        log::info("config other : temp : {} ",
                  {},
                  config.other.tempFolder);
    }

    inline void initLanguage() {
        auto cfg = bus::config::getClientConfig();
        lang::language(cfg.main.lang);
    }

    inline void initNetwork() {
        auto cfg = bus::config::getClientConfig();
        network::initialize([cfg](network::config::NetConfig &config) {
            std::string proxy(cfg.net.proxy);
            bool
                dev = cfg.dev.enable,
                tls = cfg.dev.tls,
                proxyAddressInvalid = util::logic::allTrue((proxy != ""), (proxy != "true"), !util::check::isProxyAddress(proxy));
            if (proxyAddressInvalid)
                proxy = "";
            std::stringstream userAgentStream;
            userAgentStream << "NekoLc/" << app::getVersion() << " ("
                            << system::getOsName() << "; Build " << app::getBuildId() << ")";
            config
                .setUserAgent(userAgentStream.str())
                .setProxy(proxy)
                .setProtocol((dev && !tls) ? "http://" : "https://");
            log::info("Network initialized with UserAgent: {}, Proxy: {}, Protocol: {}", {}, config.getUserAgent(), config.getProxy(), config.getProtocol());

            if (cfg.dev.enable && std::string(cfg.dev.server) != "auto" && util::check::isUrl(network::buildUrl("/path",cfg.dev.server)) ){
                config.pushAvailableHost(std::string(cfg.dev.server));
                log::info("Network::initialize() : Developer mode enabled, using custom server host: {}", {}, cfg.dev.server);
                return;
            }

            log::info("Network::initialize : Starting test hosts...");

            for (auto it : lc::NetworkHostList) {
                neko::network::Network net;
                std::string url = network::buildUrl(lc::api::testing, it.data());

                network::RetryConfig cfg{
                    .config = neko::network::RequestConfig{
                        .url = url,
                        .method = neko::network::RequestType::Get,
                        .userAgent = config.getUserAgent(),
                        .proxy = config.getProxy(),
                        .requestId = "Testing - " + std::string(it)},
                    .maxRetries = 2,
                    .retryDelay = std::chrono::milliseconds{50},
                    .successCodes = {200}};

                auto result = net.executeWithRetry(cfg);

                if (result.isSuccess()) {
                    neko::log::info("Network::initialize() : Testing host available, host: {} , statusCode: {}", {}, it, result.statusCode);
                    config.pushAvailableHost(std::string(it));
                    continue;
                }

                neko::log::warn("Network::initialize() : Testing host failed, host: {}, statusCode: {}, errorMessage: {}", {}, it, std::to_string(result.statusCode), result.errorMessage);
            }
        });
    }

    inline void initialize() {
        bus::config::load(app::getConfigFileName());

        initLog();
        initThreads();

        initDeviceID();
        initLanguage();

        configInfoPrint(bus::config::getClientConfig());

        initNetwork();

        app::subscribeToAppEvent();
        core::subscribeToCoreEvents();
        minecraft::subscribeToMinecraftEvents();
        ui::subscribeToUiEvent();
    }
} // namespace neko::app::init
