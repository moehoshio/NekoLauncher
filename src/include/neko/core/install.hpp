
#include "neko/log/nlog.hpp"
#include "neko/schema/clientconfig.hpp"
#include "neko/schema/wmsg.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

#include "neko/minecraft/installMinecraft.hpp"

#include <condition_variable>
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>

namespace neko::core {

    inline void checkAndAutoInstall(ClientConfig cfg, std::function<void(const neko::ui::hintMsg &)> hintFunc = nullptr, std::function<void(const neko::ui::loadMsg &)> loadFunc = nullptr, std::function<void(unsigned int val, const char *msg)> setLoadInfo = nullptr) {
        std::string resVer = (cfg.more.resourceVersion) ? std::string(cfg.more.resourceVersion) : std::string();
        if (resVer.empty()) {
            // Customize your installation logic, resource version needs to be stored after installation
            bool stop = false;
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            while (!stop) {
                try {
                    loadFunc({neko::ui::loadMsg::OnlyRaw, info::translations(info::lang.general.installMinecraft)});
                    installMinecraft("./.minecraft", "1.16.5", DownloadSource::Official, hintFunc, loadFunc, setLoadInfo);
                    cfg.more.resourceVersion = "v0.0.1";
                    cfg.save(exec::getConfigObj(), info::getConfigFileName(), cfg);
                } catch (const nerr::Error &e) {
                    hintFunc({info::translations(info::lang.title.error), info::translations(info::lang.error.installMinecraft) + e.msg, "", 2, [&mtx, &condVar, &stop](bool check) {
                                  if (!check) {
                                      stop = true;
                                      QApplication::quit();
                                  }
                                  condVar.notify_all();
                              }});
                }
                condVar.wait(lock);
            }
        }
    }
} // namespace neko::core
