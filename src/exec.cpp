#include "exec.h"
namespace exec
{
    // NekoL Project Customization
    
    neko::ThreadPool &getThreadObj(){
        static neko::ThreadPool obj;
        return obj;
    };

    CSimpleIniA &getConfigObj(){
        static CSimpleIniA obj;
        return obj;
    }
} // namespace exec
