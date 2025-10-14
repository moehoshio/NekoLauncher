/**
 * @file clientConfig.hpp
 * @brief Client configuration class definition for NekoLauncher
 */
#pragma once

#include <neko/schema/types.hpp>
#include <SimpleIni.h>

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
            neko::cstr backgroundType;
            neko::cstr background;
            neko::cstr windowSize;
            long launcherMethod;
            bool useSysWindowFrame;
            bool headBarKeepRight;
            neko::cstr deviceID;
        } main;

        /**
         * @brief Style and appearance settings
         */
        struct Style {
            neko::cstr blurEffect;
            long blurRadius;
            long fontPointSize;
            neko::cstr fontFamilies;
        } style;

        /**
         * @brief Network configuration settings
         */
        struct Net {
            long thread;
            neko::cstr proxy;
        } net;

        /**
         * @brief Developer options
         */
        struct Dev {
            bool enable;
            bool debug;
            neko::cstr server;
            bool tls;
        } dev;

        /**
         * @brief Additional configuration options
         */
        struct Other {
            neko::cstr tempFolder;
            neko::cstr resourceVersion;
        } other;

        /**
         * @brief Minecraft settings
         */
        struct Minecraft {
            neko::cstr minecraftFolder;
            neko::cstr javaPath; // Path to the Java executable
            neko::cstr downloadSource;

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
        } minecraft;

        ClientConfig() = default;
        ~ClientConfig() = default;

        /**
         * @brief Constructor to initialize from a SimpleIni configuration
         *
         * @param cfg SimpleIni configuration object to load settings from
         */
        ClientConfig(const CSimpleIniA &cfg) noexcept {
            main.lang = cfg.GetValue("main", "language", "en");
            main.backgroundType = cfg.GetValue("main", "backgroundType", "image");
            main.background = cfg.GetValue("main", "background", "img/bg.png");
            main.windowSize = cfg.GetValue("main", "windowSize", "");
            main.launcherMethod = cfg.GetLongValue("main", "launcherMethod", 1);
            main.useSysWindowFrame = cfg.GetBoolValue("main", "useSystemWindowFrame", true);
            main.headBarKeepRight = cfg.GetBoolValue("main", "headBarKeepRight", true);
            main.deviceID = cfg.GetValue("main", "deviceID", "");

            style.blurEffect = cfg.GetValue("style", "blurEffect", "Animation");
            style.blurRadius = cfg.GetLongValue("style", "blurRadius", 10);
            style.fontPointSize = cfg.GetLongValue("style", "fontPointSize", 10);
            style.fontFamilies = cfg.GetValue("style", "fontFamilies", "");

            net.thread = cfg.GetLongValue("net", "thread", 0);
            net.proxy = cfg.GetValue("net", "proxy", "true");

            dev.enable = cfg.GetBoolValue("dev", "enable", false);
            dev.debug = cfg.GetBoolValue("dev", "debug", false);
            dev.server = cfg.GetValue("dev", "server", "auto");
            dev.tls = cfg.GetBoolValue("dev", "tls", true);

            other.tempFolder = cfg.GetValue("other", "customTempDir", "");
            other.resourceVersion = cfg.GetValue("other", "resourceVersion", "");

            minecraft.minecraftFolder = cfg.GetValue("minecraft", "minecraftFolder", "./.minecraft");
            minecraft.javaPath = cfg.GetValue("minecraft", "javaPath", "");
            minecraft.downloadSource = cfg.GetValue("minecraft", "downloadSource", "Official");

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
         * @brief sets the configuration to a SimpleIni configuration object
         *
         * @param cfg SimpleIni configuration object to save settings to
         */
        void setToConfig(CSimpleIniA &cfg) const noexcept {
            cfg.SetValue("main", "language", main.lang);
            cfg.SetValue("main", "backgroundType", main.backgroundType);
            cfg.SetValue("main", "background", main.background);
            cfg.SetValue("main", "windowSize", main.windowSize);
            cfg.SetLongValue("main", "launcherMethod", main.launcherMethod);
            cfg.SetBoolValue("main", "useSystemWindowFrame", main.useSysWindowFrame);
            cfg.SetBoolValue("main", "headBarKeepRight", main.headBarKeepRight);
            cfg.SetValue("main", "deviceID", main.deviceID);

            cfg.SetValue("style", "blurEffect", style.blurEffect);
            cfg.SetLongValue("style", "blurRadius", style.blurRadius);
            cfg.SetLongValue("style", "fontPointSize", style.fontPointSize);
            cfg.SetValue("style", "fontFamilies", style.fontFamilies);

            cfg.SetLongValue("net", "thread", net.thread);
            cfg.SetValue("net", "proxy", net.proxy);

            cfg.SetBoolValue("dev", "enable", dev.enable);
            cfg.SetBoolValue("dev", "debug", dev.debug);
            cfg.SetValue("dev", "server", dev.server);
            cfg.SetBoolValue("dev", "tls", dev.tls);

            cfg.SetValue("other", "customTempDir", other.tempFolder);
            cfg.SetValue("other", "resourceVersion", other.resourceVersion);

            cfg.SetValue("minecraft", "minecraftFolder", minecraft.minecraftFolder);
            cfg.SetValue("minecraft", "javaPath", minecraft.javaPath);
            cfg.SetValue("minecraft", "downloadSource", minecraft.downloadSource);

            cfg.SetValue("minecraft", "playerName", minecraft.playerName);
            cfg.SetValue("minecraft", "account", minecraft.account);
            cfg.SetValue("minecraft", "uuid", minecraft.uuid);
            cfg.SetValue("minecraft", "accessToken", minecraft.accessToken);

            cfg.SetValue("minecraft", "targetVersion", minecraft.targetVersion);

            cfg.SetLongValue("minecraft", "maxMemoryLimit", minecraft.maxMemoryLimit);
            cfg.SetLongValue("minecraft", "minMemoryLimit", minecraft.minMemoryLimit);
            cfg.SetLongValue("minecraft", "needMemoryLimit", minecraft.needMemoryLimit);

            cfg.SetValue("minecraft", "authlibName", minecraft.authlibName);
            cfg.SetValue("minecraft", "authlibPrefetched", minecraft.authlibPrefetched);
            cfg.SetValue("minecraft", "authlibSha256", minecraft.authlibSha256);

            cfg.SetBoolValue("minecraft", "tolerantMode", minecraft.tolerantMode);

            cfg.SetValue("minecraft", "customResolution", minecraft.customResolution);
            cfg.SetValue("minecraft", "joinServerAddress", minecraft.joinServerAddress);
            cfg.SetValue("minecraft", "joinServerPort", minecraft.joinServerPort);
        }
    };
} // namespace neko