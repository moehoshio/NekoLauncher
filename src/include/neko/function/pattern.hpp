#pragma once
#include <filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>


/**
 * @namespace neko::util::pattern
 * @brief Utility functions for pattern matching on file paths and names.
 */
namespace neko::util::pattern {

    /**
     * @brief Check if the pattern represents a directory (ends with '/').
     * @param pat The pattern string.
     * @return True if the pattern is a directory, false otherwise.
     */
    bool isPatternDir(const std::string &pat) {
        return !pat.empty() && pat.back() == '/';
    }

    /**
     * @brief Check if the pattern contains a wildcard character '*'.
     * @param pattern The pattern string.
     * @return True if the pattern contains '*', false otherwise.
     */
    bool containsWildcard(const std::string &pattern) {
        return pattern.find('*') != std::string::npos;
    }

    /**
     * @brief Check if the pattern is an extension pattern (e.g., ".txt").
     * @param pattern The pattern string.
     * @return True if the pattern is an extension pattern, false otherwise.
     */
    bool isExtensionPattern(const std::string &pattern) {
        return !pattern.empty() && pattern[0] == '.' && pattern.find('/', 0) == std::string::npos;
    }

    /**
     * @brief Convert a wildcard pattern to a regex string.
     * @param pattern The wildcard pattern.
     * @return The equivalent regex string.
     */
    std::string wildcardToRegexString(const std::string &pattern) {
        std::string regexStr = "";

        for (char c : pattern) {
            switch (c) {
                case '*':
                    regexStr += ".*";
                    break;
                case '.':
                case '+':
                case '[':
                case ']':
                case '(':
                case ')':
                case '{':
                case '}':
                case '^':
                case '$':
                case '|':
                case '\\':
                    regexStr += '\\';
                    regexStr += c;
                    break;
                case '/':
                    regexStr += "\\/";
                    break;
                default:
                    regexStr += c;
            }
        }

        return regexStr;
    }

    /**
     * @brief Match a target string against a wildcard pattern.
     * @param target The target string (file path).
     * @param pattern The wildcard pattern.
     * @return True if the target matches the pattern, false otherwise.
     */
    bool matchWildcardPattern(const std::string &target, const std::string &pattern) {
        if (pattern == "*")
            return true;

        // Convert wildcard pattern to regex string
        std::string regexStr = wildcardToRegexString(pattern);

        // Filename or extension pattern does not contain '/'
        if (pattern.find('/') == std::string::npos) {
            std::string filename = std::filesystem::path(target).filename().string();
            std::regex fileRegex("^" + regexStr + "$");
            if (std::regex_match(filename, fileRegex)) {
                return true;
            }
        }
        // Handle directory-specific pattern (e.g. "src/*.cpp")
        else {
            size_t lastSlash = pattern.find_last_of('/');
            if (lastSlash != std::string::npos) {
                std::string dirPrefix = pattern.substr(0, lastSlash + 1); // includes "/"
                std::string filePattern = pattern.substr(lastSlash + 1);

                // Two cases:
                // 1. If pattern is absolute path (starts with '/')
                if (pattern[0] == '/') {
                    // Target must start with this prefix
                    if (target.compare(0, dirPrefix.size(), dirPrefix) != 0) {
                        return false;
                    }

                    size_t targetLastSlash = target.find_last_of('/');
                    if (targetLastSlash != std::string::npos && targetLastSlash >= dirPrefix.size() - 1) {
                        std::string targetFile = target.substr(targetLastSlash + 1);
                        std::regex fileRegex("^" + wildcardToRegexString(filePattern) + "$");
                        return std::regex_match(targetFile, fileRegex);
                    }
                }
                // 2. If pattern is relative path
                else {
                    // Check if target path contains this prefix
                    size_t pos = target.find(dirPrefix);
                    if (pos == std::string::npos) {
                        return false;
                    }

                    std::string dirSegment = target.substr(pos + dirPrefix.size());
                    size_t nextSlash = dirSegment.find('/');
                    std::string targetFile = (nextSlash == std::string::npos) ? dirSegment : dirSegment.substr(0, nextSlash);

                    std::regex fileRegex("^" + wildcardToRegexString(filePattern) + "$");
                    return std::regex_match(targetFile, fileRegex);
                }
            }
            // Other cases, use full match
            else {
                std::regex fullRegex(".*" + regexStr + "$");
                return std::regex_match(target, fullRegex);
            }
        }

        return false;
    }

    /**
     * @brief Match a target file's extension against an extension pattern.
     * @param target The target file path.
     * @param pattern The extension pattern (e.g., ".txt").
     * @return True if the file extension matches the pattern, false otherwise.
     */
    bool matchExtension(const std::string &target, const std::string &pattern) {
        std::string targetExtension = std::filesystem::path(target).extension().string();
        return targetExtension == pattern;
    }

    /**
     * @brief Check if the target path matches the absolute pattern exactly.
     * @param target The target file path.
     * @param pattern The absolute pattern.
     * @return True if the target matches the pattern, false otherwise.
     */
    bool matchAbsolutePattern(const std::string &target, const std::string &pattern) {
        return target == pattern;
    }

    /**
     * @brief Check if the target path is under the specified absolute directory pattern.
     * @param target The target file path.
     * @param dirPattern The absolute directory pattern.
     * @return True if the target is under the directory, false otherwise.
     */
    bool isTargetUnderAbsoluteDir(const std::string &target, const std::string &dirPattern) {
        std::string dir = dirPattern;
        if (!dir.empty() && dir.back() == '/') {
            dir.pop_back(); // Remove trailing slash for prefix comparison
        }

        return target.size() >= dir.size() &&
               target.substr(0, dir.size()) == dir &&
               (target.size() == dir.size() || target[dir.size()] == '/');
    }

    /**
     * @brief Check if the target path ends with the specified pattern.
     * @param target The target file path.
     * @param pattern The pattern to check.
     * @return True if the target ends with the pattern, false otherwise.
     */
    bool doesTargetEndWithPattern(const std::string &target, const std::string &pattern) {
        return target.size() >= pattern.size() &&
               target.substr(target.size() - pattern.size()) == pattern &&
               (target.size() == pattern.size() || target[target.size() - pattern.size() - 1] == '/');
    }

    /**
     * @brief Check if the target path contains the specified directory name.
     * @param target The target file path.
     * @param dirName The directory name to search for.
     * @return True if the directory name is found in the path, false otherwise.
     */
    bool containsDirectoryName(const std::string &target, const std::string &dirName) {
        std::string dirToFind = dirName;
        if (!dirToFind.empty() && dirToFind.back() == '/') {
            dirToFind.pop_back(); // Remove trailing slash
        }

        size_t pos = 0;
        while ((pos = target.find('/', pos)) != std::string::npos) {
            pos++;

            size_t end = target.find('/', pos);
            if (end == std::string::npos) {
                end = target.size();
            }

            if (end - pos == dirToFind.size() &&
                target.substr(pos, dirToFind.size()) == dirToFind) {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Match the target path against any of the provided patterns.
     * @param target The target file path.
     * @param patterns A vector of pattern strings.
     * @return True if the target matches any pattern, false otherwise.
     */
    bool matchAny(const std::string &target, const std::vector<std::string> &patterns) {
        std::string normalizedTarget = std::filesystem::path(target).lexically_normal().generic_string();
        std::string filename = std::filesystem::path(target).filename().string();

        for (const auto &pattern : patterns) {
            if (pattern.empty())
                continue;

            // Handle wildcard pattern (check first, as it may contain other pattern features)
            if (containsWildcard(pattern)) {
                if (matchWildcardPattern(normalizedTarget, pattern)) {
                    return true;
                }
                continue;
            }

            // Handle extension pattern, e.g. ".txt"
            if (isExtensionPattern(pattern)) {
                if (matchExtension(normalizedTarget, pattern)) {
                    return true;
                }
                continue;
            }

            std::string normalizedPattern = std::filesystem::path(pattern).lexically_normal().generic_string();
            bool isAbsolutePattern = !pattern.empty() && pattern[0] == '/';
            bool isDirPattern = isPatternDir(pattern);

            if (isAbsolutePattern) {
                if (isDirPattern) {
                    // Absolute directory path (/path/to/logs/)
                    if (isTargetUnderAbsoluteDir(normalizedTarget, normalizedPattern)) {
                        return true;
                    }
                } else {
                    // Absolute file path (/path/to/file.txt)
                    if (matchAbsolutePattern(normalizedTarget, normalizedPattern)) {
                        return true;
                    }
                }
            } else {
                if (isDirPattern) {
                    // Relative directory path (logs/)
                    if (containsDirectoryName(normalizedTarget, normalizedPattern)) {
                        return true;
                    }
                } else {
                    // Filename or relative file path

                    if (filename == pattern) {
                        return true;
                    }

                    if (doesTargetEndWithPattern(normalizedTarget, normalizedPattern)) {
                        return true;
                    }
                }
            }
        }

        return false;
    }
} // namespace neko::util::pattern
