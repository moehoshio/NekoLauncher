/**
 * @file clientConfig.hpp
 * @brief Client configuration class definition for NekoLauncher
 */
#pragma once

#include <neko/schema/types.hpp>
#include <SimpleIni.h>
#include <string>

namespace neko {
    /**
     * @brief Configuration structure for the NekoLauncher client
     *
     * Stores all settings and preferences for the launcher application
     */
    struct ClientConfig {
    public:
        /**
         * @brief Main launcher configuration settings
         */
        struct Main {
            std::string lang;
            std::string backgroundType;
            std::string background;
            std::string windowSize;
            std::string launcherMethod;
            std::string resourceVersion;
            std::string deviceID;
        } main;

        /**
         * @brief Style and appearance settings
         */
        struct Style {
            std::string theme;
            std::string blurEffect;
            long blurRadius;
            long fontPointSize;
            std::string fontFamilies;
            std::string animation;  // Animation style: none, minimal, smooth, ios, bounce
        } style;

        /**
         * @brief Network configuration settings
         */
        struct Net {
            long thread;
            std::string proxy;
        } net;

        /**
         * @brief Developer options
         */
        struct Dev {
            bool enable;
            bool debug;
            bool showLogViewer;
            bool showMusicControl;            // Whether to show music control widget
            std::string server;
            bool tls;
        } dev;

        /**
         * @brief Additional configuration options
         */
        struct Other {
            std::string tempFolder;
            long logRetentionDays;
            long maxLogFiles;
            bool lastRunUnclean;
            bool immediateSave;
            long newsDismissUntil;           // Unix timestamp until which news is dismissed
            std::string newsDismissVersion;   // Version string until which news is dismissed
            bool bgmEnabled;                  // Whether background music is enabled
            float bgmVolume;                  // BGM master volume (0.0 to 1.0)
        } other;

        /**
         * @brief Minecraft settings
         */
        struct Minecraft {
            std::string minecraftFolder;
            std::string javaPath; // Path to the Java executable
            std::string downloadSource;

            std::string playerName;
            std::string account;
            std::string uuid;
            std::string accessToken;

            std::string targetVersion; // Minecraft version to launch

            long maxMemoryLimit;
            long minMemoryLimit;
            long needMemoryLimit;

            std::string authlibName; // Name of the authlib injector jar file
            std::string authlibPrefetched;
            std::string authlibSha256;

            bool tolerantMode; // Whether to use tolerant mode for launching Minecraft

            std::string customResolution;  // Custom resolution for Minecraft, if any. for example, "1920x1080"
            std::string joinServerAddress; // Address of the server to join
            std::string joinServerPort;    // Port of the server to join
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
            main.launcherMethod = cfg.GetValue("main", "launcherMethod", "launchVisible");
            main.resourceVersion = cfg.GetValue("main", "resourceVersion", "");
            main.deviceID = cfg.GetValue("main", "deviceID", "");

            style.theme = cfg.GetValue("style", "theme", "dark");
            style.blurEffect = cfg.GetValue("style", "blurEffect", "animation");
            style.blurRadius = cfg.GetLongValue("style", "blurRadius", 10);
            style.fontPointSize = cfg.GetLongValue("style", "fontPointSize", 10);
            style.fontFamilies = cfg.GetValue("style", "fontFamilies", "");
            style.animation = cfg.GetValue("style", "animation", "ios");

            net.thread = cfg.GetLongValue("net", "thread", 0);
            net.proxy = cfg.GetValue("net", "proxy", "true");

            dev.enable = cfg.GetBoolValue("dev", "enable", false);
            dev.debug = cfg.GetBoolValue("dev", "debug", false);
            dev.showLogViewer = cfg.GetBoolValue("dev", "showLogViewer", false);
            dev.showMusicControl = cfg.GetBoolValue("dev", "showMusicControl", false);
            dev.server = cfg.GetValue("dev", "server", "auto");
            dev.tls = cfg.GetBoolValue("dev", "tls", true);

            other.tempFolder = cfg.GetValue("other", "customTempDir", "");
            other.logRetentionDays = cfg.GetLongValue("other", "logRetentionDays", 14);
            other.maxLogFiles = cfg.GetLongValue("other", "maxLogFiles", 20);
            other.lastRunUnclean = cfg.GetBoolValue("other", "lastRunUnclean", false);
            other.immediateSave = cfg.GetBoolValue("other", "immediateSave", false);
            other.newsDismissUntil = cfg.GetLongValue("other", "newsDismissUntil", 0);
            other.newsDismissVersion = cfg.GetValue("other", "newsDismissVersion", "");
            other.bgmEnabled = cfg.GetBoolValue("other", "bgmEnabled", true);
            other.bgmVolume = static_cast<float>(cfg.GetDoubleValue("other", "bgmVolume", 0.7));

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
            cfg.SetValue("main", "language", main.lang.c_str());
            cfg.SetValue("main", "backgroundType", main.backgroundType.c_str());
            cfg.SetValue("main", "background", main.background.c_str());
            cfg.SetValue("main", "windowSize", main.windowSize.c_str());
            cfg.SetValue("main", "launcherMethod", main.launcherMethod.c_str());
            cfg.SetValue("main", "resourceVersion", main.resourceVersion.c_str());
            cfg.SetValue("main", "deviceID", main.deviceID.c_str());

            cfg.SetValue("style", "theme", style.theme.c_str());
            cfg.SetValue("style", "blurEffect", style.blurEffect.c_str());
            cfg.SetLongValue("style", "blurRadius", style.blurRadius);
            cfg.SetLongValue("style", "fontPointSize", style.fontPointSize);
            cfg.SetValue("style", "fontFamilies", style.fontFamilies.c_str());
            cfg.SetValue("style", "animation", style.animation.c_str());

            cfg.SetLongValue("net", "thread", net.thread);
            cfg.SetValue("net", "proxy", net.proxy.c_str());

            cfg.SetBoolValue("dev", "enable", dev.enable);
            cfg.SetBoolValue("dev", "debug", dev.debug);
            cfg.SetBoolValue("dev", "showLogViewer", dev.showLogViewer);
            cfg.SetBoolValue("dev", "showMusicControl", dev.showMusicControl);
            cfg.SetValue("dev", "server", dev.server.c_str());
            cfg.SetBoolValue("dev", "tls", dev.tls);

            cfg.SetValue("other", "customTempDir", other.tempFolder.c_str());
            cfg.SetLongValue("other", "logRetentionDays", other.logRetentionDays);
            cfg.SetLongValue("other", "maxLogFiles", other.maxLogFiles);
            cfg.SetBoolValue("other", "lastRunUnclean", other.lastRunUnclean);
            cfg.SetBoolValue("other", "immediateSave", other.immediateSave);
            cfg.SetLongValue("other", "newsDismissUntil", other.newsDismissUntil);
            cfg.SetValue("other", "newsDismissVersion", other.newsDismissVersion.c_str());
            cfg.SetBoolValue("other", "bgmEnabled", other.bgmEnabled);
            cfg.SetDoubleValue("other", "bgmVolume", static_cast<double>(other.bgmVolume));

            cfg.SetValue("minecraft", "minecraftFolder", minecraft.minecraftFolder.c_str());
            cfg.SetValue("minecraft", "javaPath", minecraft.javaPath.c_str());
            cfg.SetValue("minecraft", "downloadSource", minecraft.downloadSource.c_str());

            cfg.SetValue("minecraft", "playerName", minecraft.playerName.c_str());
            cfg.SetValue("minecraft", "account", minecraft.account.c_str());
            cfg.SetValue("minecraft", "uuid", minecraft.uuid.c_str());
            cfg.SetValue("minecraft", "accessToken", minecraft.accessToken.c_str());

            cfg.SetValue("minecraft", "targetVersion", minecraft.targetVersion.c_str());

            cfg.SetLongValue("minecraft", "maxMemoryLimit", minecraft.maxMemoryLimit);
            cfg.SetLongValue("minecraft", "minMemoryLimit", minecraft.minMemoryLimit);
            cfg.SetLongValue("minecraft", "needMemoryLimit", minecraft.needMemoryLimit);

            cfg.SetValue("minecraft", "authlibName", minecraft.authlibName.c_str());
            cfg.SetValue("minecraft", "authlibPrefetched", minecraft.authlibPrefetched.c_str());
            cfg.SetValue("minecraft", "authlibSha256", minecraft.authlibSha256.c_str());

            cfg.SetBoolValue("minecraft", "tolerantMode", minecraft.tolerantMode);

            cfg.SetValue("minecraft", "customResolution", minecraft.customResolution.c_str());
            cfg.SetValue("minecraft", "joinServerAddress", minecraft.joinServerAddress.c_str());
            cfg.SetValue("minecraft", "joinServerPort", minecraft.joinServerPort.c_str());
        }
    };
} // namespace neko