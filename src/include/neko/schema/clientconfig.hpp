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
         * @brief Minecraft settings
         */
        struct Minecraft {
            neko::cstr minecraftFolder;
            neko::cstr javaPath; // Path to the Java executable

            neko::cstr playerName;
            neko::cstr account;
            neko::cstr uuid;
            neko::cstr accessToken;

            neko::cstr targetVersion; // Minecraft version to launch

            long maxMemoryLimit;
            long minMemoryLimit;
            long needMemoryLimit;

            neko::cstr authlibName; // Name of the authlib injector jar file
            neko::cstr authlibPrefetched;
            neko::cstr authlibSha256;

            bool tolerantMode; // Whether to use tolerant mode for launching Minecraft

            neko::cstr customResolution;  // Custom resolution for Minecraft, if any. for example, "1920x1080"
            neko::cstr joinServerAddress; // Address of the server to join
            neko::cstr joinServerPort;    // Port of the server to join
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
            main.lang = cfg.GetValue("main", "language", "en");
            main.bgType = cfg.GetValue("main", "backgroundType", "image");
            main.bg = cfg.GetValue("main", "background", "img/bg.png");
            main.windowSize = cfg.GetValue("main", "windowSize", "");
            main.launcherMode = cfg.GetLongValue("main", "launcherMode", 1);
            main.useSysWindowFrame = cfg.GetBoolValue("main", "useSystemWindowFrame", true);
            main.barKeepRight = cfg.GetBoolValue("main", "barKeepRight", true);

            style.blurHint = cfg.GetLongValue("style", "blurHint", 1);
            style.blurValue = cfg.GetLongValue("style", "blurValue", 10);
            style.fontPointSize = cfg.GetLongValue("style", "fontPointSize", 10);
            style.fontFamilies = cfg.GetValue("style", "fontFamilies", "");

            net.thread = cfg.GetLongValue("net", "thread", 0);
            net.proxy = cfg.GetValue("net", "proxy", "true");

            dev.enable = cfg.GetBoolValue("dev", "enable", false);
            dev.debug = cfg.GetBoolValue("dev", "debug", false);
            dev.server = cfg.GetValue("dev", "server", "auto");
            dev.tls = cfg.GetBoolValue("dev", "tls", true);

            more.tempDir = cfg.GetValue("more", "customTempDir", "");
            more.resourceVersion = cfg.GetValue("more", "resourceVersion", "");

            minecraft.minecraftFolder = cfg.GetValue("minecraft", "minecraftFolder", "./.minecraft");
            minecraft.javaPath = cfg.GetValue("minecraft", "javaPath", "");

            minecraft.playerName = cfg.GetValue("minecraft", "playerName", "");
            minecraft.account = cfg.GetValue("minecraft", "account", "");
            minecraft.uuid = cfg.GetValue("minecraft", "uuid", "");
            minecraft.accessToken = cfg.GetValue("minecraft", "accessToken", "");

            minecraft.targetVersion = cfg.GetValue("minecraft", "targetVersion", "");

            minecraft.maxMemoryLimit = cfg.GetLongValue("minecraft", "maxMemoryLimit", 2048);
            minecraft.minMemoryLimit = cfg.GetLongValue("minecraft", "minMemoryLimit", 1024);
            minecraft.needMemoryLimit = cfg.GetLongValue("minecraft", "needMemoryLimit", 1024);

            minecraft.authlibName = cfg.GetValue("minecraft", "authlibName", "authlib-injector.jar");
            minecraft.authlibPrefetched = cfg.GetValue("minecraft", "authlibPrefetched", "");
            minecraft.authlibSha256 = cfg.GetValue("minecraft", "authlibSha256", "");

            minecraft.tolerantMode = cfg.GetBoolValue("minecraft", "tolerantMode", false);

            minecraft.customResolution = cfg.GetValue("minecraft", "customResolution", "");
            minecraft.joinServerAddress = cfg.GetValue("minecraft", "joinServerAddress", "");
            minecraft.joinServerPort = cfg.GetValue("minecraft", "joinServerPort", "25565");
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

            saveCfg.SetValue("minecraft", "minecraftFolder", minecraft.minecraftFolder);
            saveCfg.SetValue("minecraft", "javaPath", minecraft.javaPath);

            saveCfg.SetValue("minecraft", "playerName", minecraft.playerName);
            saveCfg.SetValue("minecraft", "account", minecraft.account);
            saveCfg.SetValue("minecraft", "uuid", minecraft.uuid);
            saveCfg.SetValue("minecraft", "accessToken", minecraft.accessToken);

            saveCfg.SetValue("minecraft", "targetVersion", minecraft.targetVersion);

            saveCfg.SetLongValue("minecraft", "maxMemoryLimit", minecraft.maxMemoryLimit);
            saveCfg.SetLongValue("minecraft", "minMemoryLimit", minecraft.minMemoryLimit);
            saveCfg.SetLongValue("minecraft", "needMemoryLimit", minecraft.needMemoryLimit);

            saveCfg.SetValue("minecraft", "authlibName", minecraft.authlibName);
            saveCfg.SetValue("minecraft", "authlibPrefetched", minecraft.authlibPrefetched);
            saveCfg.SetValue("minecraft", "authlibSha256", minecraft.authlibSha256);

            saveCfg.SetBoolValue("minecraft", "tolerantMode", minecraft.tolerantMode);

            saveCfg.SetValue("minecraft", "customResolution", minecraft.customResolution);
            saveCfg.SetValue("minecraft", "joinServerAddress", minecraft.joinServerAddress);
            saveCfg.SetValue("minecraft", "joinServerPort", minecraft.joinServerPort);

            // Save the configuration to the specified file
            saveCfg.SaveFile(fileName);
        }
    };
} // namespace neko
