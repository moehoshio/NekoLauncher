/**
 * @file autoinit.hpp
 * @brief Initialization for NekoLauncher, including logging, thread pool, and configuration.
 */

#pragma once

#include "neko/log/nlog.hpp"

#include "neko/schema/types.hpp"
#include "neko/schema/clientconfig.hpp"

#include "neko/network/network.hpp"


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
            std::string fileName = exec::sum<std::string>("logs/", exec::getTimeString(), ".log");

            std::fstream file(fileName, std::ios::out);
            if (file.is_open()) {
                file.close();
            }

            (void)loguru::add_file(file_name.c_str(), loguru::Append, loguru::Verbosity_INFO);
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
        neko::uint64 nums = exec::getThreadObj().get_thread_nums();
        nlog::autoLog log{FI, LI, FN, "init threadNums : " + std::to_string(nums)};

        for (neko::uint64 i = 0; i < nums; ++i) {
            exec::getThreadObj().enqueue(
                [i_str = std::to_string(i + 1)]() {
                    loguru::set_thread_name((std::string("thread ") + i_str).c_str());
                    
                    // Wait some time to ensure the thread name is set
                    // If a thread finishes its work too quickly, it may repeatedly enter work and prevent other threads from setting their names correctly
                    std::this_thread::sleep_for(std::chrono::milliseconds(100)); 

                    nlog::Info(FI, LI, "%s : Hello thread %s", FN, i_str.c_str());
                });
        }
    }

    /**
     * @brief Set the number of threads in the thread pool.
     * @param nums Number of threads to set. If <= 0, use hardware_concurrency.
     */
    inline void setThreadNums(int nums) {
        nlog::Info(FI, LI, "%s : set threadNums : %d (if nums <= 0, use hardware_concurrency)", FN, nums);
        if (nums > 0)
            exec::getThreadObj().set_pool_size(static_cast<neko::uint64>(nums));
    }

    /**
     * @brief Print configuration information to the log.
     * @param config The client configuration to print.
     */
    inline void configInfoPrint(const ClientConfig & config) {
        nlog::Info(FI, LI, "%s : config main : lang : %s , bgType : %s , bg : %s , windowSize : %s , launcherMode : %d ,  useSysWinodwFrame: %s , barKeepRight : %s ", FN, config.main.lang, config.main.bgType, config.main.bg, config.main.windowSize, config.main.launcherMode, exec::boolTo<neko::cstr >(config.main.useSysWindowFrame), exec::boolTo<neko::cstr >(config.main.barKeepRight));
        nlog::Info(FI, LI, "%s : config net : thread : %d , proxy : %s", FN, config.net.thread, config.net.proxy);
        nlog::Info(FI, LI, "%s : config style : blurHint : %d , blurValue : %d , fontPointSize : %d , fontFamilies : %s ", FN, config.style.blurHint, config.style.blurValue, config.style.fontPointSize, config.style.fontFamilies);
        nlog::Info(FI, LI, "%s : config dev : enable : %s , debug : %s , server : %s , tls : %s ", FN, exec::boolTo<neko::cstr >(config.dev.enable), exec::boolTo<neko::cstr >(config.dev.debug), config.dev.server, exec::boolTo<neko::cstr >(config.dev.tls));
        nlog::Info(FI, LI, "%s : config minecraft : account : %s , name : %s , uuid : %s , authlibPrefetched : %s ", FN, config.minecraft.account, config.minecraft.playerName, config.minecraft.uuid, config.minecraft.authlibPrefetched);
        nlog::Info(FI, LI, "%s : config more : temp : %s , resVersion : %s", FN, config.more.tempDir, config.more.resourceVersion);
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
        exec::getConfigObj().LoadFile(info::getConfigFileName())

        neko::ClientConfig cfg(exec::getConfigObj());

        initLog(argc, argv, cfg);

        setThreadNums(cfg.net.thread);
        initThreadName();

        // Enable error logging
        nerr::Error::enableLogger = true;

        info::language(cfg.main.lang);

        configInfoPrint(cfg);

        return neko::network::NetworkBase::initialize();
    };
} // namespace neko::init
