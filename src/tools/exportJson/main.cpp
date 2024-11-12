#include <filesystem>
#include <iostream>
#include <string>
#include <openssl/sha.h>
#include "exec.h"
// ./exportTool /raw/path /new/path exportV1.0.1.json
int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Not enough parameters!\ne.g ./exportTool /raw/path/(old version file) /new/path/(new version file) exportV1.0.1.json(export file name)\n";
        return -1;
    }

    for (auto it : {argv[1], argv[2]}) {

        if (!std::filesystem::is_directory(it) || !std::filesystem::exists(it)) {
            std::cerr << "Not the correct folder path or not exists!\n";
            return -2;
        }
    }

    struct FileInfos {
        std::string fileName, hash;
    };
    std::vector<FileInfos> oldVersionFiles, newVersionFiles;

    for (auto it : std::filesystem::directory_iterator(argv[1])) {
        if (it.is_regular_file()) {
            oldVersionFiles.push_back({it.path().filename().string(), hashFile(it.path().string())});
        }
    }
    for (auto it : std::filesystem::directory_iterator(argv[2])) {
        if (it.is_regular_file()) {
            newVersionFiles.push_back({it.path().filename().string(), hashFile(it.path().string())});
        }
    }
}