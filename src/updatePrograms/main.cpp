#include "exec.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <chrono>
#include <thread>

#if _WIN32
#include <Windows.h>
    void launchNewProcess(const std::string &command) {
        STARTUPINFOA si = {sizeof(si)};
        PROCESS_INFORMATION pi;
        std::string inDir = std::filesystem::current_path().string();
        if (!CreateProcessA(NULL, const_cast<char *>(command.c_str()), NULL, NULL, false, NULL, NULL, inDir.c_str(), &si, &pi)) {
            std::cerr<<"Faild to Create process! cmd : "<<command<< "in dir : "<<inDir<<"\n";
        }
    }
#else
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
    void launchNewProcess(const std::string &command) {
        pid_t pid = fork();
        if (pid == 0) {
            execl("/bin/sh", "sh", "-c", command.c_str(), (char *)0);
            _exit(EXIT_FAILURE); // if execl return ,an the err
        } else if (pid < 0) {
            // fork err
        } else {
            _exit(EXIT_FAILURE); 
        }
    }
#endif

// e.g update /apps/Nekolc/ /temp/curl.so /temp/update.zip:
int main(int argc, char *argv[]) {
    if (argc < 3) {
        std::cerr << "Not enough parameters!\n e.g update /apps/Nekolc/(work dir) /temp/curl.so(copy file) /temp/update.zip(unzip)";
        return -1;
    }
    if (!std::filesystem::is_directory(argv[1])) {
        std::cerr << "This is not the correct folder path! :" << argv[1] << " \n";
        return -2;
    }
    std::string dir = argv[1];
    std::string k7zPath = dir + ((_WIN32)? "/7z" : "/7zz");
    std::vector<std::string> list;
    std::vector<std::string> compressTypes = {"zip", "7z", "tar", "gz", "xz"};
    for (size_t i = 2; i < argc; ++i) {
        std::cerr << "push : " << argv[i] << "\n";
        list.push_back(argv[i]);
    }
    //wait main programs exit
    std::cerr<<"now sleep 5 seconds\n";
    std::this_thread::sleep_for(std::chrono::seconds(6));

    for (const auto &it : list) {
        if (exec::matchExtNames(it, compressTypes)) {
            std::string cmd = k7zPath + " x " + it + " " + "-o\"" + dir + "\"";
            std::cerr << "unzip : " << cmd << "\n";
            std::system(cmd.c_str());

        } else {
            std::cerr << "copy : " << it << " to " << dir << "\n";
            std::filesystem::copy(it, dir,std::filesystem::copy_options::overwrite_existing);
        }
    }
    launchNewProcess(dir+"/NekoLc");
    
}