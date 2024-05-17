    // one.h
    // cpp Lang
    // Version 0.0.2
    // https://github.com/moehoshio/one.h
    // MIT License
    // Welcome to  submit questions, light up star , error corrections (even just for better translations), and feature suggestions/construction.
    // :D
/*
MIT License
Copyright (c) 2024 Hoshi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include "err.h"// NekoL Project Customization

#include <algorithm>
#include <chrono>
#include <mutex>
#include <tuple>
#include <vector>

namespace one {

    // Already customized in the entrust function
    // using exception_ = std::runtime_error;
    
    
    inline namespace Opt {
        // Indicates the use of the default constructor.
        struct notUseConditionConstructor{};
    }; // namespace Opt

    // base, save list
    template <typename T, typename... Conditions>
    class oneMethod {
    private:
        static std::vector<std::tuple<Conditions...>> list;
    protected:
        static std::timed_mutex mtx;

    public:
        template <typename... Args>
        static void add(Args&&... args) noexcept {
            list.push_back(std::make_tuple(std::forward<Args...>(args...)));
        }

        static void add(std::tuple<Conditions...> &&t) noexcept {
            list.push_back(t);
        }
        template <typename... Args>
        static bool verified(Args&&... args) noexcept {
            return (std::find(list.begin(), list.end(), std::make_tuple(std::forward<Args...>(args...))) != list.end());
        }

        static bool verified(std::tuple<Conditions...> &&t) noexcept {
            return (std::find(list.begin(), list.end(), t) != list.end());
        }
        template <typename... Args>
        static void erase(Args&&... args) noexcept {
            list.erase(std::remove(list.begin(), list.end(), std::make_tuple(std::forward<Args...>(args...))), list.end());
        }

        static void erase(std::tuple<Conditions...> &&t) noexcept {
            list.erase(std::remove(list.begin(), list.end(), t), list.end());
        }
    };
    template <typename T, typename... Conditions>
    std::vector<std::tuple<Conditions...>> oneMethod<T, Conditions...>::list;
    template <typename T, typename... Conditions>
    std::timed_mutex oneMethod<T, Conditions...>::mtx;

    /// @brief Construct a T type object using parameters and ensure that there is only one instance of T object with the same parameters
    // need args identical , e.g <std::string,std::string> and <std::string,std::string> ,Only then will this be effective (Otherwise it is a different instance)
    // if is Custom type, there should be T&& Construct 、operator == and operator &&== 、new not =delete .
    // Here is an example that meets the criteria:
    /*
    struct X{
        int i = 0;
        X(const X& x): i(x){}
        bool operator==(const X& x)const{return (i==x)? true : false ;}
        X& operator=(const X& x) { i = x.i; return *this;}
    };
    */
    /// @param Conditions... , If it's a pointer, the comparison will be based on the pointer address (including char*!), To compare whether strings are the same, std::string should be used
    /// @param condition Condition generates one additional copy.
    template <typename T, typename... Conditions>
    class one : private oneMethod<T, Conditions...> {
        
    private:
        std::tuple<Conditions...> data;
        struct Base {
            virtual inline T *get() = 0;
            virtual ~Base() {}
        };
        Base *base;
        struct retain : public Base {
            T *ptr;
            retain() {
                ptr = new T();
            };
            template <typename... Args>
            retain(Args&&... args) {
                ptr = new T{args...};
            }
            inline T *get() {
                return ptr;
            }
            ~retain() {
                delete ptr;
            }
        };
        struct package : public Base {
            T &obj;
            package(T &o) : obj(o){};
            inline T *get() {
                return &obj;
            };
        };

        template <typename... Args>
        inline void entrust(const std::chrono::milliseconds &t, Args&&... condition) {

            // if need Guaranteed not to be modified during traversal ,Just get the lock first(Need Unlock before throwing)
            if (this->verified(std::forward<Args...>(condition...)))
                throw nerr::error(nerr::errCode(1001), "There is the same", nerr::errType::TheSame);

            if (!this->mtx.try_lock_for(t))
                throw nerr::error(nerr::errCode(1002), "Get lock the time out", nerr::errType::TimeOut);

            this->add(std::forward<Args...>(condition...));
            data = std::make_tuple(std::forward<Args...>(condition...));
            this->mtx.unlock();
        }

    public:
        // The default constructor does nothing; if used, the init function should be called.
        one() noexcept {}
        // Constructing objects using constructArgs.
        template <typename... Args>
        one(Conditions... condition, std::chrono::milliseconds t, Args&&... constructArgs) {
            entrust(t, std::move(condition...));
            base = new retain(constructArgs...);
        }
        // Constructing objects using constructArgs.
        template <typename... Args>
        one(Conditions... condition, Args&&... constructArgs) : one(condition..., std::chrono::milliseconds(5000) , constructArgs...){};

        // Constructing objects using condition.
        one(Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000)) {
            entrust(t, std::move(condition...));
            base = new retain(condition...);
        }
        /// @param o Indicates the use of the default constructor.
        one(const Opt::notUseConditionConstructor &o, Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000)) {
            entrust(t, std::move(condition...));
            base = new retain();
        }
        // Passing reference wrappers for use after construction by the user.
        // For move semantics: right-value references should be passed directly as construction parameters; please select another constructor version.
        //  If the same condition already exists , or time out get lock ,throw ex.
        one(T &d, Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000)) {
            entrust(t, std::move(condition...));
            base = new package(d);
        }
        
        inline bool init(Conditions... condition,std::chrono::milliseconds t = std::chrono::milliseconds(5000)){
            try
            {
                entrust(t,std::move(condition...));
                base = new retain(condition...);
                return true;
            }
            catch(...)
            {
                return false;
            }
        }

        // If the same condition already exists , or time out get lock ,ret false.
        // If the returns false, it can be called again until successful. There should be no resource leaks.
        template <typename... Args>
        inline bool init(Conditions... condition, std::chrono::milliseconds t, Args&&... constructArgs) noexcept {
            try {
                entrust(t, std::move(condition...));
                base = new retain(constructArgs...);
                return true;
            } catch (...) {
                return false;
            }
        }
        // If the same condition already exists , or time out get lock ,ret false.
        // If the returns false, it can be called again until successful. There should be no resource leaks.
        template <typename... Args>
        inline bool init(Conditions... condition, Args &&...constructArgs)noexcept {
            return init(condition..., std::chrono::milliseconds(5000), constructArgs...);
        }
        // Passing reference wrappers for use after construction by the user.
        // For move semantics: right-value references should be passed directly as construction parameters; please select another constructor version.
        //  If the same condition already exists , or time out get lock ,throw ex.
        // If the returns false, it can be called again until successful. There should be no resource leaks.
        inline bool init(T &d, Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000)) noexcept {
            try {
                entrust(t, std::move(condition...));
                base = new package(d);
                return true;
            } catch (...) {
                return false;
            }
        }
        /// @param o Indicates the use of the default constructor.
        //// If the same condition already exists , or time out get lock ,ret false.
        // If the returns false, it can be called again until successful. There should be no resource leaks.
        inline bool init(const Opt::notUseConditionConstructor &o, Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000) ) noexcept {
            try {
                entrust(t, std::move(condition...));
                base = new retain();
                return true;
            } catch (...) {
                return false;
            }
        }

        ~one() noexcept {
            this->mtx.lock();
            this->erase(std::move(data));
            delete base;
            this->mtx.unlock();
        }

        inline operator T &() noexcept {
            return *base->get();
        }

        inline operator T *() const noexcept {
            return base->get();
        }

        inline T *get() const noexcept {
            return base->get();
        }
    };

    // It differs from `one` in that it can directly access the member object, directly holding the member instead of a pointer.
    /// @brief Construct a T type object using parameters and ensure that there is only one instance of T object with the same parameters
    // need args identical , e.g <std::string,std::string> and <std::string,std::string> ,Only then will this be effective (Otherwise it is a different instance)
    // if is Custom type, there should be T&& Construct 、operator == and operator &&== 、new not =delete
    /// @param Args... , If it's a pointer, the comparison will be based on the pointer address (including char*!), To compare whether strings are the same, std::string should be used
    /// @param condition Condition generates one additional copy.
    template <typename T, typename... Conditions>
    struct oneR : private oneMethod<T, Conditions...> {
        T obj;
        std::tuple<Conditions...> data;
        template <typename... Args>
        inline void entrust(std::chrono::milliseconds t, Args&&... args) {
            // if need Guaranteed not to be modified during traversal ,Just get the lock first(Need Unlock before throwing)
            if (this->verified(std::forward<Args...>(args...)))
                throw nerr::error(nerr::errCode(1001), "There is the same", nerr::errType::TheSame);

            if (!this->mtx.try_lock_for(t))
                throw nerr::error(nerr::errCode(1002), "Get lock the time out", nerr::errType::TimeOut);

            this->add(std::forward<Args...>(args...));
            data = std::make_tuple(std::forward<Args...>(args...));
            this->mtx.unlock();
        }
        oneR() noexcept {};
        template <typename... Args>
        oneR(std::chrono::milliseconds t, Conditions... condition, Args&&... args) {
            entrust(t, std::move(condition...));
            obj = T{args...};
        };
        oneR(const Opt::notUseConditionConstructor &o, Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000U)) {
            entrust(t, std::move(condition...));
        }
        inline bool init(const Opt::notUseConditionConstructor &o, Conditions... condition, std::chrono::milliseconds t = std::chrono::milliseconds(5000U)) {
            try {
                entrust(t, std::move(condition...));
                return true;
            } catch (...) {
                return false;
            }
        }
        template <typename... Args>
        inline bool init(std::chrono::milliseconds t, Conditions... condition, Args&&... args) noexcept {
            try {
                entrust(t, std::move(condition...));
                obj = T{args...};
                return true;
            } catch (...) {
                return false;
            }
        }
        // After moving, the original member object should no longer be used.
        // And attention needs to be paid to the issue of object lifetimes (including the lifetimes of objects and lists).
        inline decltype(auto) move(){
            return std::move(obj);
        };

        ~oneR() {
            this->mtx.lock();
            this->erase(std::move(data));
            this->mtx.unlock();
        }
    };

} // namespace one
