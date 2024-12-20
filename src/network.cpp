#include "network.h"
#include <QtCore/QByteArray>
namespace neko {

    using namespace std::literals;

    std::unordered_map<networkBase::Opt, std::string_view> networkBase::optMap = {
        {networkBase::Opt::downloadFile, "downloadFile"s},
        {networkBase::Opt::onlyRequest, "onlyRequest"s},
        {networkBase::Opt::postText, "postText"s},
        {networkBase::Opt::postFile, "postFile"s},
        {networkBase::Opt::getSize, "getSize"s},
        {networkBase::Opt::getContent, "getContent"s},
        {networkBase::Opt::getContentType, "getContentType"s},
        {networkBase::Opt::getHeadContent, "getHeadContent"s}};

    std::future<void> networkBase::init() {

        std::string proxy = exec::getConfigObj().GetValue("net", "proxy", "true");
        // "" or true or proxyAdd ,otherwise set ""
        bool proxyUnexpected = exec::allTrue((proxy != ""), (proxy != "true"), !exec::isProxyAddress(proxy));
        if (proxyUnexpected)
            proxy = ""s;

        bool
            dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
            tls = exec::getConfigObj().GetBoolValue("dev", "tls", true);

        networkBase::Dconfig = {
            "NekoLc /"s + info::getVersion(),
            proxy | exec::move,
            Api::hostList[0],
            (dev == true && tls == false) ? "http://"s : "https://"s};

        nlog::Info(FI, LI, "%s :  proxy : %s , dev: %s , tls : %s , protocol : %s , dUseragent : %s ", FN, Dconfig.proxy.c_str(),exec::boolTo<const char *>(dev), exec::boolTo<const char *>(tls), Dconfig.protocol.c_str() ,Dconfig.userAgent.c_str());

        return exec::getThreadObj().enqueue( []{
            network net;
            for (auto it : Api::hostList) {

                std::string url = buildUrl(Api::testing, it);
                int retCode;

                decltype(net)::Args args{
                    url.c_str(),
                    nullptr,
                    &retCode};
                    std::string id = "testing-"s + it;
                    args.id = id.c_str();

                decltype(net)::autoRetryArgs Aargs{args, std::vector<int>{200}, 2, 50};

                if (net.autoRetry(Opt::onlyRequest, Aargs)) {
                    nlog::Info(FI, LI, "%s : testing okay , host : %s , retCode : %d", FN, it, retCode);
                    neko::networkBase::Dconfig.host = std::string(it);
                    return;
                } else {
                    nlog::Warn(FI,LI,"%s : faild to testing host : %s , now try to the next",FN,it);
                }
            };
            nlog::Err(FI, LI, "%s : Test: No available hosts! End to network test", FN);
            return;
        });
    }

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

size_t neko::networkBase::WriteCallbackQBA(char *contents, size_t size, size_t nmemb, void *userp) {
    QByteArray *buffer = static_cast<QByteArray *>(userp);
    buffer->append(contents, size * nmemb);
    return size * nmemb;
}