#include "info.h"

namespace neko {
    info::LanguageKey info::lang;

    std::string info::language(const std::string &lang) {
        static std::string preferredLanguage = "en";

        if (!lang.empty())
            preferredLanguage = lang;

        return preferredLanguage;
    }
} // namespace neko
