#include "network.h"
namespace neko {

    using namespace std::literals;
    networkBase::Config networkBase::Dconfig{};
    constexpr networkBase::Api networkBase::api;

    std::unordered_map<networkBase::Opt, std::string> networkBase::optMap = {
        {networkBase::Opt::downloadFile, "downloadFile"s},
        {networkBase::Opt::onlyRequest, "onlyRequest"s},
        {networkBase::Opt::postText, "postText"s},
        {networkBase::Opt::postFile, "postFile"s},
        {networkBase::Opt::getSize, "getSize"s},
        {networkBase::Opt::getContent, "getContent"s},
        {networkBase::Opt::getContentType, "getContentType"s},
        {networkBase::Opt::getHeadContent, "getHeadContent"s}};

} // namespace neko

size_t neko::networkBase::WriteCallbackString(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string *buffer = reinterpret_cast<std::string *>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t neko::networkBase::WriteCallbackFile(char *contents, size_t size, size_t nmemb, void *userp) {
    std::fstream *file = static_cast<std::fstream *>(userp);
    file->write(contents, size * nmemb);
    return size * nmemb;
}
