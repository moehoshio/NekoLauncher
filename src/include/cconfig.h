#pragma once
#include "SimpleIni/SimpleIni.h"

namespace neko {
    struct Config {
        struct Main {
            const char *bg;
            const char *windowSize;
            bool useSysWindowFrame;
            bool barKeepRight;
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
        };

        Main main;
        Net net;
        Dev dev;
        More more;
        Config(const CSimpleIniA &cfg) {
            main = Main{
                cfg.GetValue("main", "background", "img/bg.png"),
                cfg.GetValue("main", "windowSize", ""),
                cfg.GetBoolValue("main", "useSystemWindowFrame", false),
                cfg.GetBoolValue("main", "barKeepRight", true)};
            net = Net{
                cfg.GetLongValue("net", "thread", 0),
                cfg.GetValue("net", "proxy", "true")};
            dev = Dev{
                cfg.GetBoolValue("dev", "enable", false),
                cfg.GetBoolValue("dev", "debug", false),
                cfg.GetValue("dev", "server", "auto"),
                cfg.GetBoolValue("dev", "tls", true)};
            more = More{
                cfg.GetValue("more", "temp", "")};
        }
        
    };
} // namespace neko
