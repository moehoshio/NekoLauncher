// client config class
#pragma once
#include "SimpleIni/SimpleIni.h"

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
            const char *temp;
            const char *resVersion;
        };
        struct Manage {
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
        Manage manage;
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
                cfg.GetValue("more", "temp", ""),
                cfg.GetValue("more","resVersion","")};
            manage = Manage{
                cfg.GetValue("manage","account",""),
                cfg.GetValue("manage","displayName",""),
                cfg.GetValue("manage","uuid",""),
                cfg.GetValue("manage","accessToken",""),
                cfg.GetValue("manage","authlibPrefetched","")};
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

        saveCfg.SetValue("more","temp",nowConfig.more.temp);
        saveCfg.SetValue("more","resVersion",nowConfig.more.resVersion);

        saveCfg.SetValue("manage","account",nowConfig.manage.account);
        saveCfg.SetValue("manage","displayName",nowConfig.manage.displayName);
        saveCfg.SetValue("manage","uuid",nowConfig.manage.uuid);
        saveCfg.SetValue("manage","accessToken",nowConfig.manage.accessToken);
        saveCfg.SetValue("manage","authlibPrefetched",nowConfig.manage.authlibPrefetched);

        saveCfg.SaveFile(fileName);
    };
    };
} // namespace neko
