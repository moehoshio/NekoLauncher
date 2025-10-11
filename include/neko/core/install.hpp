#pragma once

// Neko Module
#include <neko/schema/types.hpp>
#include <neko/schema/exception.hpp>

#include <neko/log/nlog.hpp>

// NekoLc project
#include "neko/schema/clientconfig.hpp"

#include "neko/ui/uiMsg.hpp"
#include "neko/core/resources.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

#include "neko/minecraft/installMinecraft.hpp"

#include <condition_variable>
#include <mutex>
#include <string>

namespace neko::core {

    inline void checkAndAutoInstall(ClientConfig cfg, std::function<void(const neko::ui::HintMsg &)> showHint = nullptr, std::function<void(const neko::ui::LoadMsg &)> showLoading = nullptr, std::function<void(neko::uint32)> setLoadingVal = nullptr, std::function<void(neko::cstr)> setLoadingNow = nullptr) {
        log::autoLog log;
        std::string resVer = (cfg.other.resourceVersion) ? std::string(cfg.other.resourceVersion) : std::string();
        if (resVer.empty()) {
            // Customize your installation logic, resource version needs to be stored after installation
            std::atomic<bool> stop(false);
            std::mutex mtx;
            std::condition_variable condVar;
            std::unique_lock<std::mutex> lock(mtx);
            while (!stop.load()) {
                try {
                    if (showLoading)
                        showLoading({neko::ui::LoadMsg::Type::OnlyRaw, info::lang::tr(info::lang::Keys::General::installMinecraft)});
                    minecraft::installMinecraft("./.minecraft", "1.16.5", DownloadSource::Official, showHint, showLoading, setLoadingVal);
                    cfg.other.resourceVersion = "v0.0.1";
                    cfg.save(core::getConfigObj(), info::app::getConfigFileName());
                    break; // Installation successful, exit loop
                } catch (const ex::Exception &e) {
                    auto debugMsg = std::string(e.msg);
                    try {
                        std::rethrow_if_nested(e);
                    } catch (const std::exception &nested) {
                        debugMsg += exec::sum<std::string>("\nNested: ", nested.what());
                    } catch (...) {
                        debugMsg += "\nNested: <unknown exception>";
                    }
                    log::Err(e.getFile() ,e.getLine(), "%s : installMinecraft Failed : %s",e.getFunc(), e.msg.c_str());
                    log::Debug(e.getFile() ,e.getLine(), "%s : Debug installMinecraft : DebugMsg : %s , StackTrace : %s",e.getFunc(), debugMsg.c_str(),e.getStackTraceStr().c_str());
                    if (showHint)
                        showHint({info::lang::tr(info::lang::Keys::Title::error), info::lang::tr(info::lang::Keys::Error::installMinecraft) + e.msg + debugMsg, "", {info::lang::tr(info::lang::Keys::General::ok), info::lang::tr(info::lang::Keys::General::cancel)}, [&mtx, &condVar, &stop](neko::uint32 checkId) {
                                      if (checkId == 1) {
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
