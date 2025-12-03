
#include "neko/app/lang.hpp"
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

using namespace neko::lang;

class LangTest : public ::testing::Test {
protected:
    std::string testLangDir;
    std::string originalLang;

    void SetUp() override {
        // Save original language setting
        originalLang = language();
        
        // Create temporary test language directory
        testLangDir = (std::filesystem::temp_directory_path() / "test_lang").string();
        std::filesystem::create_directories(testLangDir);
        
        // Create test English language file
        createTestLanguageFile("en", {
            {"language", "English"},
            {"test", {
                {"testKey", "Test Value"},
                {"greeting", "Hello"},
                {"farewell", "Goodbye"},
                {"placeholder", "Hello {name}, welcome to {place}!"}
            }},
            {"maintenance", {
                {"title", "Maintenance"},
                {"message", "System is under maintenance"}
            }}
        });
        
        // Create test Chinese language file
        createTestLanguageFile("zh_tw", {
            {"language", "Traditional Chinese"},
            {"test", {
                {"testKey", "Test Value ZH"},
                {"greeting", "Hello ZH"},
                {"farewell", "Goodbye ZH"},
                {"placeholder", "Hello {name}, welcome to {place} ZH!"}
            }},
            {"maintenance", {
                {"title", "Maintenance ZH"},
                {"message", "System maintenance ZH"}
            }}
        });
    }

    void TearDown() override {
        // Restore original language setting
        language(originalLang);
        
        // Clean up test files
        if (std::filesystem::exists(testLangDir)) {
            std::filesystem::remove_all(testLangDir);
        }
    }

    void createTestLanguageFile(const std::string& lang, const nlohmann::json& content) {
        std::string filePath = testLangDir + "/" + lang + ".json";
        std::ofstream file(filePath);
        file << content.dump(4);
        file.close();
    }
};

// Test language getter and setter
TEST_F(LangTest, LanguageGetterSetter) {
    // Test default language
    EXPECT_EQ(language(), originalLang);
    
    // Test setting new language
    language("zh_tw");
    EXPECT_EQ(language(), "zh_tw");
    
    // Test setting again
    language("en");
    EXPECT_EQ(language(), "en");
}

// Test getting language folder path
TEST_F(LangTest, GetLanguageFolder) {
    std::string langFolder = getLanguageFolder();
    EXPECT_FALSE(langFolder.empty());
    EXPECT_TRUE(langFolder.find("lang") != std::string::npos);
}

// Test getting available languages list
TEST_F(LangTest, GetLanguages) {
    auto languages = getLanguages(testLangDir);
    
    EXPECT_EQ(languages.size(), 2);
    
    // Check if en and zh_tw are in the list (as pairs)
    bool foundEn = false, foundZhTw = false;
    for (const auto& [code, name] : languages) {
        if (code == "en" && name == "English") foundEn = true;
        if (code == "zh_tw" && name == "Traditional Chinese") foundZhTw = true;
    }
    EXPECT_TRUE(foundEn);
    EXPECT_TRUE(foundZhTw);
}

// Test empty directory language list
TEST_F(LangTest, GetLanguagesEmptyDirectory) {
    std::string emptyDir = (std::filesystem::temp_directory_path() / "empty_lang").string();
    std::filesystem::create_directories(emptyDir);
    
    auto languages = getLanguages(emptyDir);
    EXPECT_TRUE(languages.empty());
    
    std::filesystem::remove_all(emptyDir);
}

// Test loading translation files
TEST_F(LangTest, LoadTranslations) {
    auto translations = loadTranslations("en", testLangDir);
    
    EXPECT_FALSE(translations.empty());
    EXPECT_FALSE(translations.is_discarded());
    EXPECT_EQ(translations["language"], "English");
    EXPECT_TRUE(translations.contains("test"));
    EXPECT_EQ(translations["test"]["testKey"], "Test Value");
}

// Test loading non-existent language file
TEST_F(LangTest, LoadTranslationsNonExistent) {
    auto translations = loadTranslations("fr", testLangDir);
    
    // Should return empty JSON object
    EXPECT_TRUE(translations.empty() || translations.is_object());
}

// Test translation caching mechanism
TEST_F(LangTest, LoadTranslationsCaching) {
    // First load of zh_tw (not cached yet)
    auto trans1 = loadTranslations("zh_tw", testLangDir);
    EXPECT_FALSE(trans1.empty());
    EXPECT_TRUE(trans1.contains("language"));
    EXPECT_EQ(trans1["language"], "Traditional Chinese");
    
    // Second load of same language from same folder (should use cache)
    auto trans2 = loadTranslations("zh_tw", testLangDir);
    EXPECT_EQ(trans1, trans2);
    
    // Load different language (should update cache)
    auto trans3 = loadTranslations("en", testLangDir);
    EXPECT_FALSE(trans3.empty());
    EXPECT_EQ(trans3["language"], "English");
    EXPECT_NE(trans1["language"], trans3["language"]);
}

// Test translation key lookup
TEST_F(LangTest, TranslationLookup) {
    auto translations = loadTranslations("en", testLangDir);
    
    std::string result = tr("test", "testKey", "Not found", translations);
    EXPECT_EQ(result, "Test Value");
    
    result = tr("test", "greeting", "Not found", translations);
    EXPECT_EQ(result, "Hello");
}

// Test translation fallback when key doesn't exist
TEST_F(LangTest, TranslationFallback) {
    auto translations = loadTranslations("en", testLangDir);
    
    std::string result = tr("test", "nonExistentKey", "Fallback message", translations);
    EXPECT_EQ(result, "Fallback message");
    
    result = tr("nonExistentCategory", "anyKey", "Fallback message", translations);
    EXPECT_EQ(result, "Fallback message");
}

// Test empty translation file
TEST_F(LangTest, TranslationEmptyFile) {
    nlohmann::json emptyJson = nlohmann::json::object();
    
    std::string result = tr("anyCategory", "anyKey", "Default", emptyJson);
    EXPECT_EQ(result, "Default");
}

// Test placeholder replacement
TEST_F(LangTest, PlaceholderReplacement) {
    std::string input = "Hello {name}, welcome to {place}!";
    std::map<std::string, std::string> replacements = {
        {"{name}", "Alice"},
        {"{place}", "Wonderland"}
    };
    
    std::string result = withPlaceholdersReplaced(input, replacements);
    EXPECT_EQ(result, "Hello Alice, welcome to Wonderland!");
}

// Test multiple same placeholders replacement
TEST_F(LangTest, MultipleSamePlaceholders) {
    std::string input = "{name} loves {name}";
    std::map<std::string, std::string> replacements = {
        {"{name}", "Bob"}
    };
    
    std::string result = withPlaceholdersReplaced(input, replacements);
    EXPECT_EQ(result, "Bob loves Bob");
}

// Test string without placeholders
TEST_F(LangTest, NoPlaceholders) {
    std::string input = "Plain text without placeholders";
    std::map<std::string, std::string> replacements = {
        {"{name}", "Alice"}
    };
    
    std::string result = withPlaceholdersReplaced(input, replacements);
    EXPECT_EQ(result, input);
}

// Test empty string replacement
TEST_F(LangTest, EmptyStringReplacement) {
    std::string input = "";
    std::map<std::string, std::string> replacements = {
        {"{name}", "Alice"}
    };
    
    std::string result = withPlaceholdersReplaced(input, replacements);
    EXPECT_EQ(result, "");
}

// Test trWithReplaced function
TEST_F(LangTest, TrWithReplaced) {
    // Create a separate directory to avoid cache conflicts
    std::string separateTestDir = (std::filesystem::temp_directory_path() / "test_lang_tr").string();
    std::filesystem::create_directories(separateTestDir);
    
    // Create test file
    nlohmann::json testJson = {
        {"language", "English"},
        {"test", {
            {"placeholder", "Hello {name}, welcome to {place}!"}
        }}
    };
    std::string filePath = separateTestDir + "/en.json";
    std::ofstream file(filePath);
    file << testJson.dump(4);
    file.close();
    
    // Load translations from separate directory
    auto translations = loadTranslations("en", separateTestDir);
    
    std::map<std::string, std::string> replacements = {
        {"{name}", "Charlie"},
        {"{place}", "TestLand"}
    };
    
    // Get the translation with the loaded translations
    std::string placeholderText = tr("test", "placeholder", "Not found", translations);
    EXPECT_NE(placeholderText, "Not found");
    
    // Apply placeholders
    std::string result = withPlaceholdersReplaced(placeholderText, replacements);
    EXPECT_EQ(result, "Hello Charlie, welcome to TestLand!");
    
    // Cleanup
    std::filesystem::remove_all(separateTestDir);
}

// Test language constant keys
TEST_F(LangTest, LanguageKeys) {
    // Test language key
    EXPECT_STREQ(keys::language, "language");
    
    // Test maintenance namespace
    EXPECT_STREQ(keys::maintenance::category, "maintenance");
    EXPECT_STREQ(keys::maintenance::title, "title");
    EXPECT_STREQ(keys::maintenance::message, "message");
    EXPECT_STREQ(keys::maintenance::checkingStatus, "checkingStatus");
    EXPECT_STREQ(keys::maintenance::parseIng, "parseIng");
    EXPECT_STREQ(keys::maintenance::downloadPoster, "downloadPoster");
}

// Test complete translation workflow
TEST_F(LangTest, CompleteTranslationWorkflow) {
    // Set language
    language("zh_tw");
    
    // Load translations
    auto translations = loadTranslations("zh_tw", testLangDir);
    
    // Get translation
    std::string greeting = tr("test", "greeting", "Not found", translations);
    EXPECT_EQ(greeting, "Hello ZH");
    
    // Test maintenance keys
    std::string maintenanceTitle = tr("maintenance", "title", "Not found", translations);
    EXPECT_EQ(maintenanceTitle, "Maintenance ZH");
    
    // Use placeholders
    std::string placeholderText = tr("test", "placeholder", "Not found", translations);
    std::string result = withPlaceholdersReplaced(placeholderText, {
        {"{name}", "TestName"},
        {"{place}", "TestPlace"}
    });
    EXPECT_EQ(result, "Hello TestName, welcome to TestPlace ZH!");
}
