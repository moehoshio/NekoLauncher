#pragma once

constexpr const char *launcherMode = "minecraft"; // Option: `minecraft`. Used in the `launcher` function in core.hpp

using uint_64 =  unsigned long long;
using uint_32 = unsigned int;
using uint_16 = unsigned short;

using int_64 = long long;
using int_32 = int;
using int_16 = short;

using uint = uint_32;

// Import the logging module for nerr exceptions , in nerr.hpp
#define nerrImpLoggerModeDefine true

// Define a convenient macro for printing logs
#define FN __FUNCTION__

#define FI __FILE__

#define LI __LINE__

// e.g {"api.example.com","api.example.org","" ...}
#define NetWorkHostListDefine {"api.example.com","www.example.org"};

// If true, it means that the definition of NetWorkHostListDefine will be used regardless of the presence of a hostlist file. By default (false), it will attempt to read the hostlist file.
#define UseNetWorkHostListDefine false


//any string
#define NekoLcCoreVersionDefine "v0.0.1"

// If true, the version number defined by NekoLcCoreVersionDefine will be used; otherwise, the version number in the data/version file will be used.
#define UseNekoLcCoreVersionDefine false


#define NetWorkAuthlibHostDefine "skin.example.org"
