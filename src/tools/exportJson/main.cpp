#include "./exec.h"
#include "../../include/nlohmann/json.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>

struct FileInfos {
    std::string url, fileName, hash;
};

std::string inputDir;
std::string host;

std::vector<FileInfos> versionFiles;

void processDir(const std::filesystem::path &dirPath) {
    for (const auto &it : std::filesystem::directory_iterator(dirPath)) {
        if (it.is_regular_file()) {
            std::string fileName = exec::unifiedThePaths(std::filesystem::relative(it.path(), inputDir).string());
            versionFiles.push_back({host + fileName,
                                    fileName,
                                    exec::hashFile(fileName)});
        } else if (it.is_directory()) {
            processDir(it);
        }
    }
}

// ./exportTool /to/path/ https://www.example.com/download exportV1.0.1.json
int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Not enough parameters!\ne.g: ./exportTool /to/path (need to update files dir) https://example.com/download/ (url prefix) exportV1.0.1.json (export file name)\n";
        return -1;
    }

    if (!std::filesystem::is_directory(argv[1]) || !std::filesystem::exists(argv[1])) {
        std::cerr << "Not the correct folder path or not exists!\n";
        return -2;
    }
    host = argv[2];
    inputDir = argv[1];
    processDir(inputDir);
    nlohmann::json outJson = {
        {"title",""},
        {"msg",""},
        {"poster",""},
        {"time",""},
        {"resVersion",""},
        {"update",{}}
    };
    for (const auto& fileInfo : versionFiles) { 
        outJson["update"].push_back({
            {"url",fileInfo.url},
            {"name",fileInfo.fileName},
            {"hash",fileInfo.hash},
            {"meta",{
                {"hashAlgorithm",exec::mapAlgorithm(exec::hashs::Algorithm::sha256)},
                {"multis",false},
                {"temp",false},
                {"randName",false},
                {"absoluteUrl",true}
            }}
        });
        }
    std::string fileName = (argc < 4)? "export.json": argv[3];
    std::ofstream outFile(fileName);
    if (!outFile.is_open())
    {
        std::cout<<"faild to open file : "<<fileName<<"\n";
        return -3;
    }
    outFile<<outJson;
    outFile.close();
}