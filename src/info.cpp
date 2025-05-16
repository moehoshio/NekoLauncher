#include "neko/function/info.hpp"

namespace neko
{
    // This object defined in separate translation unit because of a bug in Apple Clang.
    info::LanguageKey info::lang;
    
} // namespace neko
