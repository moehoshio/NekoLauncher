// Neko Modules
#pragma once

#include <neko/log/nlog.hpp>
#include <neko/schema/exception.hpp>

#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/app/clientConfig.hpp"

#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"

#include "neko/event/eventTypes.hpp"

#include "neko/core/update.hpp"

#include "neko/minecraft/installMinecraft.hpp"
#include "neko/minecraft/downloadSource.hpp"

namespace neko::core::install {

	/**
	 * @brief Whether resources are missing (resourceVersion not set).
	 */
	inline bool needsInstall() {
		return bus::config::getClientConfig().main.resourceVersion.empty();
	}

	/**
	 * @brief Auto-install resources when none are present.
	 * @return true if an install was triggered; false if already installed.
	 * @throws ex::Exception bubbled up from update::autoUpdate when install fails.
	 */
	inline bool autoInstall() {
		log::autoLog log;

		if (!needsInstall()) {
			log::info("Resource version present, skipping auto-install");
			return false;
		}

		auto status = lang::tr(lang::keys::update::category, lang::keys::update::startingUpdate, "Installing resources...");
		bus::event::publish(event::LoadingStatusChangedEvent{.statusMessage = status});

		log::info("Resource version missing; starting resource install via update pipeline");

		try {
			update::autoUpdate();

			// When launcher mode targets Minecraft, perform MC assets install once core resources are ready.
			if constexpr (lc::LauncherMode == neko::strview("minecraft")) {
				const auto cfg = bus::config::getClientConfig();
				const std::string targetVersion = cfg.minecraft.targetVersion.empty() ? "1.16.5" : cfg.minecraft.targetVersion;
				const std::string installPath = cfg.minecraft.minecraftFolder.empty() ? "./.minecraft" : cfg.minecraft.minecraftFolder;

				auto toSource = [](const std::string &name) {
					if (name == "BMCLAPI") return minecraft::DownloadSource::BMCLAPI;
					return minecraft::DownloadSource::Official;
				};

				log::info("LauncherMode=minecraft; starting Minecraft install. targetVersion={}, path={}", {}, targetVersion, installPath);
				minecraft::installMinecraft(installPath, targetVersion, toSource(cfg.minecraft.downloadSource));
			}

			return true;
		} catch (const ex::Exception &e) {
			std::string reason = std::string("Auto-install failed: ") + e.what();
			log::error(reason);
			bus::event::publish(event::UpdateFailedEvent{.reason = reason});
			throw;
		} catch (const std::exception &e) {
			std::string reason = std::string("Auto-install unexpected error: ") + e.what();
			log::error(reason);
			bus::event::publish(event::UpdateFailedEvent{.reason = reason});
			throw ex::Exception(reason);
		}
	}

} // namespace neko::core::install
