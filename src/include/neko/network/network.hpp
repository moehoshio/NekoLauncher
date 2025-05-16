#pragma once

#include "neko/log/nlog.hpp"

#include "neko/schema/nerr.hpp"
#include "neko/schema/nekodefine.hpp"

#include "neko/function/one.hpp"
#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"


#include <curl/curl.h>


#include <algorithm>
#include <future>
#include <string>

namespace neko {

    class networkBase {
    public:
        using RetHttpCode = int;

        // if not waiting , testing host (set Dconfig.host) may not be ready
        inline static std::future<void> init();

        struct Config {
            std::string userAgent;
            std::string proxy;
            std::string host;
            std::string protocol;
        };

        struct Api {
            constexpr static const char *hostList[] =
            {
                NetWorkHostListDefine
            };
                
            constexpr static const char *mainenance = "/v1/api/maintenance";
            constexpr static const char *checkUpdates = "/v1/api/checkUpdates";
            constexpr static const char *feedback = "/v1/api/feedbacklog";
            constexpr static const char *testing = "/v1/testing/ping";
            // static std::vector<std::string> dynamicHostList;
            struct Authlib {
                constexpr static const char *host = NetWorkAuthlibHostDefine;
                constexpr static const char *root = "/api/yggdrasil";
                constexpr static const char *authenticate = "/api/yggdrasil/authserver/authenticate";
                constexpr static const char *refresh = "/api/yggdrasil/authserver/refresh";
                constexpr static const char *validate = "/api/yggdrasil/authserver/validate";
                constexpr static const char *invalidate = "/api/yggdrasil/authserver/invalidate";
                constexpr static const char *signout = "/api/yggdrasil/authserver/signout";
                struct Injector {
                    constexpr static const char *downloadHost = "authlib-injector.yushi.moe";
                    constexpr static const char *getVersionsList = "/artifacts.json";
                    constexpr static const char *latest = "/artifact/latest.json";
                    constexpr static const char *baseRoot = "/artifact"; // + {build_number}.json
                };
                Injector injector;
            };
            Authlib authlib;
        };

        inline static networkBase::Config Dconfig;
        constexpr inline static Api api;

        enum class Opt {
            none,
            onlyRequest,
            downloadFile, // When opt is downloadFile, the specified callback function is invalid.
            postText,
            postFile,
            getSize,        // using getCase or getSize
            getContentType, // using getCase func.
            getContent,     // with return value.
            getHeadContent  // with return value.

        };

        inline static std::unordered_map<Opt, std::string_view> optMap = {
            {networkBase::Opt::downloadFile, "downloadFile"},
            {networkBase::Opt::onlyRequest, "onlyRequest"},
            {networkBase::Opt::postText, "postText"},
            {networkBase::Opt::postFile, "postFile"},
            {networkBase::Opt::getSize, "getSize"},
            {networkBase::Opt::getContent, "getContent"},
            {networkBase::Opt::getContentType, "getContentType"},
            {networkBase::Opt::getHeadContent, "getHeadContent"}};

        inline static std::string optStr(Opt opt) {
            for (const auto &it : optMap) {
                if (it.first == opt) {
                    return std::string(it.second);
                }
            }
            return std::string("unknown");
        }
        inline static auto strOpt(const std::string_view str) {
            for (const auto &it : optMap) {
                if (it.second == str) {
                    return it.first;
                }
            }
            return Opt::none;
        }

        /// @param path starts with /
        /// @param host should only contain the domain name.
        inline static auto buildUrl(const std::string &path, const std::string &host = Dconfig.host, const std::string &protocol = Dconfig.protocol) {
            return exec::sum(protocol, host, path);
        }

        template <typename T = std::string, typename... Paths>
        constexpr static T buildUrlP(const T &protocol, const T &host, Paths &&...paths) {
            return exec::sum(protocol, host, std::forward<Paths...>(paths...));
        }

        template <typename T = std::string>
        static T getSysProxy() {
            auto proxy = std::getenv("http_proxy");
            auto tlsProxy = std::getenv("https_proxy");
#if defined(_WIN32)
            if (!proxy) {
                HKEY hKey;
                LPCSTR lpSubKey = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
                if (RegOpenKeyExA(HKEY_CURRENT_USER, lpSubKey, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                    DWORD dwSize = 0;
                    DWORD dwType = 0;
                    RegQueryValueExA(hKey, "ProxyServer", 0, &dwType, 0, &dwSize);
                    if (dwSize > 0) {
                        char *proxyServer = new char[dwSize];
                        RegQueryValueExA(hKey, "ProxyServer", 0, &dwType, (LPBYTE)proxyServer, &dwSize);
                        std::string res(proxyServer);
                        if (res.find("http") == std::string::npos)
                            res.insert(0, "http://");
                        return T(res.c_str());
                    }
                    RegCloseKey(hKey);
                }
            }
#endif

            if (tlsProxy)
                return T(tlsProxy);
            else if (proxy)
                return T(proxy);

            return T();
        }
        template <typename T = std::string>
        constexpr static T errCodeReason(int code) {
            switch (code) {
                case -1:
                    return T("Failed to initialize libcurl.");
                    break;
                case -2:
                    return T("Failed to open file.");
                    break;
                case -3:
                    return T("Unexpected standard exception occurred");
                    break;
                case -4:
                    return T("Get network req failed !");
                    break;
                case -5:
                    return T("The use of an incorrect method/option");
                    break;
                case -6:
                    return T("Invalid Content-Length value.");
                    break;
                case -7:
                    return T("Content-Length value out of range");
                case -8:
                    return T("In getCase use invalid method! ");
                default:
                    return T("unknown");
                    break;
            }
        }

        // use std string save ,Can be used in most cases ,binary or text
        inline static size_t WriteCallbackString(char *ptr, size_t size, size_t nmemb, void *userdata) {
            std::string *buffer = reinterpret_cast<std::string *>(userdata);
            buffer->append(ptr, size * nmemb);
            return size * nmemb;
        };
        // ofstream
        inline static size_t WriteCallbackFile(char *contents, size_t size, size_t nmemb, void *userp) {
            std::fstream *file = static_cast<std::fstream *>(userp);
            file->write(contents, size * nmemb);
            return size * nmemb;
        };
    };

    template <typename T = std::string>
    class network : public networkBase {
        static_assert(!std::is_pointer<T>::value, "Error: Template parameter T cannot be a pointer type! If using pointer types, please use the T * getPtr function.");

    public:
        network() = default;
        struct Args {
            const char *url;
            const char *fileName = nullptr;
            RetHttpCode *code = nullptr;
            bool resBreakPoint = false;
            const char *range = nullptr;
            const char *userAgent = nullptr;
            const char *header = nullptr;
            const char *data = nullptr;
            const char *id = nullptr;
            size_t (*writeCallback)(char *, size_t, size_t, void *) = &networkBase::WriteCallbackString;
            size_t (*headerCallback)(char *, size_t, size_t, void *) = &networkBase::WriteCallbackString;
            networkBase::Config config = networkBase::Dconfig;
        };
        struct autoRetryArgs {
            Args args;
            std::vector<int> code = {200, 416}; // Try again if this is not the case.
            size_t times = 5;
            // milliseconds
            size_t sleep = 150;
        };
        /// @param nums threading nums , if is 0 ,auto use all
        /// @param code considered a success
        /// @param approach size: fixed size 5MB , quantity: fixed quantity 100 files
        struct MultiArgs {
            Args args;
            enum Approach {
                Auto = 0,
                Size = 1,
                Quantity = 2
            };
            size_t nums = 0;                    // threading nums , if is 0 ,auto use all
            Approach approach = Approach::Auto; // size: fixed size 5MB , quantity: fixed quantity 100 files
            std::vector<int> code = {200, 206}; // considered a success
        };

    private:
        inline static void doErr(const char *file, unsigned int line, const char *msg, const char *formFuncName, RetHttpCode *ref, int val) {
            if (ref)
                *ref = val;
            nlog::Err(file, line, "%s : %s", formFuncName, msg);
        }

        inline static void handleNerr(const nerr::Error &e, const char *file, unsigned int line, const char *formFuncName, const char *id, RetHttpCode *ret) {
            doErr(file, line, std::string(std::string(e.msg) + std::string(", id :") + std::string((id == nullptr) ? "" : id)).c_str(), (std::string(FN) + formFuncName).c_str(), ret, -2);
        }

        inline static void handleStdError(const std::exception &e, const char *file, unsigned int line, const char *formFuncName, const char *id, RetHttpCode *ret) {
            if (ret)
                *ret = -3;
            nlog::Err(file, line, "%s(%s) :%s id: %s", FN, formFuncName, e.what(), id);
        }

        inline static void handleFileResume(const char *range, CURL *curl, size_t file_size) {
            if (range) {
                size_t resume_offset = file_size + std::stoull(std::string(range).substr(0, std::string(range).find('-')));
                curl_easy_setopt(curl, CURLOPT_RESUME_FROM, resume_offset);
            } else
                curl_easy_setopt(curl, CURLOPT_RESUME_FROM, file_size);
        }

        inline static void doLog(Opt opt, const Args &args) {
            bool dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
                 debug = exec::getConfigObj().GetBoolValue("dev", "debug", false);
            std::string resBreakPointStr = (args.resBreakPoint) ? "true" : "false";
            std::string userAgent = (args.userAgent) ? args.userAgent : args.config.userAgent.c_str();
            std::string data = (dev && debug) ? ((args.data) ? args.data : "null") : "*****";
            std::string optStrT = optStr(opt);
            nlog::Info(FI, LI,
                       "%s : url : %s , opt : %s , fileName : %s , range : %s , resBreakPoint : %s , userAgent : %s , protocol : %s , proxy : %s , system proxy : %s ,data : %s , id : %s",
                       FN, args.url, optStrT.c_str(), args.fileName, args.range,
                       resBreakPointStr.c_str(),
                       userAgent.c_str(),
                       args.config.protocol.c_str(),
                       args.config.proxy.c_str(),
                       getSysProxy<const char *>(),
                       data.c_str(),
                       args.id);
        }
        inline static bool initOpt(CURL *curl, Args &args) {
            if (!curl) {
                doErr(FI, LI, std::string(std::string("Failed to initialize curl. id : ") + std::string((args.id == nullptr) ? "" : args.id)).c_str(), FN, args.code, -1);
                return false;
            }
            curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"); // https://curl.se/ca/cacert.pem

            if (auto sysProxy = getSysProxy<const char *>();
                args.config.proxy == "true" && exec::isProxyAddress(sysProxy))
                curl_easy_setopt(curl, CURLOPT_PROXY, getSysProxy<const char *>());
            else if (exec::isProxyAddress(args.config.proxy))
                curl_easy_setopt(curl, CURLOPT_PROXY, args.config.proxy.c_str());
            else
                curl_easy_setopt(curl, CURLOPT_PROXY, "");

            if (args.resBreakPoint) {
                try {
                    oneIof infile(args.fileName, args.fileName, std::ios::in | std::ios::binary);

                    if (infile.get()->is_open()) {
                        infile.get()->seekg(0, std::ios::end);
                        size_t file_size = infile.get()->tellg();
                        infile.get()->close();
                        handleFileResume(args.range, curl, file_size);
                    }

                } catch (const nerr::Error &e) {
                    handleNerr(e, FI, LI, FN, args.id, args.code);
                    return false;
                } catch (const std::exception &e) {
                    handleStdError(e, FI, LI, FN, args.id, args.code);
                    return false;
                }
            } // resBreakPoint

            if (args.header)
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, args.header));
            if (args.range)
                curl_easy_setopt(curl, CURLOPT_RANGE, args.range);

            curl_easy_setopt(curl, CURLOPT_USERAGENT, (args.userAgent) ? args.userAgent : args.config.userAgent.c_str());
            curl_easy_setopt(curl, CURLOPT_URL, args.url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);

            return true;
        }

        inline static bool perform(CURL *curl, RetHttpCode *ref, const char *id) {
            nlog::Info(FI, LI, "%s : Now start perform , id : %s", FN, id);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::string msg(std::string("Faild to get network req! :") + std::string(curl_easy_strerror(res) + std::string(" , id :") + ((id == nullptr) ? "" : std::string(id))));
                doErr(FI, LI, msg.c_str(), FN, ref, -4);
                curl_easy_cleanup(curl);
                return false;
            }
            nlog::Info(FI, LI, "%s : Perform is okay , id : %s", FN, id);
            return true;
        }

        inline static void setRetCodeAndClean(CURL *curl, RetHttpCode *ref, const char *id) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, ref);
            curl_easy_cleanup(curl);
            nlog::Info(FI, LI, "%s : this ref code : %d (0 is null), id : %s", FN, ((ref) ? *ref : 0), id);
        }

    public:
        // When opt is downloadFile, the specified callback function is invalid.
        inline static void Do(Opt opt, Args &args) noexcept {
            doLog(opt, args);

            CURL *curl = curl_easy_init();
            if (!initOpt(curl, args))
                return;

            switch (opt) {
                case Opt::onlyRequest: {
                    if (!perform(curl, args.code, args.id))
                        return;
                    setRetCodeAndClean(curl, args.code, args.id);
                    break;
                }
                case Opt::downloadFile: {
                    try {
                        oneIof file(args.fileName, std::chrono::milliseconds(5000), args.fileName, std::ios::out | std::ios::binary | ((args.resBreakPoint) ? std::ios::app : std::ios::trunc));
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &networkBase::WriteCallbackFile);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file.get());
                        if (!perform(curl, args.code, args.id))
                            return;
                        setRetCodeAndClean(curl, args.code, args.id);

                    } catch (const nerr::Error &e) {
                        handleNerr(e, FI, LI, FN, args.id, args.code);
                    } catch (const std::exception &e) {
                        handleStdError(e, FI, LI, FN, args.id, args.code);
                    } catch (...) {
                    }

                    break;
                }
                case Opt::postText: {
                    curl_easy_setopt(curl, CURLOPT_POST, 1L);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args.data);
                    if (!perform(curl, args.code, args.id))
                        return;
                    setRetCodeAndClean(curl, args.code, args.id);

                    break;
                }
                case Opt::postFile: {
                    // To be implemented...
                    break;
                }
                default: {
                    doErr(FI, LI, std::string(std::string("The incorrect method was used! (The selected method has a return value, but a function that does not provide a return value was used.) id : ") + ((args.id == nullptr) ? "" : std::string(args.id))).c_str(), FN, args.code, -5);
                    break;
                }
            }
        }

        // This method can only be used with Opt::getSize and Opt::getContentType.
        inline std::string getCase(Opt opt, Args &args) {
            doLog(opt, args);

            if (opt != Opt::getSize && opt != Opt::getContentType) {
                doErr(FI, LI, std::string(std::string("Invalid method! Only Opt::getSize and Opt::getContentType can use this method(getCase)! id : ") + ((args.id == nullptr) ? "" : std::string(args.id))).c_str(), FN, args.code, -8);
                return std::string();
            }

            CURL *curl = curl_easy_init();
            if (!initOpt(curl, args))
                return std::string();

            std::string res;
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, args.headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &res);
            if (!perform(curl, args.code, args.id))
                return std::string();
            setRetCodeAndClean(curl, args.code, args.id);
            if (res.empty())
                return std::string();

            std::transform(res.begin(), res.end(), res.begin(), ::tolower);
            std::size_t pos = res.find((opt == Opt::getSize) ? "content-length:" : "content-type:");
            if (pos == std::string::npos)
                return std::string();

            std::size_t start = pos + ((opt == Opt::getSize) ? 15 : 13); // Skip "content-length:" or "type" length
            std::size_t end = res.find_first_of("\r\n", start);
            std::string content_str = res.substr(start, end - start);

            nlog::Info(FI, LI, "%s : Exit , ret : %s", FN, content_str.c_str());
            return content_str;
        }

        inline size_t getSize(Args &args) noexcept {
            std::string res = getCase(Opt::getSize, args);
            if (res.empty())
                return 0;
            try {
                std::size_t size = std::stoull(res);
                return size;
            } catch (const std::invalid_argument &e) {
                doErr(FI, LI, std::string(std::string("Invalid Content-Length value. id : ") + ((args.id == nullptr) ? "" : std::string(args.id))).c_str(), FN, args.code, -6);
            } catch (const std::out_of_range &e) {
                doErr(FI, LI, std::string(std::string("Content-Length value out of range. id :") + ((args.id == nullptr) ? "" : std::string(args.id))).c_str(), FN, args.code, -7);
            }
            return 0;
        }
        // T must support operator<< for std::ostream
        inline T getContentAndStorage(Args &args) noexcept {
            T res = get(Opt::getContent, args);
            try {
                oneIof file(args.fileName, args.fileName, std::ios::out | std::ios::binary | ((args.resBreakPoint) ? std::ios::app : std::ios::trunc));
                (*file.get()) << res;
                return res;
            } catch (const nerr::Error &e) {
                handleNerr(e, FI, LI, FN, args.id, args.code);
            } catch (const std::exception &e) {
                handleStdError(e, FI, LI, FN, args.id, args.code);
            }
            return T();
        }

        // If error occurs, construct an empty object
        // Exceptions should not be thrown unless thrown within the constructor of T.
        // Opt::getSize and Opt::getContentType require the use of the getSize or getCase method.
        // Opt::getConentAndStorage : use getConentAndStorage method
        inline T get(Opt opt, Args &args) noexcept {

            doLog(opt, args);

            CURL *curl = curl_easy_init();
            if (!initOpt(curl, args))
                return T();

            T ret;
            switch (opt) {
                case Opt::postText: {
                    curl_easy_setopt(curl, CURLOPT_POST, 1L);
                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args.data);
                    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, args.writeCallback);
                    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
                    if (!perform(curl, args.code, args.id))
                        return T();
                    setRetCodeAndClean(curl, args.code, args.id);
                    return ret;
                    break;
                }
                case Opt::getContent:
                case Opt::getHeadContent: {
                    if (opt == Opt::getHeadContent) {
                        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // using HEAD
                        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
                        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, args.headerCallback);
                    } else {
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, args.writeCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
                    }
                    if (!perform(curl, args.code, args.id))
                        return T();
                    setRetCodeAndClean(curl, args.code, args.id);
                    return ret;
                    break;
                }
                default: {
                    doErr(FI, LI, std::string(std::string("The incorrect method was used! (The selected method has a return value, but it was used with an option that does not have a return value.) id : ") + ((args.id == nullptr) ? "" : std::string(args.id))).c_str(), FN, args.code, -5);
                    break;
                }
            }
            return T();
        }

        // Exceptions should not be thrown unless thrown within the constructor of T.
        inline T *getPtr(Opt opt, Args &args) noexcept {
            return new T(get(opt, args));
        }
        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code
        inline T *getPtr(Opt opt, const Args &args) noexcept {
            Args a{args};
            return getPtr(opt, a);
        }
        // Exceptions should not be thrown unless thrown within the constructor of T.
        inline std::shared_ptr<T> getShadPtr(Opt opt, Args &args) noexcept {
            return std::make_shared<T>(std::move(get(opt, args)));
        }
        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code
        inline std::shared_ptr<T> getShadPtr(Opt opt, const Args &args) noexcept {
            Args a{args};
            return getShadPtr(opt, a);
        }
        // Exceptions should not be thrown unless thrown within the constructor of T.
        inline std::unique_ptr<T> getUnqePtr(Opt opt, Args &args) noexcept {
            return std::make_unique<T>(std::move(get(opt, args)));
        }
        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code
        inline std::unique_ptr<T> getUnqePtr(Opt opt, const Args &args) noexcept {
            Args a{args};
            return getUnqePtr(opt, a);
        }

        inline bool autoRetry(Opt opt, autoRetryArgs &ra) {
            std::string expectCodes;
            for (auto it : ra.code) {
                expectCodes.append(std::to_string(it) + std::string(","));
            }
            nlog::Info(FI, LI, "%s : expect code : %s , sleep : %d , times : %d , id : %s", FN, expectCodes.c_str(), ra.sleep, ra.times, ra.args.id);

            for (size_t i = 0; i < ra.times; ++i) {

                Do(opt, ra.args);
                nlog::Info(FI, LI, "%s : this req code : %d , id : %s", FN, *ra.args.code, ra.args.id);
                for (auto it : ra.code) {
                    if (*ra.args.code == it) {
                        return true;
                    }
                }
                *ra.args.code = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(ra.sleep));
            }
            return false;
        };

        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code
        inline bool autoRetry(Opt opt, autoRetryArgs &&ra) {
            autoRetryArgs a{ra};
            return autoRetry(opt, a);
        }

        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code.
        inline T autoRetryGet(Opt opt, const autoRetryArgs &ra) {
            autoRetryArgs args{ra};
            return autoRetryGet(opt, args);
        }

        inline T autoRetryGet(Opt opt, autoRetryArgs &ra) {
            std::string expectCodes;
            for (auto it : ra.code) {
                expectCodes.append(std::to_string(it) + std::string(","));
            }
            nlog::Info(FI, LI, "%s : expect code : %s , sleep : %d , times : %d , id : %s", FN, expectCodes.c_str(), ra.sleep, ra.times, ra.args.id);
            RetHttpCode code;
            if (!ra.args.code) {
                ra.args.code = &code;
            }

            for (size_t i = 0; i < ra.times; ++ra.times) {

                T res = get(opt, ra.args);
                nlog::Info(FI, LI, "%s : this req code : %d , id : %s", FN, *ra.args.code, ra.args.id);
                for (auto it : ra.code) {
                    if (*ra.args.code == it) {
                        return res;
                    }
                }
                *ra.args.code = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(ra.sleep));
            }
            return T();
        }

        inline auto nonBlockingDo(Opt opt, Args &args) -> std::future<void> {
            return exec::getThreadObj().enqueue(
                [=, this] { Do(opt, args); });
        }
        inline auto nonBlockingGet(Opt opt, Args &args) -> std::future<T> {
            return exec::getThreadObj().enqueue(
                [=, this] { return get(opt, args); });
        }
        inline auto nonBlockingGetPtr(Opt opt, Args &args) -> std::future<T *> {
            return exec::getThreadObj().enqueue(
                [=, this] { return getPtr(opt, args); });
        }

        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code.
        inline bool Multi(Opt opt, const MultiArgs &ma) {
            MultiArgs args{ma};
            return Multi(opt, args);
        }
        inline bool Multi(Opt opt, MultiArgs &ma) {
            nlog::Info(FI, LI, "%s : Enter , id :%s", FN, ma.args.id);
            constexpr size_t fiveM = (5 * 1024) * 1024;

            struct Data {
                std::string range;
                std::string name;
                std::string id;
                std::future<bool> result;
            };

            std::vector<Data> list;

            size_t maxSize = getSize(ma.args);
            if (maxSize == 0 && getSize(ma.args) == 0) {
                return false;
            }

            size_t numThreads = 0; // fixed size 5MB
            size_t chunkSize = 0;  // fixed quantity 100 files
            std::string expectCodes;
            for (auto it : ma.code) {
                expectCodes.append(std::to_string(it) + std::string(","));
            }

            switch (ma.approach) {
                case MultiArgs::Size: // fixed size 5MB
                    numThreads = maxSize / fiveM;
                    break;
                case MultiArgs::Quantity: // fixed quantity 100 files
                    chunkSize = maxSize / 100;
                    break;
                case MultiArgs::Auto: {
                    if (maxSize < (10 * fiveM))
                        chunkSize = maxSize / 100;
                    else
                        numThreads = maxSize / fiveM;
                    break;
                }
                default:
                    break;
            }
            nlog::Info(FI, LI, "%s : approach : %s , used thread nums : %zu , expect codes : %s , id: %s", FN, ((ma.approach == MultiArgs::Auto) ? "Auto" : (ma.approach == MultiArgs::Size) ? "SIze"
                                                                                                                                                                                             : "Quantity"),
                       ma.nums, expectCodes.c_str(), ma.args.id);
            nlog::Info(FI, LI, "%s : maxSize : %zu , numThreads : %zu , chunkSize : %zu , id : %s", FN, maxSize, numThreads, chunkSize, ma.args.id);
            for (size_t i = 0; i < ((numThreads != 0) ? numThreads : 100); ++i) {
                std::string start;
                std::string end;
                if (i != 0) {
                    int startVal = (i * ((numThreads != 0) ? fiveM : chunkSize)) + 1;
                    start = std::to_string(startVal);
                    nlog::Info(FI, LI, "%s : i not 0 , statrt = %s", FN, start.c_str());
                } else {
                    nlog::Info(FI, LI, "%s : i == 0 , statrt = 0", FN);
                    start = "0";
                }

                if (numThreads != 0) {
                    if (i != (numThreads - 1)) {
                        end = std::to_string(
                            (i + 1) * fiveM);
                        nlog::Info(FI, LI, "%s : nums not 0 , i not nums , end range : %s  (i + 1) * fiveM) , i : %zu , id : %s", FN, end.c_str(), i, ma.args.id);
                    } else {
                        end = std::to_string(maxSize);
                        nlog::Info(FI, LI, "%s : nums not 0 , i is nums (max) , end range : %s  (maxSize) , i : %zu , id : %s", FN, end.c_str(), i, ma.args.id);
                    }
                } else {
                    if (i != 99) {
                        end = std::to_string(
                            (i + 1) * chunkSize);
                        nlog::Info(FI, LI, "%s : chunkSize != 0 , i!= 100 , end range : %s  (i + 1) * chunkSize) , i : %zu , id : %s", FN, end.c_str(), i, ma.args.id);
                    } else {
                        end = std::to_string(maxSize);
                        nlog::Info(FI, LI, "%s : chunkSize !=0 , i is 100 , end range : %s  (maxSize) , i : %zu , id : %s", FN, end.c_str(), i, ma.args.id);
                    }
                }
                std::string range = start + "-" + end;
                std::string name(info::temp() + exec::generateRandomString(12) + "-" + std::to_string(i));
                std::string id(std::string((ma.args.id == nullptr) ? "" : ma.args.id) + "-" + std::to_string(i));

                list.push_back(
                    Data{
                        range,
                        name,
                        id,
                        exec::getThreadObj().enqueue([=, this] {
                            Args args{ma.args};
                            args.range = range.c_str();
                            args.fileName = name.c_str();
                            args.id = id.c_str();
                            return autoRetry(opt, autoRetryArgs{args, ma.code});
                        }) //
                    });
            }
            nlog::Info(FI, LI, "%s : Now check download state , id : %s", FN, ma.args.id);
            for (size_t i = 0; i < list.size(); ++i) {

                bool ret = list[i].result.get();
                if (!ret && !exec::getThreadObj().enqueue([=, &list, this] {
                                                     Args args{ma.args};
                                                     args.range = list[i].range.c_str();
                                                     args.fileName = list[i].name.c_str();
                                                     args.id = list[i].id.c_str();
                                                     return autoRetry(opt, {args, ma.code});
                                                 })
                                 .get()) {
                    nlog::Err(FI, LI, "%s :  i : %d state : fail to twice ! , range : %s , file : %s , id : %s", FN, i, list[i].range.c_str(), list[i].name.c_str(), list[i].id.c_str());
                    return false;
                }
            }
            oneIof file;
            file.init(ma.args.fileName);
            (*file.get()).open(ma.args.fileName, std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);

            if (!(*file.get()).is_open()) {
                nlog::Info(FI, LI, "%s : fail to open file %s! id : %s", FN, ma.args.fileName, ma.args.id);
                return false;
            }

            for (const auto &it : list) {
                std::ifstream src(it.name, std::ios::binary);
                if (!src.is_open())
                    nlog::Err(FI, LI, "%s : fail to open temp file %s ! id : %s", it.name.c_str(), ma.args.id);
                (*file.get()) << src.rdbuf();
                src.close();
            }
            file.get()->close();
            return true;
        }

    }; // network

    inline auto networkBase::init() -> std::future<void> {
        using namespace std::literals::string_literals;

        std::string proxy = exec::getConfigObj().GetValue("net", "proxy", "true");
        // "" or true or proxyAdd ,otherwise set ""
        bool proxyUnexpected = exec::allTrue((proxy != ""), (proxy != "true"), !exec::isProxyAddress(proxy));
        if (proxyUnexpected)
            proxy = ""s;

        bool
            dev = exec::getConfigObj().GetBoolValue("dev", "enable", false),
            tls = exec::getConfigObj().GetBoolValue("dev", "tls", true);

        networkBase::Dconfig = {
            "NekoLc /"s + info::getVersion() + "-" + info::getOsNameS() + "/" + std::string(build_id),
            proxy | exec::move,
            Api::hostList[0],
            (dev == true && tls == false) ? "http://"s : "https://"s};

        nlog::Info(FI, LI, "%s :  proxy : %s , dev: %s , tls : %s , protocol : %s , dUseragent : %s ", FN, Dconfig.proxy.c_str(), exec::boolTo<const char *>(dev), exec::boolTo<const char *>(tls), Dconfig.protocol.c_str(), Dconfig.userAgent.c_str());

        return exec::getThreadObj().enqueue([] {
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
                    nlog::Warn(FI, LI, "%s : faild to testing host : %s , now try to the next", FN, it);
                }
            };
            nlog::Err(FI, LI, "%s : Test: No available hosts! End to network test", FN);
            return;
        });
    };

} // namespace neko