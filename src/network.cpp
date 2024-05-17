#include "network.h"
#include <QByteArray>
namespace neko {

    using namespace std::literals;

    std::unordered_map<networkBase::Opt, std::string> networkBase::optMap = {
        {networkBase::Opt::downloadFile, "downloadFile"s},
        {networkBase::Opt::onlyRequest, "onlyRequest"s},
        {networkBase::Opt::postText, "postText"s},
        {networkBase::Opt::postFile, "postFile"s},
        {networkBase::Opt::retPostText, "retPostText"s},
        {networkBase::Opt::getSize, "getSize"s},
        {networkBase::Opt::getContent, "getContent"s},
        {networkBase::Opt::getContentType, "getContentType"s},
        {networkBase::Opt::getHeadContent, "getHeadContent"s}};

    std::future<void> networkBase::init() {

        std::string proxy = exec::getConfigObj().GetValue("main", "proxy", "true");
        // "" or true or proxyAdd ,otherwise set ""
        bool proxyUnexpected = exec::allTrue((proxy != ""), (proxy != "true"), !exec::isProxyAddress(proxy));
        if (proxyUnexpected)
            proxy = ""s;
        
        nlog::Info(FI, LI, "%s : proxy : %s", FN, proxy.c_str());
        bool
            dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
            tls = exec::getConfigObj().GetBoolValue("dev", "tls", true);

        networkBase::Dconfig = {
            "NekoL /"s + info::getVersion(),
            proxy | exec::move,
            "", // host
            (dev == true && tls == false) ? "http://"s : "https://"s};

        nlog::Info(FI, LI, "%s :  dev: %s , tls : %s , protocol : %s", FN, exec::boolTo<const char *>(dev), exec::boolTo<const char *>(tls), Dconfig.protocol.c_str());

        return exec::getThreadObj().enqueue( []() {
            network net;
            for (auto it : Api::hostList) {

                std::string url = buildUrl<std::string>(Api::testing, it);
                RetHttpCode retCode;

                decltype(net)::Args args{
                    url.c_str(),
                    nullptr,
                    &retCode};

                decltype(net)::autoRetryArgs Aargs{args, 200, 2, 50};

                bool res = net.autoRetry(Opt::onlyRequest, Aargs);

                if (res) {
                    nlog::Info(FI, LI, "%s : get is true , host : %s , retCode : %d", FN, it, retCode);
                    Dconfig.host = it;
                    return;
                }

            };
            nlog::Err(FI, LI, "%s : Test: No available hosts!", FN);
            Dconfig.host = Api::hostList[0];
        });
    }

} // namespace neko

size_t neko::networkBase::Write_Callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string *buffer = reinterpret_cast<std::string *>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t neko::networkBase::Header_Callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
    std::string *buffer = reinterpret_cast<std::string *>(userdata);
    buffer->append(ptr, size * nmemb);
    return size * nmemb;
}

size_t neko::networkBase::Write_CallbackFile(char *contents, size_t size, size_t nmemb, void *userp) {
    std::ofstream *file = static_cast<std::ofstream *>(userp);
    file->write(contents, size * nmemb);
    return size * nmemb;
}

size_t neko::networkBase::Write_CallbackQBA(char *contents, size_t size, size_t nmemb, void *userp) {
    size_t totalSize = size * nmemb;
    QByteArray *buffer = static_cast<QByteArray *>(userp);
    buffer->append(contents, totalSize);
    return totalSize;
}