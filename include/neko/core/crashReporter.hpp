#pragma once

#include <string>

namespace neko::core::crash {

    /**
     * @brief Mark the start of a run. Sets the crash flag to true and returns the previous state.
     */
    bool markRunStart();

    /**
     * @brief Mark a clean shutdown by setting the crash flag to false.
     */
    void markCleanShutdown();

    /**
     * @brief Upload logs if the previous run did not shut down cleanly.
     * @param previousRunUnclean true if the last recorded run was unclean.
     */
    void uploadLogsIfNeeded(bool previousRunUnclean);

} // namespace neko::core::crash
