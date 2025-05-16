#pragma once

// core version , any string
#define NekoLcCoreVersionDefine "v0.0.1"

// server list ,  e.g "api.example.com","api.example.org","" ...
#define NetWorkHostListDefine "api.example.com","www.example.org"

// Option: `minecraft`. Used in the `launcher` function in core.hpp
constexpr const char *launcherMode = "minecraft"; 




// authlib host , e.g "auth.example.com"
#define NetWorkAuthlibHostDefine "skin.example.org"

// Import the logging module for nerr exceptions , in nerr.hpp
#define nerrImpLoggerModeDefine true




// unique identifier for the build
#include <string_view>

#if defined(GIT_BUILD_ID)
constexpr inline const std::string_view build_id = GIT_BUILD_ID;
#else
#include "exec.hpp"
inline const std::string_view build_id = exec::generateUUID(10);
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