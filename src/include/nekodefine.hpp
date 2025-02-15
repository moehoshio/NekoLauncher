#pragma once

constexpr const char *launcherMode = "minecraft"; // Option: `minecraft`. Used in the `launcher` function in core.hpp

// Import the logging module for nerr exceptions , in nerr.hpp
#define nerrImpLoggerModeDefine true

#define NetWorkAuthlibHostDefine "skin.example.org"

// If true, it means that the definition of NetWorkHostListDefine will be used regardless of the presence of a hostlist file. By default (false), it will attempt to read the include/data/hostlist file.
#define UseNetWorkHostListDefine false
// e.g {"api.example.com","api.example.org","" ...}
#define NetWorkHostListDefine {"api.example.com","www.example.org"};

// If true, the version number defined by NekoLcCoreVersionDefine will be used; otherwise, attempt the version number in the include/data/version file will be used.
#define UseNekoLcCoreVersionDefine false
//any string
#define NekoLcCoreVersionDefine "v0.0.1"

#include <string_view>

#if defined(GIT_BUILD_ID)
constexpr inline const std::string_view build_id = GIT_BUILD_ID;
#else
#include "exec.hpp"
inline const std::string_view build_id = exec::generateUUID(16);
#endif




using uint_64 =  unsigned long long;
using uint_32 = unsigned int;
using uint_16 = unsigned short;

using int_64 = long long;
using int_32 = int;
using int_16 = short;

using uint = uint_32;

// Define a convenient macro for printing logs
#define FN __FUNCTION__

#define FI __FILE__

#define LI __LINE__