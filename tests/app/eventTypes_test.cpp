#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "neko/app/eventTypes.hpp"
#include <gtest/gtest.h>

using namespace neko::event;

class EventTypesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

// Test Global Events
TEST_F(EventTypesTest, StartEventCreation) {
    StartEvent event;
    // StartEvent is an empty struct, just verify it can be instantiated
    SUCCEED();
}

TEST_F(EventTypesTest, QuitEventCreation) {
    QuitEvent event;
    // QuitEvent is an empty struct, just verify it can be instantiated
    SUCCEED();
}

// Test Window Events
TEST_F(EventTypesTest, ChangeWindowTitleEvent) {
    ChangeWindowTitleEvent event;
    event.title = "New Window Title";
    EXPECT_EQ(event.title, "New Window Title");
}

TEST_F(EventTypesTest, ChangeWindowIconEvent) {
    ChangeWindowIconEvent event;
    event.iconPath = "path/to/icon.png";
    EXPECT_EQ(event.iconPath, "path/to/icon.png");
}

TEST_F(EventTypesTest, CloseWindowEvent) {
    CloseWindowEvent event;
    SUCCEED();
}

TEST_F(EventTypesTest, MinimizeWindowEvent) {
    MinimizeWindowEvent event;
    SUCCEED();
}

TEST_F(EventTypesTest, MaximizeWindowEvent) {
    MaximizeWindowEvent event;
    SUCCEED();
}

TEST_F(EventTypesTest, RestoreWindowEvent) {
    RestoreWindowEvent event;
    SUCCEED();
}

TEST_F(EventTypesTest, FocusWindowEvent) {
    FocusWindowEvent event;
    SUCCEED();
}

TEST_F(EventTypesTest, ResizeWindowEvent) {
    ResizeWindowEvent event;
    event.width = 1920;
    event.height = 1080;
    
    EXPECT_EQ(event.width, 1920);
    EXPECT_EQ(event.height, 1080);
}

TEST_F(EventTypesTest, MoveWindowEvent) {
    MoveWindowEvent event;
    event.x = 100;
    event.y = 200;
    
    EXPECT_EQ(event.x, 100);
    EXPECT_EQ(event.y, 200);
}

TEST_F(EventTypesTest, ChangeCurrentPageEvent) {
    ChangeCurrentPageEvent event;
    event.page = neko::ui::Page::home;
    EXPECT_EQ(event.page, neko::ui::Page::home);
    
    event.page = neko::ui::Page::setting;
    EXPECT_EQ(event.page, neko::ui::Page::setting);
    
    event.page = neko::ui::Page::loading;
    EXPECT_EQ(event.page, neko::ui::Page::loading);
}

TEST_F(EventTypesTest, ShowHintEvent) {
    neko::ui::HintMsg msg;
    msg.title = "Test Title";
    msg.message = "Test Message";
    msg.poster = "path/to/poster.png";
    msg.buttonText = {"OK", "Cancel"};
    msg.autoClose = 5000;
    msg.defaultButtonIndex = 0;
    
    ShowHintEvent event(msg);
    EXPECT_EQ(event.title, "Test Title");
    EXPECT_EQ(event.message, "Test Message");
    EXPECT_EQ(event.poster, "path/to/poster.png");
    EXPECT_EQ(event.buttonText.size(), 2);
    EXPECT_EQ(event.buttonText[0], "OK");
    EXPECT_EQ(event.buttonText[1], "Cancel");
    EXPECT_EQ(event.autoClose, 5000);
    EXPECT_EQ(event.defaultButtonIndex, 0);
}

TEST_F(EventTypesTest, ShowLoadEvent) {
    neko::ui::LoadMsg msg;
    msg.type = neko::ui::LoadMsg::Type::Progress;
    msg.process = "Loading data...";
    msg.h1 = "Main Title";
    msg.h2 = "Subtitle";
    msg.message = "Please wait";
    msg.poster = "path/to/poster.png";
    msg.icon = "img/loading.gif";
    msg.speed = 150;
    msg.progressVal = 50;
    msg.progressMax = 100;
    
    ShowLoadEvent event(msg);
    EXPECT_EQ(event.type, neko::ui::LoadMsg::Type::Progress);
    EXPECT_EQ(event.process, "Loading data...");
    EXPECT_EQ(event.h1, "Main Title");
    EXPECT_EQ(event.h2, "Subtitle");
    EXPECT_EQ(event.message, "Please wait");
    EXPECT_EQ(event.poster, "path/to/poster.png");
    EXPECT_EQ(event.icon, "img/loading.gif");
    EXPECT_EQ(event.speed, 150);
    EXPECT_EQ(event.progressVal, 50);
    EXPECT_EQ(event.progressMax, 100);
}

TEST_F(EventTypesTest, ShowInputEvent) {
    neko::ui::InputMsg msg;
    msg.title = "Input Title";
    msg.message = "Enter your data";
    msg.poster = "path/to/poster.png";
    msg.lineText = {"Line 1", "Line 2", "Line 3"};
    
    ShowInputEvent event(msg);
    EXPECT_EQ(event.title, "Input Title");
    EXPECT_EQ(event.message, "Enter your data");
    EXPECT_EQ(event.poster, "path/to/poster.png");
    EXPECT_EQ(event.lineText.size(), 3);
    EXPECT_EQ(event.lineText[0], "Line 1");
    EXPECT_EQ(event.lineText[1], "Line 2");
    EXPECT_EQ(event.lineText[2], "Line 3");
}

TEST_F(EventTypesTest, UpdateLoadingValEvent) {
    UpdateLoadingValEvent event;
    event.progressVal = 75;
    EXPECT_EQ(event.progressVal, 75);
}

TEST_F(EventTypesTest, UpdateLoadingNowEvent) {
    UpdateLoadingNowEvent event;
    event.process = "Processing files...";
    EXPECT_EQ(event.process, "Processing files...");
}

TEST_F(EventTypesTest, UpdateLoadingEvent) {
    UpdateLoadingEvent event("Downloading...", 60);
    EXPECT_EQ(event.process, "Downloading...");
    EXPECT_EQ(event.progressVal, 60);
}

// Test Core Events
TEST_F(EventTypesTest, MaintenanceEvent) {
    neko::ui::HintMsg msg;
    msg.title = "Maintenance";
    msg.message = "Server is under maintenance";
    
    MaintenanceEvent event(msg);
    EXPECT_EQ(event.title, "Maintenance");
    EXPECT_EQ(event.message, "Server is under maintenance");
}

TEST_F(EventTypesTest, UpdateAvailableEvent) {
    UpdateAvailableEvent event;
    // UpdateAvailableEvent inherits from api::UpdateResponse
    // We can't test much without knowing the structure of UpdateResponse
    // Just verify it can be instantiated
    SUCCEED();
}

TEST_F(EventTypesTest, UpdateCompleteEvent) {
    UpdateCompleteEvent event;
    SUCCEED();
}

// Test event type sizes (ensure they are lightweight)
TEST_F(EventTypesTest, EventSizes) {
    // Empty events should be very small
    EXPECT_LE(sizeof(StartEvent), 8);
    EXPECT_LE(sizeof(QuitEvent), 8);
    EXPECT_LE(sizeof(CloseWindowEvent), 8);
    EXPECT_LE(sizeof(MinimizeWindowEvent), 8);
    EXPECT_LE(sizeof(MaximizeWindowEvent), 8);
    EXPECT_LE(sizeof(RestoreWindowEvent), 8);
    EXPECT_LE(sizeof(FocusWindowEvent), 8);
    
    // Events with data should have reasonable sizes
    EXPECT_GT(sizeof(ChangeWindowTitleEvent), 0);
    EXPECT_GT(sizeof(ResizeWindowEvent), 0);
    EXPECT_GT(sizeof(ShowHintEvent), 0);
    EXPECT_GT(sizeof(ShowLoadEvent), 0);
}

// Test multiple event creation
TEST_F(EventTypesTest, MultipleEventsCreation) {
    std::vector<ResizeWindowEvent> resizeEvents;
    for (int i = 0; i < 10; ++i) {
        ResizeWindowEvent event;
        event.width = 800 + i * 100;
        event.height = 600 + i * 75;
        resizeEvents.push_back(event);
    }
    
    EXPECT_EQ(resizeEvents.size(), 10);
    EXPECT_EQ(resizeEvents[0].width, 800);
    EXPECT_EQ(resizeEvents[9].width, 1700);
    EXPECT_EQ(resizeEvents[0].height, 600);
    EXPECT_EQ(resizeEvents[9].height, 1275);
}

// Test event copy semantics
TEST_F(EventTypesTest, EventCopySemantics) {
    ChangeWindowTitleEvent original;
    original.title = "Original Title";
    
    ChangeWindowTitleEvent copy = original;
    EXPECT_EQ(copy.title, "Original Title");
    
    copy.title = "Modified Title";
    EXPECT_EQ(original.title, "Original Title");
    EXPECT_EQ(copy.title, "Modified Title");
}

// Test event move semantics
TEST_F(EventTypesTest, EventMoveSemantics) {
    ChangeWindowTitleEvent original;
    original.title = "Original Title";
    
    ChangeWindowTitleEvent moved = std::move(original);
    EXPECT_EQ(moved.title, "Original Title");
}
