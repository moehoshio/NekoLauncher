#include <neko/function/archive.hpp>
#include <neko/schema/exception.hpp>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

namespace {

    constexpr neko::cstr kMainExeBaseName = "NekoLc";

    void printUsage() {
        std::cerr << "Usage: update <target_dir> <file1> [file2 ...]\n";
    }

    // Best-effort stop of a running NekoLc instance so files can be replaced.
    void killRunningTarget() {
        std::string command;
#if defined(_WIN32)
        command = std::string("taskkill /IM ") + kMainExeBaseName + ".exe /F >nul 2>&1";
#else
        command = std::string("pkill -f ") + kMainExeBaseName + " > /dev/null 2>&1";
#endif
        std::system(command.c_str());
    }

    // Launch the main NekoLc executable from the target directory.
    void startMainProgram(const fs::path &targetDir) {
#if defined(_WIN32)
        fs::path exePath = targetDir / (std::string(kMainExeBaseName) + ".exe");
        if (!fs::exists(exePath)) {
            exePath = targetDir / kMainExeBaseName;
        }
        std::string cmd = "\"" + exePath.string() + "\"";
        // start without waiting
        std::system(("start \"\" " + cmd).c_str());
#else
        fs::path exePath = targetDir / kMainExeBaseName;
        std::string cmd = "\"" + exePath.string() + "\"";
        std::system((cmd + " &").c_str());
#endif
    }

    bool ensureTargetDirectory(const fs::path &target) {
        std::error_code ec;
        fs::create_directories(target, ec);
        if (ec) {
            std::cerr << "Failed to create target directory '" << target.string() << "': " << ec.message() << "\n";
            return false;
        }
        return true;
    }

    bool extractArchive(const fs::path &archivePath, const fs::path &targetDir) {
        neko::archive::ExtractConfig cfg{
            .inputArchivePath = archivePath.string(),
            .destDir = targetDir.string(),
            .password = {},
            .includePaths = {},
            .excludePaths = {},
            .overwrite = true};

        try {
            neko::archive::zip::extract(cfg);
            std::cout << "Extracted archive: " << archivePath.string() << " -> " << targetDir.string() << "\n";
            return true;
        } catch (const neko::ex::Exception &e) {
            std::cerr << "Archive extract failed for '" << archivePath.string() << "': " << e.what() << "\n";
        } catch (const std::exception &e) {
            std::cerr << "Archive extract failed for '" << archivePath.string() << "': " << e.what() << "\n";
        }
        return false;
    }

    bool copyEntry(const fs::path &source, const fs::path &targetDir) {
        fs::path destination = targetDir / source.filename();

        std::error_code ec;
        auto options = fs::copy_options::overwrite_existing | fs::copy_options::recursive;
        fs::copy(source, destination, options, ec);
        if (ec) {
            std::cerr << "Copy failed for '" << source.string() << "' -> '" << destination.string() << "': " << ec.message() << "\n";
            return false;
        }

        std::cout << "Copied: " << source.string() << " -> " << destination.string() << "\n";
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    if (argc < 3) {
        printUsage();
        return 1;
    }

    fs::path targetDir = argv[1];
    if (!ensureTargetDirectory(targetDir)) {
        return 1;
    }

    // Ensure existing instance is stopped before overwriting files.
    killRunningTarget();

    int failures = 0;

    for (int i = 2; i < argc; ++i) {
        fs::path source = argv[i];

        if (!fs::exists(source)) {
            std::cerr << "Source does not exist: " << source.string() << "\n";
            ++failures;
            continue;
        }

        bool isArchive = neko::archive::zip::isZipArchiveFile(source.string());
        if (!isArchive) {
            isArchive = neko::archive::isArchiveFile(source.string());
        }

        if (isArchive) {
            if (!extractArchive(source, targetDir)) {
                ++failures;
            }
            continue;
        }

        if (!copyEntry(source, targetDir)) {
            ++failures;
        }
    }

    if (failures != 0) {
        std::cerr << "Completed with " << failures << " error(s)." << "\n";
        return 2;
    }

    std::cout << "All files processed successfully into: " << targetDir.string() << "\n";

    // Relaunch main program after update.
    startMainProgram(targetDir);

    return 0;
}
