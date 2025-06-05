/**
 * @file clientconfig.hpp
 * @brief Client configuration class definition for NekoLauncher
 */
#pragma once

#include "neko/schema/types.hpp"

#include "library/SimpleIni/SimpleIni.h"

namespace neko {
    /**
     * @brief Configuration structure for the NekoLauncher client
     * 
     * Stores all settings and preferences for the launcher application
     */
    struct ClientConfig {
        /**
         * @brief Main launcher configuration settings
         */
        struct Main {
            neko::cstr lang;
            neko::cstr bgType;
            neko::cstr bg;
            neko::cstr windowSize;
            long launcherMode;
            bool useSysWindowFrame;
            bool barKeepRight;
        };

        /**
         * @brief Style and appearance settings
         */
        struct Style {
            long blurHint;
            long blurValue;
            long fontPointSize;
            neko::cstr fontFamilies;
        };

        /**
         * @brief Network configuration settings
         */
        struct Net {
            long thread;
            neko::cstr proxy;
        };

        /**
         * @brief Developer options
         */
        struct Dev {
            bool enable;
            bool debug;
            neko::cstr server;
            bool tls;
        };

        /**
         * @brief Additional configuration options
         */
        struct More {
            neko::cstr tempDir;
            neko::cstr resourceVersion;
        };

        /**
         * @brief Minecraft account settings
         */
        struct Minecraft {
            neko::cstr account;
            neko::cstr displayName;
            neko::cstr uuid;
            neko::cstr accessToken;
            neko::cstr authlibPrefetched;
        };

        Main main;
        Style style;
        Net net;
        Dev dev;
        More more;
        Minecraft minecraft;

        ClientConfig() = default;
        ~ClientConfig() = default;

        /**
         * @brief Constructor to initialize from a SimpleIni configuration
         * 
         * @param cfg SimpleIni configuration object to load settings from
         */
        ClientConfig(const CSimpleIniA &cfg) {
            main = Main{
                cfg.GetValue("main", "language", "en"),
                cfg.GetValue("main", "backgroundType", "image"),
                cfg.GetValue("main", "background", "img/bg.png"),
                cfg.GetValue("main", "windowSize", ""),
                cfg.GetLongValue("main", "launcherMode", 1),
                cfg.GetBoolValue("main", "useSystemWindowFrame", true),
                cfg.GetBoolValue("main", "barKeepRight", true)};
            style = Style{
                cfg.GetLongValue("style", "blurHint", 1),
                cfg.GetLongValue("style", "blurValue", 10),
                cfg.GetLongValue("style", "fontPointSize", 10),
                cfg.GetValue("style", "fontFamilies", "")};
            net = Net{
                cfg.GetLongValue("net", "thread", 0),
                cfg.GetValue("net", "proxy", "true")};
            dev = Dev{
                cfg.GetBoolValue("dev", "enable", false),
                cfg.GetBoolValue("dev", "debug", false),
                cfg.GetValue("dev", "server", "auto"),
                cfg.GetBoolValue("dev", "tls", true)};
            more = More{
                cfg.GetValue("more", "customTempDir", ""),
                cfg.GetValue("more", "resourceVersion", "")};
            minecraft = Minecraft{
                cfg.GetValue("minecraft", "account", ""),
                cfg.GetValue("minecraft", "displayName", ""),
                cfg.GetValue("minecraft", "uuid", ""),
                cfg.GetValue("minecraft", "accessToken", ""),
                cfg.GetValue("minecraft", "authlibPrefetched", "")};
        }

        /**
         * @brief Save the current configuration to a file
         * 
         * @param saveCfg SimpleIni configuration object to save settings to
         * @param fileName Path to the file where configuration will be saved
         */
        void save(CSimpleIniA &saveCfg, neko::cstr fileName) const {
            saveCfg.SetValue("main", "language", main.lang);
            saveCfg.SetValue("main", "backgroundType", main.bgType);
            saveCfg.SetValue("main", "background", main.bg);
            saveCfg.SetValue("main", "windowSize", main.windowSize);
            saveCfg.SetLongValue("main", "launcherMode", main.launcherMode);
            saveCfg.SetBoolValue("main", "useSystemWindowFrame", main.useSysWindowFrame);
            saveCfg.SetBoolValue("main", "barKeepRight", main.barKeepRight);

            saveCfg.SetLongValue("style", "blurHint", style.blurHint);
            saveCfg.SetLongValue("style", "blurValue", style.blurValue);
            saveCfg.SetLongValue("style", "fontPointSize", style.fontPointSize);
            saveCfg.SetValue("style", "fontFamilies", style.fontFamilies);

            saveCfg.SetLongValue("net", "thread", net.thread);
            saveCfg.SetValue("net", "proxy", net.proxy);

            saveCfg.SetBoolValue("dev", "enable", dev.enable);
            saveCfg.SetBoolValue("dev", "debug", dev.debug);
            saveCfg.SetValue("dev", "server", dev.server);
            saveCfg.SetBoolValue("dev", "tls", dev.tls);

            saveCfg.SetValue("more", "customTempDir", more.tempDir);
            saveCfg.SetValue("more", "resourceVersion", more.resourceVersion);

            saveCfg.SetValue("minecraft", "account", minecraft.account);
            saveCfg.SetValue("minecraft", "displayName", minecraft.displayName);
            saveCfg.SetValue("minecraft", "uuid", minecraft.uuid);
            saveCfg.SetValue("minecraft", "accessToken", minecraft.accessToken);
            saveCfg.SetValue("minecraft", "authlibPrefetched", minecraft.authlibPrefetched);

            saveCfg.SaveFile(fileName);
        }
    };
} // namespace neko
