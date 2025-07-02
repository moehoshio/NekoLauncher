/**
 * @file autoinit.hpp
 * @brief Initialization for NekoLauncher, including logging, thread pool, and configuration.
 */

#pragma once

#include "neko/log/nlog.hpp"

#include "neko/core/resources.hpp"

#include "neko/schema/types.hpp"
#include "neko/schema/clientconfig.hpp"

#include "neko/function/info.hpp"

#include "neko/network/network.hpp"

#include "neko/function/utilities.hpp"

#include <filesystem>

namespace neko::init {

    /**
     * @brief Initialize the logging system based on configuration.
     * @param argc Argument count from main().
     * @param argv Argument vector from main().
     * @param cfg Client configuration.
     */
    inline void initLog(int argc, char *argv[], const ClientConfig& cfg) {

        loguru::init(argc, argv);

        bool
            dev = cfg.dev.enable,
            debug = cfg.dev.debug;

        if (!std::filesystem::exists("logs")) {
            std::filesystem::create_directory("logs");
        }

        if (!dev) {
            loguru::g_stderr_verbosity = loguru::Verbosity_OFF; // Avoid output to console

            std::fstream file("logs/error.log", std::ios::out);
            (void)loguru::add_file("logs/error.log", loguru::Append, loguru::Verbosity_ERROR);
            return;
        }

        if (dev && !debug) {
            auto fileName = util::math::sum<std::string>("logs/", util::time::getLocalTimeString("%Y-%m-%d-%H-%M-%S").value(), ".log");

            std::fstream file(fileName, std::ios::out);
            if (file.is_open()) {
                file.close();
            }

            (void)loguru::add_file(fileName.c_str(), loguru::Append, loguru::Verbosity_INFO);
            return;
        }

        if (dev && debug) {
            loguru::g_stderr_verbosity = loguru::Verbosity_9; // Output to console

            neko::cstr debugFileName = "logs/debug.log";
            neko::cstr newDebugFileName = "logs/new-debug.log";

            for (auto path : std::array{debugFileName, newDebugFileName}) {
                if (!std::filesystem::exists(std::filesystem::path(path))) {
                    std::fstream file(path, std::ios::out);
                    if (file.is_open()) {
                        file.close();
                    }
                }
            }

            (void)loguru::add_file(debugFileName, loguru::Append, loguru::Verbosity_9);
            (void)loguru::add_file(newDebugFileName, loguru::Truncate, loguru::Verbosity_9);
            return;
        }
    }

    /**
     * @brief Initialize thread names for the thread pool.
     */
    inline void initThreadName() {
        neko::uint64 nums = core::getThreadPool().get_thread_nums();
        log::autoLog log{"init threadNums : " + std::to_string(nums)};

        for (neko::uint64 i = 0; i < nums; ++i) {
            neko::core::getThreadPool().enqueue(
                [i_str = std::to_string(i + 1)]{
                    loguru::set_thread_name(("thread " + i_str).c_str());
                    
                    // Wait some time to ensure the thread name is set
                    // If a thread finishes its work too quickly, it may repeatedly enter work and prevent other threads from setting their names correctly
                    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 

                    log::Info("Hello thread %s",i_str.c_str());
                });
        }
    }

    /**
     * @brief Set the number of threads in the thread pool.
     * @param nums Number of threads to set. If <= 0, use hardware_concurrency.
     */
    inline void setThreadNums(int nums) {
        log::Info(log::SrcLoc::current(), "set threadNums : %d (if nums <= 0, use hardware_concurrency)",nums);
        if (nums > 0)
            neko::core::getThreadPool().set_pool_size(static_cast<neko::uint64>(nums));
    }

    /**
     * @brief Print configuration information to the log.
     * @param config The client configuration to print.
     */
    inline void configInfoPrint(const ClientConfig & config) {
        
        log::Info(log::SrcLoc::current(), "config main : lang : %s , backgroundType : %s , background : %s , windowSize : %s , launcherMethod : %ld , useSysWindowFrame: %s , headBarKeepRight : %s",
            config.main.lang,
            config.main.backgroundType,
            config.main.background,
            config.main.windowSize,
            config.main.launcherMethod,
            util::logic::boolTo<neko::cstr>(config.main.useSysWindowFrame),
            util::logic::boolTo<neko::cstr>(config.main.headBarKeepRight)
        );

        log::Info(log::SrcLoc::current(), "config net : thread : %ld , proxy : %s",
            config.net.thread,
            config.net.proxy
        );

        log::Info(log::SrcLoc::current(), "config style : blurEffect : %ld , blurRadius : %ld , fontPointSize : %ld , fontFamilies : %s",
            config.style.blurEffect,
            config.style.blurRadius,
            config.style.fontPointSize,
            config.style.fontFamilies
        );

        log::Info(log::SrcLoc::current(), "config dev : enable : %s , debug : %s , server : %s , tls : %s",
            util::logic::boolTo<neko::cstr>(config.dev.enable),
            util::logic::boolTo<neko::cstr>(config.dev.debug),
            config.dev.server,
            util::logic::boolTo<neko::cstr>(config.dev.tls)
        );

        std::string accToken = config.minecraft.accessToken ? config.minecraft.accessToken : "";
        std::string maskToken = accToken.empty() ? "null" : "**********" + accToken.substr(accToken.size() - 4); // Hide accessToken for security
        log::Info(log::SrcLoc::current(), "config minecraft : folder : %s , javaPath : %s , account : %s , name : %s , uuid : %s , accessToken : %s , targetVersion : %s , maxMemory : %ld , minMemory : %ld , needMemory : %ld , authlibName : %s , authlibPrefetched : %s , authlibSha256 : %s , tolerantMode : %s , customResolution : %s , joinServerAddress : %s , joinServerPort : %s",
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
            config.minecraft.joinServerPort
        );

        log::Info(log::SrcLoc::current(), "config other : temp : %s , resVersion : %s",
            config.other.tempFolder,
            config.other.resourceVersion
        );
    }

    /**
     * @brief Automatic initialization: configuration, logging, thread pool, language, etc.
     * @param argc Number of arguments from main().
     * @param argv Argument array from main().
     * @return std::future<void> - Returns a std::future object indicating whether the network module initialization is complete.
     * This function initializes the network module, sets global configuration, and tests the host.
     */
    inline auto autoInit(int argc, char *argv[]) {

        // If loading the configuration file fails, all options will use default values
        (void)core::getConfigObj().LoadFile(info::app::getConfigFileName());

        neko::ClientConfig cfg(core::getConfigObj());

        initLog(argc, argv, cfg);

        setThreadNums(cfg.net.thread);
        initThreadName();

        info::lang::language(cfg.main.lang);

        configInfoPrint(cfg);

        return neko::network::NetworkBase::initialize();
    };
} // namespace neko::init
