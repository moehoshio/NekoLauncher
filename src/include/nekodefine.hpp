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
#define nerrImpLoggerMode true

// Define a convenient macro for printing logs
#define FN __FUNCTION__

#define FI __FILE__

#define LI __LINE__