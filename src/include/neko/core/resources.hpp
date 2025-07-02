#pragma once

#include "library/SimpleIni/SimpleIni.h"
#include "library/threadpool.hpp"
#include "neko/event/event.hpp"

namespace neko::core {

    /**
     * @brief Central access point for global resources in the Neko framework.
     * 
     * Provides access to shared global resources like the thread pool and configuration.
     * These resources are lazily initialized as static variables with global lifetime.
     */
    class Resources {
    public:
        /**
         * @brief Gets the global thread pool instance.
         * @return Reference to the global thread pool object.
         */
        static neko::ThreadPool& getThreadPool() {
            static neko::ThreadPool instance;
            return instance;
        }

        /**
         * @brief Gets the global configuration object.
         * @return Reference to the global SimpleIni configuration object.
         */
        static CSimpleIniA& getConfigObj() {
            static CSimpleIniA instance;
            return instance;
        }

        /**
         * @brief Gets the global event loop instance.
         * @return Reference to the global event loop object.
         */
        static event::EventLoop& getEventLoop() {
            static event::EventLoop instance;
            return instance;
        }
    };

    /**
     * @brief Gets the global thread pool instance.
     * @return Reference to the global thread pool object.
     * 
     * This is a convenience function that wraps Resources::getThreadPool()
     */
    inline neko::ThreadPool& getThreadPool() {
        return Resources::getThreadPool();
    }

    /**
     * @brief Gets the global configuration object.
     * @return Reference to the global SimpleIni configuration object.
     * 
     * This is a convenience function that wraps Resources::getConfigObj()
     */
    inline CSimpleIniA& getConfigObj() {
        return Resources::getConfigObj();
    }

    /**
     * @brief Gets the global event loop instance.
     * @return Reference to the global event loop object.
     */
    inline event::EventLoop& getEventLoop() {
        return Resources::getEventLoop();
    }

} // namespace neko::core
