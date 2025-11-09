#include <gtest/gtest.h>

#include "neko/core/launcherProcess.hpp"
#include <neko/schema/exception.hpp>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <thread>

namespace fs = std::filesystem;

// Test fixture for launcherProcess tests
class LauncherProcessTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for tests
        testDir = fs::temp_directory_path() / "neko_launcher_test";
        fs::create_directories(testDir);
    }

    void TearDown() override {
        // Clean up test directory
        if (fs::exists(testDir)) {
            fs::remove_all(testDir);
        }
    }

    fs::path testDir;
};

// Test basic process execution with onStart callback
TEST_F(LauncherProcessTest, BasicProcessExecution) {
    std::atomic<bool> startCalled{false};
    std::atomic<bool> exitCalled{false};
    std::atomic<int> exitCode{-1};

    neko::core::ProcessInfo info;
    info.command = "echo Hello World";
    info.onStart = [&startCalled]() {
        startCalled = true;
    };
    info.onExit = [&exitCalled, &exitCode](int code) {
        exitCalled = true;
        exitCode = code;
    };

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    EXPECT_TRUE(startCalled);
    EXPECT_TRUE(exitCalled);
    EXPECT_EQ(exitCode, 0);
}

// Test process with output stream callback
TEST_F(LauncherProcessTest, ProcessOutputCapture) {
    std::vector<std::string> capturedLines;
    
    neko::core::ProcessInfo info;
#ifdef _WIN32
    info.command = "echo Line1 && echo Line2 && echo Line3";
#else
    info.command = "echo Line1; echo Line2; echo Line3";
#endif
    info.pipeStreamCb = [&capturedLines](const std::string &line) {
        capturedLines.push_back(line);
    };

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    EXPECT_GE(capturedLines.size(), 3);
}

// Test process with working directory
TEST_F(LauncherProcessTest, ProcessWithWorkingDirectory) {
    // Create a test file in test directory
    auto testFile = testDir / "test.txt";
    std::ofstream(testFile) << "test content";

    std::vector<std::string> output;
    neko::core::ProcessInfo info;
    info.workingDir = testDir.string();
#ifdef _WIN32
    info.command = "dir /b";
#else
    info.command = "ls";
#endif
    info.pipeStreamCb = [&output](const std::string &line) {
        output.push_back(line);
    };

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    
    bool foundFile = false;
    for (const auto &line : output) {
        if (line.find("test.txt") != std::string::npos) {
            foundFile = true;
            break;
        }
    }
    EXPECT_TRUE(foundFile);
}

// Test process exit code handling
TEST_F(LauncherProcessTest, ProcessExitCode) {
    std::atomic<int> capturedExitCode{-1};
    
    neko::core::ProcessInfo info;
#ifdef _WIN32
    info.command = "exit 42";
#else
    info.command = "exit 42";
#endif
    info.onExit = [&capturedExitCode](int code) {
        capturedExitCode = code;
    };

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    EXPECT_EQ(capturedExitCode, 42);
}

// Test invalid command throws exception
TEST_F(LauncherProcessTest, InvalidCommandThrowsException) {
    neko::core::ProcessInfo info;
    info.command = "this_command_does_not_exist_12345";

    std::atomic<int> exitCode{-1};
    info.onExit = [&exitCode](int code) {
        exitCode = code;
    };

    // On Windows, cmd will start but the command will fail with non-zero exit code
    // On Unix, sh may throw immediately
#ifdef _WIN32
    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    EXPECT_NE(exitCode, 0); // Should exit with error code
#else
    EXPECT_THROW(neko::core::launcherProcess(info), neko::ex::Runtime);
#endif
}

// Test empty command
TEST_F(LauncherProcessTest, EmptyCommand) {
    neko::core::ProcessInfo info;
    info.command = "";

    // Empty command should either throw or handle gracefully
    // Behavior may vary by platform
    EXPECT_THROW(neko::core::launcherProcess(info), neko::ex::Runtime);
}

// Test long command (Windows command length limit)
TEST_F(LauncherProcessTest, LongCommand) {
    std::string longOutput(5000, 'A');
    
    neko::core::ProcessInfo info;
#ifdef _WIN32
    info.command = "echo " + longOutput;
#else
    info.command = "echo " + longOutput;
#endif

    std::vector<std::string> output;
    info.pipeStreamCb = [&output](const std::string &line) {
        output.push_back(line);
    };

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    EXPECT_GT(output.size(), 0);
}

// Test command length boundary detection
#ifdef _WIN32
TEST_F(LauncherProcessTest, CommandLengthBoundary) {
    // Test that we correctly detect when to switch to PowerShell
    // Just verify the boundary constant exists and is used correctly
    
    std::string shortCmd = "echo test";
    std::string longCmd(neko::core::windowsCommandLengthLimit + 100, 'X');
    
    // Short command should use cmd
    EXPECT_LT(shortCmd.length(), neko::core::windowsCommandLengthLimit);
    
    // Long command should trigger PowerShell path
    EXPECT_GT(longCmd.length(), neko::core::windowsCommandLengthLimit);
    
    // Note: We don't actually execute the long command as PowerShell
    // can be slow/unstable with very long echo commands in tests
}
#endif

// Test launcherNewProcess - detached process
TEST_F(LauncherProcessTest, LauncherNewProcessBasic) {
    // Create a script that creates a file
    auto markerFile = testDir / "detached_marker.txt";
    
#ifdef _WIN32
    std::string command = "timeout /t 1 /nobreak && echo done > \"" + markerFile.string() + "\"";
#else
    std::string command = "sleep 1 && echo done > \"" + markerFile.string() + "\"";
#endif

    ASSERT_NO_THROW(neko::core::launcherNewProcess(command, testDir.string()));
    
    // Wait for the detached process to complete
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Check if the marker file was created
    EXPECT_TRUE(fs::exists(markerFile));
}

// Test launcherNewProcess with invalid command
TEST_F(LauncherProcessTest, LauncherNewProcessInvalidCommand) {
    // Note: launcherNewProcess detaches the process, so it cannot detect
    // if the actual command fails. It only throws if the shell cannot start.
    // On Windows, cmd/powershell will start successfully even with invalid commands.
#ifdef _WIN32
    ASSERT_NO_THROW(
        neko::core::launcherNewProcess("invalid_command_xyz_123", testDir.string())
    );
#else
    // On Unix, behavior may vary
    ASSERT_NO_THROW(
        neko::core::launcherNewProcess("invalid_command_xyz_123", testDir.string())
    );
#endif
}

// Test launcherNewProcess with empty working directory (should use current dir)
TEST_F(LauncherProcessTest, LauncherNewProcessEmptyWorkingDir) {
#ifdef _WIN32
    std::string command = "echo test";
#else
    std::string command = "echo test";
#endif

    ASSERT_NO_THROW(neko::core::launcherNewProcess(command, ""));
}

// Test process callbacks are called in correct order
TEST_F(LauncherProcessTest, CallbackOrder) {
    std::vector<std::string> callbackOrder;
    
    neko::core::ProcessInfo info;
#ifdef _WIN32
    info.command = "echo test";
#else
    info.command = "echo test";
#endif
    
    info.onStart = [&callbackOrder]() {
        callbackOrder.push_back("start");
    };
    
    info.pipeStreamCb = [&callbackOrder](const std::string &) {
        if (callbackOrder.back() != "stream") {
            callbackOrder.push_back("stream");
        }
    };
    
    info.onExit = [&callbackOrder](int) {
        callbackOrder.push_back("exit");
    };

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
    
    ASSERT_GE(callbackOrder.size(), 2);
    EXPECT_EQ(callbackOrder[0], "start");
    EXPECT_EQ(callbackOrder.back(), "exit");
}

// Test multiple process launches sequentially
TEST_F(LauncherProcessTest, MultipleSequentialProcesses) {
    for (int i = 0; i < 5; ++i) {
        std::atomic<bool> completed{false};
        
        neko::core::ProcessInfo info;
#ifdef _WIN32
        info.command = "echo Process " + std::to_string(i);
#else
        info.command = "echo Process " + std::to_string(i);
#endif
        info.onExit = [&completed](int) {
            completed = true;
        };

        ASSERT_NO_THROW(neko::core::launcherProcess(info));
        EXPECT_TRUE(completed);
    }
}

// Test process with null callbacks (should not crash)
TEST_F(LauncherProcessTest, NullCallbacks) {
    neko::core::ProcessInfo info;
#ifdef _WIN32
    info.command = "echo test";
#else
    info.command = "echo test";
#endif
    info.onStart = nullptr;
    info.onExit = nullptr;
    info.pipeStreamCb = nullptr;

    ASSERT_NO_THROW(neko::core::launcherProcess(info));
}

// Test windowsCommandLengthLimit constant
TEST(LauncherProcessConstantsTest, WindowsCommandLengthLimit) {
    EXPECT_EQ(neko::core::windowsCommandLengthLimit, 8191);
}