#pragma once
#include "SimpleIni/SimpleIni.h"

namespace neko {
    struct Config {
        struct Main {
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

        Main main;
        Style style;
        Net net;
        Dev dev;
        More more;
        Config(const CSimpleIniA &cfg) {
            main = Main{
                cfg.GetValue("main","backgroundType","image"),
                cfg.GetValue("main", "background", "img/bg.png"),
                cfg.GetValue("main", "windowSize", ""),
                cfg.GetLongValue("main","launcherMode",1),
                cfg.GetBoolValue("main", "useSystemWindowFrame", false),
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
        }
    static void save(CSimpleIniA &cfg,const char * fileName,Config config){
        cfg.SetValue("main","backgroundType",config.main.bgType);
        cfg.SetValue("main","background",config.main.bg);
        cfg.SetValue("main","windowSize",config.main.windowSize);
        cfg.SetLongValue("main","launcherMode",config.main.launcherMode);
        cfg.SetBoolValue("main","useSystemWindowFrame",config.main.useSysWindowFrame);
        cfg.SetBoolValue("main","barKeepRight",config.main.barKeepRight);

        cfg.SetLongValue("style","blurHint",config.style.blurHint);
        cfg.SetLongValue("style","blurValue",config.style.blurValue);
        cfg.SetLongValue("style","fontPointSize",config.style.fontPointSize);
        cfg.SetValue("style","fontFamilies",config.style.fontFamilies);

        cfg.SetLongValue("net","thread",config.net.thread);
        cfg.SetValue("net","proxy",config.net.proxy);

        cfg.SetBoolValue("dev","enable",config.dev.enable);
        cfg.SetBoolValue("dev","debug",config.dev.debug);
        cfg.SetValue("dev","server",config.dev.server);
        cfg.SetBoolValue("dev","tls",config.dev.tls);

        cfg.SetValue("more","temp",config.more.temp);
        cfg.SetValue("more","resVersion",config.more.resVersion);
        cfg.SaveFile(fileName);
    };
    };
} // namespace neko
