
#include "neko/log/nlog.hpp"
#include <cassert>
#include <thread>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <iostream>

namespace neko::log::test {

void test_levelToString() {
    assert(std::string(levelToString(Level::Debug)) == "Debug");
    assert(std::string(levelToString(Level::Info)) == "Info");
    assert(std::string(levelToString(Level::Warn)) == "Warn");
    assert(std::string(levelToString(Level::Error)) == "Error");
    assert(std::string(levelToString(Level::Off)) == "Off");
    assert(std::string(levelToString(static_cast<Level>(99))) == "Unknown");
}

void test_ThreadNameManager() {
    ThreadNameManager::clearAllNames();
    std::string mainName = "main_thread";
    ThreadNameManager::setCurrentThreadName(mainName);
    auto id = std::this_thread::get_id();
    assert(ThreadNameManager::getThreadName(id) == mainName);

    std::thread t([]{
        ThreadNameManager::setCurrentThreadName("worker");
        auto tid = std::this_thread::get_id();
        assert(ThreadNameManager::getThreadName(tid) == "worker");
        ThreadNameManager::removeThreadName(tid);
        // After removal, should return thread id as string (not empty)
        assert(!ThreadNameManager::getThreadName(tid).empty());
    });
    t.join();

    ThreadNameManager::removeThreadName(id);
    assert(!ThreadNameManager::getThreadName(id).empty());

    ThreadNameManager::setCurrentThreadName(mainName);
    ThreadNameManager::clearAllNames();
    assert(!ThreadNameManager::getThreadName(id).empty());
}

void test_LogRecord() {
    std::string msg = "Hello";
    neko::SrcLocInfo loc("file.cpp", 42, "func");
    LogRecord rec(Level::Warn, msg, loc);
    assert(rec.level == Level::Warn);
    assert(rec.message == msg);
    assert(rec.location.getFile() == std::string("file.cpp"));
    assert(rec.location.getLine() == 42);
    assert(!rec.threadName.empty());
}

void test_DefaultFormatter() {
    LogRecord rec(Level::Info, "Test message", neko::SrcLocInfo("f.cpp", 1, "f"));
    DefaultFormatter fmt;
    std::string out = fmt.format(rec);
    assert(out.find("Test message") != std::string::npos);
    assert(out.find("Info") != std::string::npos);
    assert(out.find("f.cpp") != std::string::npos);
}

void test_ConsoleAppender() {
    // Redirect cout/cerr
    std::ostringstream out, err;
    auto* old_cout = std::cout.rdbuf(out.rdbuf());
    auto* old_cerr = std::cerr.rdbuf(err.rdbuf());

    ConsoleAppender app;
    LogRecord rec1(Level::Debug, "debug msg");
    LogRecord rec2(Level::Info, "info msg");
    LogRecord rec3(Level::Warn, "warn msg");
    LogRecord rec4(Level::Error, "error msg");
    app.append(rec1);
    app.append(rec2);
    app.append(rec3);
    app.append(rec4);
    app.flush();

    std::cout.rdbuf(old_cout);
    std::cerr.rdbuf(old_cerr);

    std::string outstr = out.str();
    std::string errstr = err.str();
    assert(outstr.find("debug msg") != std::string::npos);
    assert(outstr.find("info msg") != std::string::npos);
    assert(outstr.find("warn msg") != std::string::npos);
    assert(errstr.find("error msg") != std::string::npos);
}

void test_FileAppender() {
    std::string fname = "test_nlog_file.log";
    {
        FileAppender app(fname);
        LogRecord rec(Level::Info, "file log test", neko::SrcLocInfo("file.cpp", 10, "func"));
        app.append(rec);
        app.flush();
    }
    std::ifstream fin(fname);
    std::string line;
    bool found = false;
    while (std::getline(fin, line)) {
        if (line.find("file log test") != std::string::npos) {
            found = true;
            break;
        }
    }
    fin.close();
    std::remove(fname.c_str());
    assert(found);
}

void test_Logger_basic() {
    Logger logger(Level::Debug);
    logger.clearAppenders();
    std::ostringstream out;
    auto* old_cout = std::cout.rdbuf(out.rdbuf());
    logger.addConsoleAppender();
    logger.debug("dbg");
    logger.info("inf");
    logger.warn("wrn");
    logger.error("err");
    logger.flush();
    std::cout.rdbuf(old_cout);
    std::string s = out.str();
    assert(s.find("dbg") != std::string::npos);
    assert(s.find("inf") != std::string::npos);
    assert(s.find("wrn") != std::string::npos);
    assert(s.find("err") != std::string::npos);
}

void test_Logger_level_filter() {
    Logger logger(Level::Warn);
    logger.clearAppenders();
    std::ostringstream out;
    auto* old_cout = std::cout.rdbuf(out.rdbuf());
    logger.addConsoleAppender();
    logger.debug("should not appear");
    logger.info("should not appear");
    logger.warn("should appear");
    logger.error("should appear");
    logger.flush();
    std::cout.rdbuf(old_cout);
    std::string s = out.str();
    assert(s.find("should appear") != std::string::npos);
    assert(s.find("should not appear") == std::string::npos);
}

void test_Logger_file_appender() {
    std::string fname = "test_nlog_logger_file.log";
    Logger logger(Level::Debug);
    logger.clearAppenders();
    logger.addFileAppender(fname);
    logger.info("file logger test");
    logger.flush();
    std::ifstream fin(fname);
    std::string line;
    bool found = false;
    while (std::getline(fin, line)) {
        if (line.find("file logger test") != std::string::npos) {
            found = true;
            break;
        }
    }
    fin.close();
    std::remove(fname.c_str());
    assert(found);
}

void test_Logger_formatted() {
    Logger logger(Level::Debug);
    logger.clearAppenders();
    std::ostringstream out;
    auto* old_cout = std::cout.rdbuf(out.rdbuf());
    logger.addConsoleAppender();
    logger.info(neko::SrcLocInfo("f.cpp", 123, "f"), "Hello {} {}", "world", 42);
    logger.flush();
    std::cout.rdbuf(old_cout);
    std::string s = out.str();
    assert(s.find("Hello world 42") != std::string::npos);
    assert(s.find("f.cpp") != std::string::npos);
    assert(s.find("123") != std::string::npos);
}

void test_Logger_async() {
    Logger logger(Level::Debug);
    logger.clearAppenders();
    std::ostringstream out;
    auto* old_cout = std::cout.rdbuf(out.rdbuf());
    logger.addConsoleAppender();
    logger.setMode(neko::SyncMode::Async);

    std::thread logThread([&logger] { logger.runLoop(); });
    logger.info("async test 1");
    logger.warn("async test 2");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logger.stopLoop();
    logThread.join();
    std::cout.rdbuf(old_cout);
    std::string s = out.str();
    assert(s.find("async test 1") != std::string::npos);
    assert(s.find("async test 2") != std::string::npos);
}

void test_globalLogger_and_convenience() {
    setLevel(Level::Debug);
    clearAppenders();
    std::ostringstream out;
    auto* old_cout = std::cout.rdbuf(out.rdbuf());
    addConsoleAppender();
    debug("global dbg");
    info("global info");
    warn("global warn");
    error("global error");
    flushLog();
    std::cout.rdbuf(old_cout);
    std::string s = out.str();
    assert(s.find("global dbg") != std::string::npos);
    assert(s.find("global info") != std::string::npos);
    assert(s.find("global warn") != std::string::npos);
    assert(s.find("global error") != std::string::npos);

    // Test formatted
    std::ostringstream out2;
    std::cout.rdbuf(out2.rdbuf());
    info(neko::SrcLocInfo("g.cpp", 77, "g"), "fmt {} {}", "A", 1);
    flushLog();
    std::cout.rdbuf(old_cout);
    std::string s2 = out2.str();
    assert(s2.find("fmt A 1") != std::string::npos);
    assert(s2.find("g.cpp") != std::string::npos);
    assert(s2.find("77") != std::string::npos);
}

} // namespace neko::log::test

int main() {
    using namespace neko::log::test;
    std::cout << "[nlog] Running tests...\n";
    neko::log::setCurrentThreadName("Main Thread");
    test_levelToString();
    test_ThreadNameManager();
    test_LogRecord();
    test_DefaultFormatter();
    test_ConsoleAppender();
    test_FileAppender();
    test_Logger_basic();
    test_Logger_level_filter();
    test_Logger_file_appender();
    test_Logger_formatted();
    test_Logger_async();
    test_globalLogger_and_convenience();
    std::cout << "[nlog] All tests passed.\n";
}