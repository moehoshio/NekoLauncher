#pragma once
#include <curl/curl.h>

#include <future>
#include <string>

#include "err.h"
#include "fn.h"
#include "io.h"
#include "log.h"
#include "exec.h"
#include "info.h"

namespace neko {

    class networkBase {
    public:
        
        using RetHttpCode = int;

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
            constexpr static const char *mainenance{"/api/get?maintenance"};
            constexpr static const char *testing{"/testing/ping"};
            constexpr static const char *info{"/api/get?info"};
            constexpr static const char *token{"api/get?token"};
        };

        static Config Dconfig;
        constexpr static Api api;

        

        enum class Opt {
            none,
            onlyRequest,
            downloadFile,
            postText,
            postFile,
            retPostText,    // with return value.
            getSize,        // generally speaking, using size_t or T support size_t constructing . with return value.
            getContent,     // with return value.
            getContentType, // T requires support std::string constructing. with return value.
            // use getContentAndStorage (to file) func, T requires operator<< (std::ostream&). with return value.
            getHeadContent // with return value.

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
        inline static auto strOpt(const std::string_view str){
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
            return exec::sum(protocol,host,path);
        }

        template <typename T = std::string, typename... Paths>
        constexpr static T buildUrlP(const T &protocol, const T &host, Paths &&...paths) {
            return exec::sum(protocol, host, std::forward<Paths...>(paths...));
        }

        // use std string save ,Can be used in most cases ,binary or text
        static size_t Write_Callback(char *ptr, size_t size, size_t nmemb, void *userdata);

        static size_t Header_Callback(char *ptr, size_t size, size_t nmemb, void *userdata);

        // write file
        static size_t Write_CallbackFile(char *contents, size_t size, size_t nmemb, void *userp);

        // write QByteArray
        static size_t Write_CallbackQBA(char *contents, size_t size, size_t nmemb, void *userp);
    };

    template <typename T = std::string, typename F = decltype(networkBase::Write_Callback), typename F2 = decltype(networkBase::Header_Callback)>
    class network : public networkBase {
        static_assert(!std::is_pointer<T>::value, "Error: Template parameter T cannot be a pointer type! If using pointer types, please use the T * getPtr function.");

    public:
        using TType = T;
        using FType = F;
        network() = default;
        // This constructor can be used for type deduction. Maybe
        network(T, F){};
        struct Args {
            const char *url;
            const char *fileName = nullptr;
            RetHttpCode *code = nullptr;
            bool resBreakPoint = false;
            const char *range = nullptr;
            const char *userAgent = nullptr;
            const char *data = nullptr;
            F *writeCallback = networkBase::Write_Callback;
            F2 *headerCallback = networkBase::Header_Callback;
            networkBase::Config config = networkBase::Dconfig;
        };
        struct autoRetryArgs {
            Args args;
            int code = 200; // Try again if this is not the case.
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
            int code = 200;                     // considered a success
        };

    private:
        inline void doErr(const char * file , unsigned int line, const char *msg, const char *formFuncName, RetHttpCode *ref, int val) {
            if (ref)
                *ref = val;
            nlog::Err(file,line,"%s : %s", formFuncName, msg);
        }
        template <typename... Args>
        inline void doErr(const char * file , unsigned int line, const char *msg, const char *formFuncName, RetHttpCode *ref, int val, Args &&...args) {
            if (ref)
                *ref = val;
            nlog::Err(file,line,"%s : %s", formFuncName, (msg + std::string(args...)).c_str());
        }

        inline void handleNerr(const nerr::error &e,const char * file , unsigned int line,  const char *formFuncName, RetHttpCode *ret) {
            switch (e.type) {
                case nerr::errType::TheSame:
                case nerr::errType::TimeOut:
                    doErr(file,line,e.msg, (std::string(FN) + formFuncName).c_str(), ret, e.code);
                    break;
                default:
                    doErr(file,line,e.msg, (std::string(FN) + formFuncName).c_str(), ret, e.code);
                    break;
            }
        }

        inline void handleStdError(const std::exception &e, const char * file , unsigned int line, const char *formFuncName, RetHttpCode *ret) {
            if (ret)
                *ret = -3;
            nlog::Err(file,line,"%s(%s) :%s", FN, formFuncName, e.what());
        }

        inline void handleFileResume(const char *range, CURL *curl, size_t file_size) {
            if (range) {
                size_t resume_offset = file_size + std::stoull(std::string(range).substr(0, std::string(range).find('-')));
                curl_easy_setopt(curl, CURLOPT_RESUME_FROM, resume_offset);
            } else
                curl_easy_setopt(curl, CURLOPT_RESUME_FROM, file_size);
        }

        inline void doLog(Opt opt, const Args &args) {
            std::string resBreakPointStr = (args.resBreakPoint) ? "true" : "false";
            std::string userAgent = (args.userAgent) ? args.userAgent : args.config.userAgent.c_str();
            std::string optStrT = optStr(opt);
            nlog::Info(FI,LI,
                "%s : url : %s , opt : %s , fileName : %s , range : %s , resBreakPoint : %s , userAgent : %s , protocol : %s , proxy : %s , data : %s ",
                FN, args.url, optStrT.c_str(), args.fileName, args.range,
                resBreakPointStr.c_str(),
                userAgent.c_str(),
                args.config.protocol.c_str(),
                args.config.proxy.c_str(),
                args.data);
        }
        inline bool initOpt(CURL *curl, Args &args) {
            if (!curl) {
                doErr(FI,LI,"Failed to initialize curl", FN, args.code, -1);
                return false;
            }

            if (args.config.proxy == "")
                curl_easy_setopt(curl, CURLOPT_NOPROXY);
            else if (args.config.proxy != "true")
                curl_easy_setopt(curl, CURLOPT_PROXY, args.config.proxy.c_str());
                

            if (args.resBreakPoint) {
                try {
                    oneIof infile(args.fileName,args.fileName ,std::ios::in | std::ios::binary);

                    if (infile.get()->is_open()) {
                        infile.get()->seekg(0, std::ios::end);
                        size_t file_size = infile.get()->tellg();
                        infile.get()->close();
                        handleFileResume(args.range, curl, file_size);
                    }

                } catch (const nerr::error &e) {
                    handleNerr(e,FI,LI, FN, args.code);
                    return false;
                } catch (const std::exception &e) {
                    handleStdError(e,FI,LI, FN, args.code);
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

        inline bool perform(CURL *curl, RetHttpCode *ref) {
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                std::string msg = "get network req failed ! ：" + std::string(curl_easy_strerror(res)) + "";
                doErr(FI,LI,msg.c_str(), FN, ref, -4);
                curl_easy_cleanup(curl);
                return false;
            }
            return true;
        }

        inline void setRetCodeAndClean(CURL *curl, RetHttpCode *ref) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, ref);
            curl_easy_cleanup(curl);
        }

    public:
        // Exceptions should not be thrown unless thrown within the constructor of T.
        inline void Do(Opt opt, Args &args) noexcept {
            doLog(opt, args);

            CURL *curl = curl_easy_init();
            if (!initOpt(curl, args))
                return;

            switch (opt) {
                case Opt::onlyRequest: {
                    if (!perform(curl, args.code))
                        return;
                    setRetCodeAndClean(curl, args.code);
                    break;
                }
                case Opt::downloadFile: {
                    try {

                        oneIof file(args.fileName ,args.fileName, std::ios::out | std::ios::binary | ((args.resBreakPoint) ? std::ios::app : std::ios::trunc));

                        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, args.headerCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, args.writeCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file.get());
                        if (!perform(curl, args.code))
                            return;
                        setRetCodeAndClean(curl, args.code);

                    } catch (const nerr::error &e) {
                        handleNerr(e, FI,LI,FN, args.code);
                    } catch (const std::exception &e) {
                        handleStdError(e, FI,LI,FN, args.code);
                    }

                    break;
                }
                case Opt::postText: {
                    // To be implemented...
                    break;
                }
                case Opt::postFile: {
                    // To be implemented...
                    break;
                }
                default: {
                    doErr(FI,LI,"The incorrect method was used! (The selected method has a return value, but a function that does not provide a return value was used.)", FN, args.code, -5);
                    break;
                }
            }
        }

        // This method can only be used with Opt::getSize and Opt::getContentType.
        inline std::string getCase(Opt opt, Args &args) {
            doLog(opt, args);

            if (opt != Opt::getSize && opt != Opt::getContentType) {
                doErr(FI,LI,"Invalid method! Only Opt::getSize and Opt::getContentType can use this method(getCase)!", FN, args.code, -8);
                return std::string();
            }

            CURL *curl = curl_easy_init();
            if (!initOpt(curl, args))
                return std::string();

            std::string res;
            curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
            curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, args.headerCallback);
            curl_easy_setopt(curl, CURLOPT_HEADERDATA, &res);
            if (!perform(curl, args.code))
                return std::string();
            setRetCodeAndClean(curl, args.code);
            if (res.empty())
                return std::string();

            std::transform(res.begin(), res.end(), res.begin(), ::tolower);
            std::size_t pos = res.find("Content-Type:");
            if (pos == std::string::npos)
                return std::string();

            std::size_t start = pos + ((opt == Opt::getSize) ? 15 : 13); // Skip "content-length:" or "type" length
            std::size_t end = res.find_first_of("\r\n", start);
            std::string content_str = res.substr(start, end - start);

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
                doErr(FI,LI,"Invalid Content-Length value: ", FN, args.code, -6, res.c_str());
            } catch (const std::out_of_range &e) {
                doErr(FI,LI,"Content-Length value out of range: ", FN, args.code, -7, res.c_str());
            }
            return 0;
        }
        // T must support operator<< for std::ostream
        inline T getContentAndStorage(Args &args) noexcept {
            T res = get(Opt::getContent, args);
            try {
                oneIof file(args.fileName,args.fileName, std::ios::out | std::ios::binary | ((args.resBreakPoint) ? std::ios::app : std::ios::trunc));
                (*file.get()) << res;
                return res;
            } catch (const nerr::error &e) {
                handleNerr(e,FI,LI, FN, args.code);
            } catch (const std::exception &e) {
                handleStdError(e,FI,LI, FN, args.code);
            }
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
                case Opt::retPostText: {
                    // To be implemented...
                    break;
                }
                case Opt::getContent:
                case Opt::getHeadContent: {
                    if (opt == Opt::getHeadContent) {
                        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // 使用HEAD请求
                        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
                        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, args.headerCallback);
                    } else {
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, args.writeCallback);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
                    }
                    if (!perform(curl, args.code))
                        return T();
                    setRetCodeAndClean(curl, args.code);
                    return ret;
                    break;
                }
                default: {
                    doErr(FI,LI,"The incorrect method was used! (The selected method has a return value, but it was used with an option that does not have a return value.)", FN, args.code, -5);
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
            nlog::Info(FI,LI,"%s : expect code : %d , sleep : %d , times : %d", FN, ra.code, ra.sleep, ra.times);

            for (size_t i = 0; i < ra.times; ++i) {

                Do(opt, ra.args);
                nlog::Info(FI,LI,"%s : this req code : %d", FN, *ra.args.code);
                if (((*ra.args.code) == ra.code) ||
                    // 416 represent the file size requested for download in the breakpoint continuation exceeded the actual size.
                    // this if it is enabled res BreakPoint ,default download end.
                    ((ra.args.resBreakPoint)
                         ? ((*ra.args.code) == 416)
                         : false)) {
                    return true;
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
            nlog::Info(FI,LI,"%s : expect code : %d , sleep : %d , times : %d", FN, ra.code, ra.sleep, ra.times);
            RetHttpCode code;
            if (!ra.args.code) {
                ra.args.code = &code;
            }

            for (size_t i = 0; i < ra.times; ++ra.times) {

                T res = get(opt, ra.args);
                nlog::Info(FI,LI,"%s : this req code : %d", FN, *ra.args.code);
                if (*ra.args.code == ra.code ||
                            // 416 represent the file size requested for download in the breakpoint continuation exceeded the actual size.
                            // this if it is enabled res BreakPoint ,default download end.
                            (ra.args.resBreakPoint)
                        ? (*ra.args.code == 416)
                        : false) {
                    return res;
                }
                *ra.args.code = 0;
                std::this_thread::sleep_for(std::chrono::milliseconds(ra.sleep));
            }
            return T();
        }

        inline auto nonBlockingDo(Opt opt, Args &args) -> std::future<void> {
            return info::getThreadObj().enqueue(Do(opt, args));
        }
        inline auto nonBlockingGet(Opt opt, Args &args) -> std::future<T> {
            return info::getThreadObj().enqueue(get(opt, args));
        }
        inline auto nonBlockingGetPtr(Opt opt, Args &args) -> std::future<T *> {
            return info::getThreadObj().enqueue(getPtr(opt, args));
        }

        // Accepts parameters as rvalue objects, indicating the discarding of the returned HTTP code.
        inline bool Multi(Opt opt, const MultiArgs &ma) {
            MultiArgs args{ma};
            Multi(opt, args);
        }
        inline bool Multi(Opt opt, MultiArgs &ma) {


            constexpr size_t fiveM = (5 * 1024) * 1024;

            struct Data {
                std::string range;
                std::string name;
                std::future<bool> result;
            };

            std::vector<Data> list;

            size_t maxSize = autoRetry(Opt::getSize, autoRetryArgs{ma.args});

            size_t nums = 0;
            size_t oneSize = 0;

            switch (ma.approach) {
                case MultiArgs::Size: // fixed size 5MB
                    nums = maxSize / fiveM;
                    nlog::Info(FI,LI,"%s : approach : %s , used thread nums : %zu , expect nums : %d",FN,"SIze",ma.nums,ma.code);
                    break;
                case MultiArgs::Quantity: // fixed quantity 100 files
                    oneSize = maxSize / 100;
                    nlog::Info(FI,LI,"%s : approach : %s , used thread nums : %zu , expect nums : %d",FN,"Quantity",ma.nums,ma.code);
                    break;
                case MultiArgs::Auto: {
                    nlog::Info(FI,LI,"%s : approach : %s , used thread nums : %zu , expect nums : %d",FN,"Auto",ma.nums,ma.code);
                    if (maxSize < (10 * fiveM))
                        oneSize = maxSize / 100;
                    else
                        nums = maxSize / fiveM;
                    break;
                }
                default:
                    break;
            }

            for (size_t i = 0; i < (nums == 0) ? 100 : nums; ++i) {
                // If i is 0, then start range  is 0; otherwise, i * (oneSize or five).
                // If i equals nums, then end range is maxSize; otherwise, (i + 1) * (oneSize or five).
                std::string start = std::to_string((i == 0) ? 0 : (i * (nums == 0) ? oneSize : fiveM));
                std::string end = std::to_string((i == nums) ? maxSize : ((i + 1) * (nums == 0) ? oneSize : fiveM));
                std::string range = start + "-" + end;
                std::string name(info::getTemp() + "/" + exec::generateRandomString(12));

                Args args{ma.args};
                args.range = range.c_str();
                args.fileName = name.c_str();

                list.push_back(
                    Data{
                        range,
                        name,
                        exec::getThreadObj().enqueue(autoRetry(opt, autoRetryArgs{args})) //
                    });
            }

            for (size_t i = 0; i < list.size(); ++i) {
                Args args{ma.args};
                args.range = list[i].range.c_str();
                args.fileName = list[i].name.c_str();
                bool ret = list[i].result.get();
                if (!ret && !exec::getThreadObj().enqueue(autoRetry(opt, autoRetryArgs{args})).get()){
                    nlog::Err(FI,LI,"%s :  %d state : fail to twice ! , range : %s , file : %s",FN,i,args.range,args.fileName);
                    return false;
                }
                    
            }
            oneIof file;
            for (size_t i = 0; i < 3; ++i) {
                bool is = file.init(ma.args.fileName, std::chrono::seconds(10),ma.args.fileName);
                if (is)
                    break;
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }

            if (!file.get()->is_open()){
                nlog::Info(FI,LI,"%s : fail to open file %s! ",FN,ma.args.fileName);
                return false;
            }
                

            for (const auto &it : list) {
                std::ifstream ifs(it.name);
                if (!ifs.is_open())
                    nlog::Err(FI,LI,"%s : fail to open temp file %s !",it.name.c_str());
                (*file.get()) << ifs;
                ifs.close();
            }
            file.get()->close();
            return true;
        }

    }; // network

} // namespace neko