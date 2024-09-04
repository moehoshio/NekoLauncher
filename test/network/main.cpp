#include "./log.h"
#include "./network.h"
#include <filesystem>
namespace netTest {
    constexpr const char *google = "https://google.com";
    constexpr const char *text = "https://example.com/test.txt";
    constexpr const char *image = "https://example.com/image.png";
    constexpr const char *post = "https://example.com/api/checkUpdates";
    constexpr const char *file = "https://example.com/file.zip";
}; // namespace netTest

int main(int argc, char *argv[]) {
    using namespace neko;
    using namespace netTest;
    using Opt = networkBase::Opt;
    loguru::init(argc, argv);
    (void)loguru::add_file("logs.log", loguru::Truncate, loguru::Verbosity_6);
    std::filesystem::create_directory("./temp/");

    []() {
        nlog::autoLog log{FI, LI, "onlyReq"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            google};
        args.code = &code;
        args.id = "1";
        net.Do(Opt::onlyRequest, args);
    }();

    [] {
        nlog::autoLog log{FI, LI, "range"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            text,
            "./rangeText.txt",
            &code,
            false,
            "0-10",
        };
        args.id = "2";
        args.writeCallback = networkBase::WriteCallbackFile;
        net.Do(Opt::downloadFile, args);
    }();

    [] {
        nlog::autoLog log{FI, LI, "downloadFile"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            image, "./DownloadFileImage.png", &code};
        args.id = "3";
        args.writeCallback = networkBase::WriteCallbackFile;
        net.Do(Opt::downloadFile, args);
    }();

    [] {
        nlog::autoLog log{FI, LI, "postText"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            post};
        args.code = &code;
        args.data = "{\"core\":\"v0.0.0.1\",\"res\":\"v1.0.0.1\"}";
        args.id = "4";
        auto postRet = net.get(Opt::postText, args);
        nlog::Info(FI, LI, "postText ret : %s", postRet.c_str());
    }();
    [] {
        nlog::autoLog log{FI, LI, "getSize"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            file};
        args.code = &code;
        args.id = "5";
        auto size = net.getSize(args);
        nlog::Info(FI, LI, "getSize size: %zu", size);
    }();

    [] {
        nlog::autoLog log{FI, LI, "getType"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            image};
        args.code = &code;
        args.id = "6";
        auto rtype = net.getCase(Opt::getContentType, args);
        nlog::Info(FI, LI, "getType type: %s", rtype.c_str());
    }();

    [] {
        nlog::autoLog log{FI, LI, "getCont"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            image};
        args.code = &code;
        args.id = "7";
        auto imgData = net.get(Opt::getContent, args);
        std::ofstream imgOf("./getContImg.png", std::ios::out | std::ios::binary);
        imgOf << imgData;
        imgOf.close();
    }();

    [] {
        nlog::autoLog log{FI, LI, "getHead"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            file};
        args.code = &code;
        args.id = "8";
        auto head = net.get(Opt::getHeadContent, args);
        nlog::Info(FI, LI, "getHead head: %s", head.c_str());
    }();

    [] {
        nlog::autoLog log{FI, LI, "getAndStorage"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            image, "./getAndImgS1.png", &code};
        args.id = "9";
        auto getAndRet = net.getContentAndStorage(args);
        std::ofstream imgOf2("./getAndImgG2.png", std::ios::in | std::ios::binary);
        imgOf2 << getAndRet;
        imgOf2.close();
    }();
    []{
        nlog::autoLog log{FI, LI, "Multi"};
        network net;
        int code = 0;
        decltype(net)::Args args{
            file, "file.zip", &code};
        args.writeCallback = networkBase::WriteCallbackFile;
        args.id = "10";
        decltype(net)::MultiArgs margs{args};
        // margs.approach = margs.Quantity;
        bool isok = net.Multi(Opt::downloadFile, margs);
        nlog::Info(FI,LI,"Multi isok : %s",exec::boolTo<const char *>(isok));
    }();
    
}