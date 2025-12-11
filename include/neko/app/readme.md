# NekoLc App Module

Core layer for lifecycle, configuration, localization, and metadata.

## Files

- `api.hpp` — API data structs/JSON helpers
- `app.hpp` / `appinit.hpp` — lifecycle + boot helpers
- `appinfo.hpp` — static/dynamic app metadata
- `clientConfig.hpp` / `configManager.hpp` — config model + thread-safe access
- `lang.hpp` — i18n helpers and translation keys
- `nekoLc.hpp` — app constants

## Quick Use

```cpp
neko::app::init::initialize();      // boot: log/device/lang/network/config
auto cfg = neko::bus::config::getClientConfig();
neko::bus::config::updateClientConfig([](neko::ClientConfig& c){ c.main.lang = "en"; });
neko::bus::config::save(neko::app::getConfigFileName());
std::string title = neko::lang::tr(neko::lang::keys::maintenance::category,
                                   neko::lang::keys::maintenance::title);
```

## Notes

- Config stored in INI (SimpleIni); language in UTF-8 JSON.
- Thread-safe config updates; shared access for translations.
- Dependencies: schema, system, network, log, bus, nlohmann::json.
