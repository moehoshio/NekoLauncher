#pragma once

#include <neko/schema/types.hpp>
#include <neko/log/nlog.hpp>
#include <neko/function/utilities.hpp>
#include <neko/network/networkCommon.hpp>

#include "neko/schema/clientconfig.hpp"
#include "neko/core/appinfo.hpp"
#include "neko/core/configBus.hpp"
#include "neko/core/threadBus.hpp"
#include "neko/function/lang.hpp"


#include <sstream>

namespace neko::core::app::init {

    inline void initDeviceID() {
        auto cfg = bus::config::getClientConfig();
        std::string deviceID = cfg.main.deviceID ? cfg.main.deviceID : "";
        if (!deviceID.empty()) {
            return;
        }

        bus::config::updateClientConfig([&deviceID](neko::ClientConfig &cfg) {
            deviceID = util::uuid::uuidV4();
            cfg.main.deviceID = deviceID.c_str();
            log::info({}, "Device ID not set, generating new one: {}", deviceID);
        });
        bus::config::save(app::getConfigFileName());
    }

    inline void initLog() {

        auto cfg = bus::config::getClientConfig();
        bool
            dev = cfg.dev.enable,
            debug = cfg.dev.debug;
        if (!std::filesystem::exists("logs")) {
            std::filesystem::create_directory("logs");
        }

        if (!dev) {
            log::setLevel(log::Level::Warn);
            auto fileName = util::math::sum<std::string>("logs/", util::time::getLocalTimeString("%Y-%m-%d-%H-%M-%S").value(), ".log");
            log::addFileAppender(fileName, false);

        } else if (dev && !debug) {
            log::setLevel(log::Level::Info);
            auto fileName = util::math::sum<std::string>("logs/", util::time::getLocalTimeString("%Y-%m-%d-%H-%M-%S").value(), ".log");
            log::addFileAppender(fileName, false);

        } else if (dev && debug) {
            log::setLevel(log::Level::Debug);
            log::addFileAppender("logs/debug.log", false);
            log::addFileAppender("logs/new-debug.log", true);
        }
    }

    inline void initThreads() {

        auto cfg = bus::config::getClientConfig();
        neko::uint64 threadCount = cfg.net.thread > 0 ? cfg.net.thread : std::thread::hardware_concurrency();
        bus::thread::setThreadCount(threadCount);

        log::setCurrentThreadName("Main Thread");
        log::info({}, "Initializing thread pool with {} threads", threadCount);

        bus::thread::setLogger([](const std::string &log) {
            log::info(log);
        });

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
                log::error({}, "Not Found Worker Thread {}", i_str);
            }
        }
    }

    inline void initSystem() {
        using namespace neko::ops::pipe;

        ClientConfig cfg = bus::config::getClientConfig();
        if (std::filesystem::is_directory(cfg.other.tempFolder)) {
            system::tempFolder(std::string(cfg.other.tempFolder) | util::unifiedPath);
        }
    }

    inline void configInfoPrint(const ClientConfig &config) {

        log::info({}, "config main : lang : {} , backgroundType : {} , background : {} , windowSize : {} , launcherMethod : {} , useSysWindowFrame: {} , headBarKeepRight : {} , deviceID : {}",
                  config.main.lang,
                  config.main.backgroundType,
                  config.main.background,
                  config.main.windowSize,
                  config.main.launcherMethod,
                  util::logic::boolTo<neko::cstr>(config.main.useSysWindowFrame),
                  util::logic::boolTo<neko::cstr>(config.main.headBarKeepRight),
                  config.main.deviceID);

        log::info({}, "config net : thread : {} , proxy : {}",
                  config.net.thread,
                  config.net.proxy);

        log::info({}, "config style : blurEffect : {} , blurRadius : {} , fontPointSize : {} , fontFamilies : {}",
                  config.style.blurEffect,
                  config.style.blurRadius,
                  config.style.fontPointSize,
                  config.style.fontFamilies);

        log::info({}, "config dev : enable : {} , debug : {} , server : {} , tls : {}",
                  util::logic::boolTo<neko::cstr>(config.dev.enable),
                  util::logic::boolTo<neko::cstr>(config.dev.debug),
                  config.dev.server,
                  util::logic::boolTo<neko::cstr>(config.dev.tls));

        std::string accToken = config.minecraft.accessToken ? config.minecraft.accessToken : "";
        std::string maskToken = accToken.empty() ? "null" : "**********" + accToken.substr(accToken.size() - 4); // Hide accessToken for security
        log::info({}, "config minecraft : folder : {} , javaPath : {} , account : {} , name : {} , uuid : {} , accessToken : {} , targetVersion : {} , maxMemory : {} , minMemory : {} , needMemory : {} , authlibName : {} , authlibPrefetched : {} , authlibSha256 : {} , tolerantMode : {} , customResolution : {} , joinServerAddress : {} , joinServerPort : {}",
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

        log::info({}, "config other : temp : {} , resVersion : {}",
                  config.other.tempFolder,
                  config.other.resourceVersion);
    }

    inline void initialize() {
        bus::config::load(app::getConfigFileName());

        initLog();
        initThreads();

        initDeviceID();

        auto cfg = bus::config::getClientConfig();
        lang::language(cfg.main.lang);
        configInfoPrint(bus::config::getClientConfig());

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
            log::info({}, "Network initialized with UserAgent: {}, Proxy: {}, Protocol: {}", config.getUserAgent(), config.getProxy(), config.getProtocol());

            log::info("Network::initialize : Starting test hosts...");

            for (auto it : schema::definitions::NetworkHostList) {
                network::Network net;
                std::string url = network::buildUrl(api::testing, it.data());

                network::RetryConfig cfg{
                    .config = network::RequestConfig{
                        .url = url,
                        .method = network::RequestType::Get,
                        .userAgent = config.getUserAgent(),
                        .proxy = config.getProxy(),
                        .requestId = "Testing - " + std::string(it)},
                    .maxRetries = 2,
                    .retryDelay = std::chrono::milliseconds{50},
                    .successCodes = {200}};

                auto result = net.executeWithRetry(cfg);

                if (result.isSuccess()) {
                    log::info({}, "Network::initialize() : Testing host available, host: {} , statusCode: {}", it, result.statusCode);
                    config.pushAvailableHost(std::string(it));
                    continue;
                }

                log::warn({}, "Network::initialize() : Testing host failed, host: {}, statusCode: {}, errorMessage: {}", it, std::to_string(result.statusCode), result.errorMessage);
            }
        });
    }
} // namespace neko::core::app::init
