// client config class
#pragma once

#include "library/SimpleIni/SimpleIni.h"

namespace neko {
    struct ClientConfig {
        struct Main {
            const char *lang;
            const char *bgType;
            const char *bg;
            const char *windowSize;
            long launcherMode;
            bool useSysWindowFrame;
            bool barKeepRight;
        };
        struct Style {
            long blurHint;
            long blurValue;
            long fontPointSize;
            const char *fontFamilies;
        };

        struct Net {
            long thread;
            const char *proxy;
        };
        struct Dev {
            bool enable;
            bool debug;
            const char *server;
            bool tls;
        };
        struct More {
            const char *tempDir;
            const char *resourceVersion;
        };
        struct Minecraft {
            const char *account;
            const char *displayName;
            const char *uuid;
            const char *accessToken;
            const char *authlibPrefetched;
        };

        Main main;
        Style style;
        Net net;
        Dev dev;
        More more;
        Minecraft minecraft;
        ClientConfig(const CSimpleIniA &cfg) {
            main = Main{
                cfg.GetValue("main","language","en"),
                cfg.GetValue("main","backgroundType","image"),
                cfg.GetValue("main", "background", "img/bg.png"),
                cfg.GetValue("main", "windowSize", ""),
                cfg.GetLongValue("main","launcherMode",1),
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
                cfg.GetValue("more","resourceVersion","")};
            minecraft = Minecraft{
                cfg.GetValue("minecraft","account",""),
                cfg.GetValue("minecraft","displayName",""),
                cfg.GetValue("minecraft","uuid",""),
                cfg.GetValue("minecraft","accessToken",""),
                cfg.GetValue("minecraft","authlibPrefetched","")};
        }
    static void save(CSimpleIniA &saveCfg,const char * fileName,ClientConfig nowConfig){
        saveCfg.SetValue("main","language",nowConfig.main.lang);
        saveCfg.SetValue("main","backgroundType",nowConfig.main.bgType);
        saveCfg.SetValue("main","background",nowConfig.main.bg);
        saveCfg.SetValue("main","windowSize",nowConfig.main.windowSize);
        saveCfg.SetLongValue("main","launcherMode",nowConfig.main.launcherMode);
        saveCfg.SetBoolValue("main","useSystemWindowFrame",nowConfig.main.useSysWindowFrame);
        saveCfg.SetBoolValue("main","barKeepRight",nowConfig.main.barKeepRight);

        saveCfg.SetLongValue("style","blurHint",nowConfig.style.blurHint);
        saveCfg.SetLongValue("style","blurValue",nowConfig.style.blurValue);
        saveCfg.SetLongValue("style","fontPointSize",nowConfig.style.fontPointSize);
        saveCfg.SetValue("style","fontFamilies",nowConfig.style.fontFamilies);

        saveCfg.SetLongValue("net","thread",nowConfig.net.thread);
        saveCfg.SetValue("net","proxy",nowConfig.net.proxy);

        saveCfg.SetBoolValue("dev","enable",nowConfig.dev.enable);
        saveCfg.SetBoolValue("dev","debug",nowConfig.dev.debug);
        saveCfg.SetValue("dev","server",nowConfig.dev.server);
        saveCfg.SetBoolValue("dev","tls",nowConfig.dev.tls);

        saveCfg.SetValue("more","customTempDir",nowConfig.more.tempDir);
        saveCfg.SetValue("more","resourceVersion",nowConfig.more.resourceVersion);

        saveCfg.SetValue("minecraft","account",nowConfig.minecraft.account);
        saveCfg.SetValue("minecraft","displayName",nowConfig.minecraft.displayName);
        saveCfg.SetValue("minecraft","uuid",nowConfig.minecraft.uuid);
        saveCfg.SetValue("minecraft","accessToken",nowConfig.minecraft.accessToken);
        saveCfg.SetValue("minecraft","authlibPrefetched",nowConfig.minecraft.authlibPrefetched);

        saveCfg.SaveFile(fileName);
    };
    };
} // namespace neko
