#pragma once
#include <curl/curl.h>

#include <algorithm>
#include <future>
#include <string>

#include "err.h"
#include "exec.h"
#include "fn.h"
#include "info.h"
#include "io.h"
#include "log.h"

namespace neko {

    class networkBase {
    public:
        using RetHttpCode = int;

        // if not waiting , testing host (set Dconfig.host) may not be ready
        static std::future<void> init();

        struct Config {
            std::string userAgent;
            std::string proxy;
            std::string host;
            std::string protocol;
        };

        struct Api {
            constexpr static const char *hostList[]{
#include "../data/hostlist"
            };
            constexpr static const char *mainenance = "/api/maintenance";
            constexpr static const char *checkUpdates = "/api/checkUpdates";
            constexpr static const char *testing = "/testing/ping";
            // constexpr static const char *token = "api/token";
        };

        static Config Dconfig;
        constexpr static Api api;

        enum class Opt {
            none,
            onlyRequest,
            downloadFile,
            postText,
            postFile,
            getSize,        // using getCase or getSize . with return value.
            getContentType, // using getCase func. with return value.
            getContent,     // with return value.
            getHeadContent  // with return value.

        };

        static std::unordered_map<Opt, std::string> optMap;

        inline static auto optStr(Opt opt) {
            for (const auto &it : optMap) {
                if (it.first == opt) {
                    return it.second;
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
        template <typename T = std::string>
        constexpr static T buildUrl(const T &path, const T &host = Dconfig.host, const T &protocol = Dconfig.protocol) {
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
            if (tlsProxy)
                return T(tlsProxy);
            else if (proxy)
                return T(proxy);

            return T();
        }

        // use std string save ,Can be used in most cases ,binary or text
        static size_t WriteCallbackString(char *ptr, size_t size, size_t nmemb, void *userdata);
        // ofstream
        static size_t WriteCallbackFile(char *contents, size_t size, size_t nmemb, void *userp);
        // write QByteArray
        static size_t WriteCallbackQBA(char *contents, size_t size, size_t nmemb, void *userp);
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

        inline static void handleNerr(const nerr::error &e, const char *file, unsigned int line, const char *formFuncName, const char *id, RetHttpCode *ret) {
            switch (e.type) {
                case nerr::errType::TheSame:
                case nerr::errType::TimeOut:
                    doErr(file, line, std::string(std::string(e.msg) + std::string(", id :") + std::string(id)).c_str(), (std::string(FN) + formFuncName).c_str(), ret, e.code);
                    break;
                default:
                    doErr(file, line, std::string(std::string(e.msg) + std::string(", id :") + std::string(id)).c_str(), (std::string(FN) + formFuncName).c_str(), ret, e.code);
                    break;
            }
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
            std::string resBreakPointStr = (args.resBreakPoint) ? "true" : "false";
            std::string userAgent = (args.userAgent) ? args.userAgent : args.config.userAgent.c_str();
            std::string optStrT = optStr(opt);
            nlog::Info(FI, LI,
                       "%s : url : %s , opt : %s , fileName : %s , range : %s , resBreakPoint : %s , userAgent : %s , protocol : %s , proxy : %s , system proxy : %s ,data : %s , id : %s",
                       FN, args.url, optStrT.c_str(), args.fileName, args.range,
                       resBreakPointStr.c_str(),
                       userAgent.c_str(),
                       args.config.protocol.c_str(),
                       args.config.proxy.c_str(),
                       getSysProxy<const char *>(),
                       args.data,
                       args.id);
        }
        inline static bool initOpt(CURL *curl, Args &args) {
            if (!curl) {
                doErr(FI, LI, std::string(std::string("Failed to initialize curl. id : ") + std::string(args.id)).c_str(), FN, args.code, -1);
                return false;
            }

            if (args.config.proxy == "true")
                curl_easy_setopt(curl, CURLOPT_PROXY, getSysProxy<const char *>());
            else if (exec::isProxyAddress(args.config.proxy))
                curl_easy_setopt(curl, CURLOPT_PROXY, args.config.proxy.c_str());
            else
                curl_easy_setopt(curl, CURLOPT_NOPROXY);

            if (args.resBreakPoint) {
                try {
                    oneIof infile(args.fileName, args.fileName, std::ios::in | std::ios::binary);

                    if (infile.get()->is_open()) {
                        infile.get()->seekg(0, std::ios::end);
                        size_t file_size = infile.get()->tellg();
                        infile.get()->close();
                        handleFileResume(args.range, curl, file_size);
                    }

                } catch (const nerr::error &e) {
                    handleNerr(e, FI, LI, FN, args.id, args.code);
                    return false;
                } catch (const std::exception &e) {
                    handleStdError(e, FI, LI, FN, args.id, args.code);
                    return false;
                }
            } // resBreakPoint

            curl_easy_setopt(curl, CURLOPT_USERAGENT, (args.userAgent) ? args.userAgent : args.config.userAgent.c_str());
            curl_easy_setopt(curl, CURLOPT_URL, args.url);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5L);
            if (args.range)
                curl_easy_setopt(curl, CURLOPT_RANGE, args.range);
            return true;
        }

        inline static bool perform(CURL *curl, RetHttpCode *ref, const char *id) {
            nlog::Info(FI, LI, "%s : Now start perform , id : %s", FN, id);
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::string msg(std::string("get network req failed ! :") + std::string(curl_easy_strerror(res) + std::string(" id :") + std::string(id)));
                doErr(FI, LI, msg.c_str(), FN, ref, -4);
                curl_easy_cleanup(curl);
                return false;
            }
            nlog::Info(FI, LI, "%s : perform is okay , id : %s", FN, id);
            return true;
        }

        inline static void setRetCodeAndClean(CURL *curl, RetHttpCode *ref, const char *id) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, ref);
            curl_easy_cleanup(curl);
            nlog::Info(FI, LI, "%s : this ref code : %d , id : %s", FN, *ref, id);
        }

    public:
        // Exceptions should not be thrown unless thrown within the constructor of T.
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
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, args.writeCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file.get());
                        if (!perform(curl, args.code, args.id))
                            return;
                        setRetCodeAndClean(curl, args.code, args.id);

                    } catch (const nerr::error &e) {
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
                    doErr(FI, LI, std::string(std::string("The incorrect method was used! (The selected method has a return value, but a function that does not provide a return value was used.) id : ") + args.id).c_str(), FN, args.code, -5);
                    break;
                }
            }
        }

        // This method can only be used with Opt::getSize and Opt::getContentType.
        inline std::string getCase(Opt opt, Args &args) {
            doLog(opt, args);

            if (opt != Opt::getSize && opt != Opt::getContentType) {
                doErr(FI, LI, std::string(std::string("Invalid method! Only Opt::getSize and Opt::getContentType can use this method(getCase)! id : ") + args.id).c_str(), FN, args.code, -8);
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
                doErr(FI, LI, std::string(std::string("Invalid Content-Length value. id : ") + args.id).c_str(), FN, args.code, -6);
            } catch (const std::out_of_range &e) {
                doErr(FI, LI, std::string(std::string("Content-Length value out of range. id :") + args.id).c_str(), FN, args.code, -7);
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
            } catch (const nerr::error &e) {
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
                    doErr(FI, LI, std::string(std::string("The incorrect method was used! (The selected method has a return value, but it was used with an option that does not have a return value.) id : ") + args.id).c_str(), FN, args.code, -5);
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
                        return true;
                    }
                }
                *ra.args.code = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(ra.sleep));
            }
            return T();
        }

        inline auto nonBlockingDo(Opt opt, Args &args) -> std::future<void> {
            return exec::getThreadObj().enqueue(
                [] { Do(opt, args); });
        }
        inline auto nonBlockingGet(Opt opt, Args &args) -> std::future<T> {
            return exec::getThreadObj().enqueue(
                [] { return get(opt, args); });
        }
        inline auto nonBlockingGetPtr(Opt opt, Args &args) -> std::future<T *> {
            return exec::getThreadObj().enqueue(
                [] { return getPtr(opt, args); });
        }

        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code.
        inline bool Multi(Opt opt, const MultiArgs &ma) {
            MultiArgs args{ma};
            Multi(opt, args);
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
                // test
                std::string name("./temp/" + exec::generateRandomString(12) + "-" + std::to_string(i));
                std::string id(std::string(ma.args.id) + "-" + std::to_string(i));

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
                if (!ret && !exec::getThreadObj().enqueue([=, this] {
                                                     Args args{ma.args};
                                                     args.range = list[i].range.c_str();
                                                     args.fileName = list[i].name.c_str();
                                                     args.id = list[i].id.c_str();
                                                     return autoRetry(opt, autoRetryArgs{args, ma.code});
                                                 })
                                 .get()) {
                    nlog::Err(FI, LI, "%s :  i : %d state : fail to twice ! , range : %s , file : %s , id : %s", FN, i, args.range, args.fileName, args.id);
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

} // namespace neko