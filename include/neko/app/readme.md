# NekoLc App Module

Core application layer providing infrastructure, configuration management, localization, and initialization.

## Module Structure

```text
neko/app/
├── api.hpp              - API data structures and JSON serialization
├── app.hpp              - Application lifecycle management
├── appinfo.hpp          - Application metadata
├── appinit.hpp          - Initialization functions
├── clientConfig.hpp     - Client configuration structure
├── configManager.hpp    - Thread-safe configuration manager
├── lang.hpp             - Localization and i18n support
└── nekoLc.hpp          - Application constants
```

## Core Components

### Configuration Management

**ClientConfig Structure:**

- `Main` - Language, background, windowSize, deviceID, resourceVersion
- `Style` - Theme, fontFamily, fontSize
- `Network` - Proxy settings, timeout
- `Dev` - Debug mode, FPS display, hot reload

**Thread-safe Operations:**

```cpp
// Get config
auto config = neko::bus::config::getClientConfig();

// Atomic update
neko::bus::config::updateClientConfig([](neko::ClientConfig& cfg) {
    cfg.main.lang = "zh_tw";
});

// Save/load
neko::bus::config::save("config.ini");
neko::bus::config::load("config.ini");
```

### Localization (lang.hpp)

**Translation API:**

```cpp
// Set language
neko::lang::language("en");

// Get available languages
auto languages = neko::lang::getLanguages();

// Translate with category and key
std::string text = neko::lang::tr("maintenance", "title", "Default", json::object());

// Translate with placeholders
std::string msg = neko::lang::trWithReplaced("greeting", "welcome", {
    {"{user}", "Alice"}
});
```

**Translation Keys:**

```cpp
// Predefined constants
neko::lang::keys::maintenance::title
neko::lang::keys::error::networkError
neko::lang::keys::minecraft::missingAccessToken
```

**Language File Format (JSON):**

```json
{
  "language": "English",
  "maintenance": {
    "title": "Maintenance",
    "message": "System is under maintenance"
  },
  "error": {
    "networkError": "Network error"
  }
}
```

### Application Initialization

```cpp
neko::app::init::initLog();         // Initialize logging
neko::app::init::initDeviceID();    // Generate device ID if needed
neko::app::init::initLanguage();    // Load language files
neko::app::init::initNetwork();     // Setup network layer
neko::app::init::initialize();      // Full initialization and load config
```

### Application Info

The application info module integrates both static constants (such as app name version and build ID) and dynamic runtime information (like resource version and device Id), providing a unified interface for retrieving essential metadata about the application.

```cpp
neko::app::getAppName();       // "NekoLauncher"
neko::app::getVersion();       // "v0.0.1"
neko::app::getBuildID();       // "v0.0.1-20250710184724-hash"
```

### Constants (nekoLc.hpp)

```cpp
neko::lc::AppName                    // "NekoLauncher"
neko::lc::ClientConfigFileName       // "config.ini"
neko::lc::LanguageFolderName         // "lang"
neko::lc::api::host                  // API endpoint
neko::lc::api::authlib::authenticate // Authlib endpoint
```

## Typical Initialization Flow

```cpp
int main() {
    neko::bus::config::load("config.ini");
    neko::app::init::initLog();
    neko::app::init::initDeviceID();
    neko::app::init::initLanguage();
    neko::app::init::initNetwork();
    neko::app::run();
    
    // Application logic...
    
    neko::app::quit();
    return 0;
}
```

## Thread Safety

- **ConfigManager**: Thread-safe atomic updates with shared mutex
- **Language**: Concurrent read access with `std::shared_mutex`
- **Caching**: Translation files cached with thread-safe access

## Dependencies

```text
neko::app
├── neko::schema      (type definitions)
├── neko::system      (platform info)
├── neko::network     (network layer)
├── neko::log         (logging)
├── neko::bus         (event/config bus)
└── nlohmann::json    (JSON serialization)
```

## Testing

```bash
cmake --build ./build --target NekoLcApp_Lang_test
./build/tests/app/Debug/NekoLcApp_Lang_test.exe
```

## Notes

- Config files use INI format (via SimpleIni)
- Language files use JSON with UTF-8 encoding
- Translation fallback to English if key not found
