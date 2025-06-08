#pragma once

namespace neko {

    inline namespace types {

        enum class State {
            Completed,     // Operation finished successfully, no further action needed
            ActionNeeded,  // Action required from user or system
            RetryRequired, // Temporary failure, should retry later
        };
    } // namespace types

} // namespace neko
