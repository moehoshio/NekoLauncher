#include "neko/ui/windows/nekoWindow.hpp"
#include "neko/ui/animation.hpp"
#include "neko/ui/fonts.hpp"
#include "neko/ui/themeIO.hpp"
#include "neko/app/nekoLc.hpp"

#include "neko/app/app.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/bus/configBus.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include "neko/core/auth.hpp"

#include <neko/function/utilities.hpp>

#include <QtCore/QTimer>
#include <QtCore/QMetaObject>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QHoverEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QStyleHints>
#include <QtGui/QWindow>

#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <algorithm>
#include <chrono>
#include <initializer_list>

namespace neko::ui::window {

    NekoWindow::NekoWindow(const ClientConfig &config)
                : centralWidget(new QWidget(this)),
                    blurEffect(new QGraphicsBlurEffect(this)),
                    noticeDialog(new dialog::NoticeDialog(this)),
                    inputDialog(new dialog::InputDialog(this)),
                    pixmapWidget(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
                    musicWidget(new widget::MusicWidget(this)),
                    aboutPage(new page::AboutPage(this)),
                    homePage(new page::HomePage(this)),
                    loadingPage(new page::LoadingPage(this)),
                    newsPage(new page::NewsPage(this)),
                    settingPage(new page::SettingPage(this)) {

        log::info("NekoWindow ctor: start widgets");
        // Setup widget stacking order
        pixmapWidget->lower();
        centralWidget->raise();
        homePage->raise();
        aboutPage->raise();
        loadingPage->raise();
        newsPage->raise();
        settingPage->raise();
        musicWidget->raise();
        inputDialog->raise();
        noticeDialog->raise();

        homePage->hide();
        aboutPage->hide();
        loadingPage->hide();
        newsPage->hide();
        noticeDialog->hide();
        inputDialog->hide();
        settingPage->hide();
        musicWidget->hide(); // Hidden by default, shown if config enables it

        centralWidget->setObjectName("centralWidget");
        centralWidget->setAttribute(Qt::WA_StyledBackground, true);

        log::info("NekoWindow ctor: setup theme");
        setupTheme(ui::getCurrentTheme());
        log::info("NekoWindow ctor: setup text");
        setupText();
        pixmapWidget->setGraphicsEffect(blurEffect);

        this->setMinimumSize(800, 420);
        // Remove hard max size so native maximize stays available
        this->setCentralWidget(centralWidget);
        this->setWindowTitle(lc::AppName.data());
        this->setWindowIcon(QIcon(lc::AppIconPath.data()));
        // Use the native window frame; no custom head bar.

        log::info("NekoWindow ctor: switch to loading page");
        switchToPage(Page::loading);
        log::info("NekoWindow ctor: settingFromConfig start");
        settingFromConfig(config);
        log::info("NekoWindow ctor: setup connections");
        setupConnections();
        if (auto *hints = QGuiApplication::styleHints()) {
            connect(hints, &QStyleHints::colorSchemeChanged, this, [this]() {
                applySystemThemeIfNeeded();
            });
        }
        windowSizeApplyTimer = new QTimer(this);
        windowSizeApplyTimer->setSingleShot(true);
        connect(windowSizeApplyTimer, &QTimer::timeout, this, &NekoWindow::applyPendingWindowSize);
        log::info("NekoWindow ctor: resizeItems start");
        resizeItems(this->width(), this->height());
        log::info("NekoWindow ctor: showLoading start");
        showLoading({
            .type = ui::LoadingMsg::Type::OnlyRaw,
            .process = lang::tr(lang::keys::loading::category, lang::keys::loading::starting, "Starting...")
        });
        log::info("NekoWindow ctor: showLangLoadWarningIfNeeded");
        showLangLoadWarningIfNeeded();
        log::info("NekoWindow ctor: done");
    }
    NekoWindow::~NekoWindow() {
    }

    void NekoWindow::setupTheme(const Theme &theme) {
        homePage->setupTheme(theme);
        aboutPage->setupTheme(theme);
        loadingPage->setupTheme(theme);
        newsPage->setupTheme(theme);
        settingPage->setupTheme(theme);
        noticeDialog->setupTheme(theme);
        inputDialog->setupTheme(theme);
        musicWidget->setupTheme(theme);
        applyCentralBackground(theme);
    }

    void NekoWindow::applySystemThemeIfNeeded() {
        if (!followSystemTheme) {
            return;
        }
        applyThemeSelection("system");
    }

    void NekoWindow::applyThemeSelection(const std::string &themeName) {
        std::string lower = themeName;
        for (auto &c : lower) {
            c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
        }
        followSystemTheme = (lower == "system");

        auto themeOpt = ui::themeio::loadThemeByName(themeName, lc::ThemeFolderName.data());
        if (!themeOpt.has_value()) {
            themeOpt = ui::lightTheme;
        }

        this->setUpdatesEnabled(false);
        ui::setCurrentTheme(*themeOpt);
        setupTheme(ui::getCurrentTheme());
        this->setUpdatesEnabled(true);
        this->update();
    }

    void NekoWindow::setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font) {
        homePage->setupFont(textFont, h1Font, h2Font);
        aboutPage->setupFont(textFont, h1Font, h2Font);
        loadingPage->setupFont(textFont, h1Font, h2Font);
        newsPage->setupFont(textFont, h1Font, h2Font);
        settingPage->setupFont(textFont, h1Font, h2Font);
        noticeDialog->setupFont(textFont, h2Font);
        inputDialog->setupFont(textFont, h2Font);
    }

    void NekoWindow::setupText() {
        homePage->setupText();
        aboutPage->setupText();
        newsPage->setupText();
        settingPage->setupText();
        // Add other widgets/pages here when they expose setupText.
    }

    void NekoWindow::applyCentralBackground(const Theme &theme) {
        if (useImageBackground) {
            centralWidget->setStyleSheet(" #centralWidget { background-color: transparent; }");
            centralWidget->setAutoFillBackground(false);
        } else {
            const auto backdrop = !theme.colors.canvas.empty() ? theme.colors.canvas : theme.colors.background;
            centralWidget->setStyleSheet(QString(" #centralWidget { background-color: %1; }")
                                             .arg(backdrop.data()));
            centralWidget->setAutoFillBackground(true);
        }
    }

    void NekoWindow::showLangLoadWarningIfNeeded() {
        const std::string err = lang::lastLoadError();
        if (err.empty()) {
            return;
        }
        log::warn("Language load warning: {}", {}, err);
        NoticeMsg notice;
        notice.title = "Language missing";
        notice.message = err + "\n" + std::string("Falling back to default text.");
        notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
        notice.callback = [](neko::uint32) {};
        showNotice(notice);
    }

    void NekoWindow::showNotice(const NoticeMsg &m) {
        noticeDialog->showNotice(m);
    }

    void NekoWindow::showInput(const InputMsg &m) {
        inputDialog->showInput(m);
    }

    void NekoWindow::hideInput() {
        inputDialog->hideInput();
    }

    std::vector<std::string> NekoWindow::getLines() {
        return inputDialog->getLines();
    }

    void NekoWindow::showLoading(const LoadingMsg &m) {
        switchToPage(Page::loading);
        emit showLoadingD(m);
    }

    void NekoWindow::setNews(std::vector<api::NewsItem> items, bool hasMore) {
        Q_UNUSED(hasMore);
        newsPage->setNews(items);
    }

    void NekoWindow::handleNewsLoadFailed(const std::string &reason) {
        log::warn("News load failed: {}", {}, reason);
        // If news loading fails, skip to home page directly
        switchToPage(Page::home);
    }

    void NekoWindow::setupConnections() {
        connect(this, &NekoWindow::showNoticeD, noticeDialog, &dialog::NoticeDialog::showNotice);
        connect(this, &NekoWindow::showInputD, inputDialog, &dialog::InputDialog::showInput);
        connect(this, &NekoWindow::showLoadingD, loadingPage, &page::LoadingPage::showLoading);
        connect(this, &NekoWindow::hideInputD, inputDialog, &dialog::InputDialog::hideInput);
        connect(this, &NekoWindow::getLinesD, inputDialog, &dialog::InputDialog::getLines);
        connect(this, &NekoWindow::resetNoticeStateD, noticeDialog, &dialog::NoticeDialog::resetState);
        connect(this, &NekoWindow::resetNoticeButtonsD, noticeDialog, &dialog::NoticeDialog::resetButtons);
        connect(this, &NekoWindow::switchToPageD, this, &NekoWindow::switchToPage);
        connect(this, &NekoWindow::setLoadingValueD, loadingPage, &page::LoadingPage::setLoadingValue);
        connect(this, &NekoWindow::setLoadingStatusD, loadingPage, &page::LoadingPage::setLoadingStatus);
        connect(this, &NekoWindow::refreshTextD, this, &NekoWindow::setupText);
        connect(this, &NekoWindow::hideWindowD, this, &QWidget::hide);
        connect(this, &NekoWindow::showWindowD, this, [this]() {
            this->show();
            this->raise();
            this->activateWindow();
        });

        // News page connections
        connect(newsPage, &page::NewsPage::continueClicked, this, [this]() {
            // If preview mode, just return to setting page without saving
            if (newsPreviewMode) {
                newsPreviewMode = false;
                switchToPage(Page::setting);
                return;
            }

            // Save dismiss option before switching
            auto dismissOption = newsPage->getDismissOption();
            bus::config::updateClientConfig([dismissOption](ClientConfig &cfg) {
                switch (dismissOption) {
                    case page::NewsDismissOption::Days3: {
                        auto now = std::chrono::system_clock::now();
                        auto future = now + std::chrono::hours(72);
                        cfg.other.newsDismissUntil = std::chrono::duration_cast<std::chrono::seconds>(
                            future.time_since_epoch()).count();
                        cfg.other.newsDismissVersion = "";
                        break;
                    }
                    case page::NewsDismissOption::Days7: {
                        auto now = std::chrono::system_clock::now();
                        auto future = now + std::chrono::hours(168);
                        cfg.other.newsDismissUntil = std::chrono::duration_cast<std::chrono::seconds>(
                            future.time_since_epoch()).count();
                        cfg.other.newsDismissVersion = "";
                        break;
                    }
                    case page::NewsDismissOption::UntilUpdate:
                        cfg.other.newsDismissUntil = 0;
                        cfg.other.newsDismissVersion = std::string(app::getVersion());
                        break;
                    default:
                        cfg.other.newsDismissUntil = 0;
                        cfg.other.newsDismissVersion = "";
                        break;
                }
            });
            bus::config::save(app::getConfigFileName());
            switchToPage(Page::home);
        });
        connect(this, &NekoWindow::quitAppD, this, [this]() {
            this->close();
            app::quit();
        });

        connect(homePage, &page::HomePage::startButtonClicked, this, [this]() {
            if (!core::auth::isLoggedIn()) {
                onLoginRequested();
                return;
            }

            bus::event::publish<event::LaunchRequestEvent>({});
        });
        connect(homePage, &page::HomePage::menuButtonClicked, this, [this]() {
            switchToPage(Page::setting);
        });
        connect(homePage, &page::HomePage::versionButtonClicked, this, [this]() {
            switchToPage(Page::about);
        });

        connect(aboutPage, &page::AboutPage::backRequested, this, [this]() {
            switchToPage(Page::home);
        });

        connect(settingPage, &page::SettingPage::closeRequested, this, [this]() {
            // Publish event so network error recovery can intercept if needed
            bus::event::publish(event::NetworkSettingsClosedEvent{});
            // The event handler will switch to appropriate page
            // If not in recovery mode, this will be handled and go to home
        });

        // Live setting updates
        connect(settingPage, &page::SettingPage::languageChanged, this, &NekoWindow::onLanguageChanged);
        connect(settingPage, &page::SettingPage::themeChanged, this, &NekoWindow::onThemeChanged);
        connect(settingPage, &page::SettingPage::fontPointSizeChanged, this, &NekoWindow::onFontPointSizeChanged);
        connect(settingPage, &page::SettingPage::fontFamiliesChanged, this, &NekoWindow::onFontFamiliesChanged);
        connect(settingPage, &page::SettingPage::blurEffectChanged, this, &NekoWindow::onBlurEffectChanged);
        connect(settingPage, &page::SettingPage::blurRadiusChanged, this, &NekoWindow::onBlurRadiusChanged);
        connect(settingPage, &page::SettingPage::backgroundTypeChanged, this, &NekoWindow::onBackgroundTypeChanged);
        connect(settingPage, &page::SettingPage::backgroundPathChanged, this, &NekoWindow::onBackgroundPathChanged);
        connect(settingPage, &page::SettingPage::windowSizeEdited, this, &NekoWindow::onWindowSizeEdited);
        connect(settingPage, &page::SettingPage::windowSizeApplyRequested, this, &NekoWindow::onWindowSizeApplyRequested);
        connect(settingPage, &page::SettingPage::configChanged, this, &NekoWindow::onConfigChanged);
        connect(settingPage, &page::SettingPage::loginRequested, this, &NekoWindow::onLoginRequested);
        connect(settingPage, &page::SettingPage::logoutRequested, this, &NekoWindow::onLogoutRequested);
        connect(settingPage, &page::SettingPage::showNoticePreviewRequested, this, [this]() {
            NoticeMsg notice;
            notice.title = "Notice Preview";
            notice.message = "This is a preview of the notice dialog.";
            notice.buttonText = {"OK", "Cancel"};
            notice.callback = [](neko::uint32) {};
            showNotice(notice);
        });
        connect(settingPage, &page::SettingPage::showInputPreviewRequested, this, [this]() {
            InputMsg msg;
            msg.title = "Input Preview";
            msg.message = "Enter demo values to preview the input dialog.";
            msg.lineText = {"First field", "Second field"};
            msg.callback = [this](bool confirmed) {
                const auto lines = getLines();
                hideInput();
                NoticeMsg resultNotice;
                resultNotice.title = "Input Preview";
                if (!confirmed) {
                    resultNotice.message = "Preview cancelled.";
                } else {
                    std::string summary;
                    for (const auto &line : lines) {
                        if (!summary.empty()) {
                            summary += " | ";
                        }
                        summary += line;
                    }
                    resultNotice.message = summary.empty() ? "No input provided." : ("Captured: " + summary);
                }
                resultNotice.buttonText = {"OK"};
                showNotice(resultNotice);
            };
            showInput(msg);
        });
        connect(settingPage, &page::SettingPage::showLoadingPreviewRequested, this, [this]() {
            LoadingMsg msg;
            msg.type = LoadingMsg::Type::All;
            msg.h1 = "Loading Preview";
            msg.h2 = "Demo task";
            msg.message = "Showing loading page for preview.";
            msg.process = "Preparing UI preview...";
            msg.progressMax = 100;
            msg.progressVal = 0;
            showLoading(msg);

            auto *timer = new QTimer(this);
            timer->setInterval(250);
            int progress = 0;
            connect(timer, &QTimer::timeout, this, [this, timer, progress]() mutable {
                progress = std::min(progress + 10, 100);
                emit setLoadingValueD(progress);
                emit setLoadingStatusD(progress >= 100 ? std::string("Done.") : std::string("Working..."));
                if (progress >= 100) {
                    timer->stop();
                    timer->deleteLater();
                    switchToPage(Page::setting);
                }
            });
            timer->start();
        });
        connect(settingPage, &page::SettingPage::showNewsPreviewRequested, this, [this]() {
            newsPreviewMode = true;
            std::vector<api::NewsItem> demoItems{
                api::NewsItem{
                    .id = "preview-001",
                    .title = "Preview: Maintenance window",
                    .summary = "Planned downtime for backend upgrades",
                    .content = "Maintenance is scheduled to improve stability and add new features.",
                    .posterUrl = "",
                    .link = "https://example.com/news/maintenance",
                    .publishTime = "2024-06-01T12:00:00Z",
                    .category = "maintenance",
                    .tags = {"windows", "linux"},
                    .priority = 10},
                api::NewsItem{
                    .id = "preview-002",
                    .title = "Preview: New launcher experience",
                    .summary = "UI refresh and better update flow",
                    .content = "The latest launcher improves update reliability and adds clearer progress messaging.",
                    .posterUrl = "",
                    .link = "https://example.com/news/launcher",
                    .publishTime = "2024-06-02T08:30:00Z",
                    .category = "general",
                    .tags = {"release"},
                    .priority = 5}
            };
            newsPage->setNews(demoItems);
            switchToPage(Page::news);
        });
    }

    void NekoWindow::persistConfigFromUi(bool saveToFile) {
        bus::config::updateClientConfig([this](ClientConfig &cfg) {
            settingPage->writeToConfig(cfg);
        });
        if (saveToFile) {
            bus::config::save(app::getConfigFileName());
        }
    }

    void NekoWindow::onThemeChanged(const QString &themeName) {
        applyThemeSelection(themeName.toStdString());
    }

    void NekoWindow::onFontPointSizeChanged(int pointSize) {
        QFont textFont = this->font();
        if (pointSize > 0) {
            textFont.setPointSize(pointSize);
        } else {
            textFont.setPointSize(10);
        }
        auto [h1Font, h2Font] = ui::computeTitleFonts(textFont);
        setupFont(textFont, h1Font, h2Font);
    }

    void NekoWindow::onFontFamiliesChanged(const QString &families) {
        QFont textFont = this->font();
        textFont.setFamily(families);
        auto [h1Font, h2Font] = ui::computeTitleFonts(textFont);
        setupFont(textFont, h1Font, h2Font);
    }

    void NekoWindow::onBlurEffectChanged(const QString &effect) {
        if (effect == "performance") {
            blurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
        } else if (effect == "quality") {
            blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
        } else {
            blurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
        }
    }

    void NekoWindow::onBlurRadiusChanged(int radius) {
        if (radius == 1) {
            return; // skip problematic radius
        }
        blurEffect->setBlurRadius(static_cast<qreal>(std::max(0, radius)));
    }

    void NekoWindow::onLanguageChanged(const QString &langCode) {
        if (!langCode.isEmpty()) {
            lang::language(langCode.toStdString());
        }
        setupText();
    }

    void NekoWindow::onLoginRequested() {
        InputMsg msg;
        msg.title = lang::tr(lang::keys::setting::category, lang::keys::setting::login, "Login");
        msg.message = "Enter account and password";
        msg.lineText = {
            lang::tr(lang::keys::input::category, lang::keys::input::placeholder, "Account"),
            lang::tr(lang::keys::input::category, lang::keys::input::password, "Password")};
        msg.callback = [this](bool confirmed) {
            if (!confirmed) {
                hideInput();
                return;
            }

            const auto lines = getLines();
            hideInput();
            if (lines.size() < 2 || lines[0].empty() || lines[1].empty()) {
                NoticeMsg notice;
                notice.title = lang::tr(lang::keys::setting::category, lang::keys::setting::login, "Login");
                notice.message = lang::tr(lang::keys::error::category, lang::keys::error::invalidInput, "Invalid input");
                notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                showNotice(notice);
                settingPage->setAuthState(false, {});
                return;
            }

            const auto result = core::auth::authLogin(lines);
            if (!result.error.empty()) {
                NoticeMsg notice;
                notice.title = lang::tr(lang::keys::setting::category, lang::keys::setting::login, "Login");
                notice.message = result.error;
                notice.buttonText = {lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK")};
                showNotice(notice);
                settingPage->setAuthState(false, result.error);
                return;
            }

            settingPage->setAuthState(true, result.name);
        };

        showInput(msg);
    }

    void NekoWindow::onLogoutRequested() {
        NoticeMsg notice;
        notice.title = lang::tr(lang::keys::setting::category, lang::keys::setting::logout, "Logout");
        notice.message = "Logout current account?";
        notice.buttonText = {
            lang::tr(lang::keys::button::category, lang::keys::button::ok, "OK"),
            lang::tr(lang::keys::button::category, lang::keys::button::cancel, "Cancel")};
        notice.callback = [this](neko::uint32 idx) {
            if (idx != 0) {
                return;
            }
            minecraft::auth::authLogout();
            settingPage->setAuthState(false, {});
        };

        showNotice(notice);
    }

    void NekoWindow::onBackgroundTypeChanged(const QString &type) {
        useImageBackground = (type == "image");
        if (useImageBackground) {
            pixmapWidget->setPixmap(settingPage->getBackgroundPath());
        } else {
            pixmapWidget->clearPixmap();
        }
        applyCentralBackground(ui::getCurrentTheme());
    }

    void NekoWindow::onBackgroundPathChanged(const QString &path) {
        if (useImageBackground) {
            pixmapWidget->setPixmap(path.toStdString());
        }
    }

    void NekoWindow::onWindowSizeEdited(const QString &sizeText) {
        pendingWindowSizeText = sizeText;
        if (windowSizeApplyTimer) {
            windowSizeApplyTimer->start(1000);
        }
    }

    void NekoWindow::onWindowSizeApplyRequested(const QString &sizeText) {
        pendingWindowSizeText = sizeText;
        if (windowSizeApplyTimer && windowSizeApplyTimer->isActive()) {
            windowSizeApplyTimer->stop();
        }
        applyWindowSizeText(sizeText, true);
    }

    void NekoWindow::onConfigChanged() {
        const bool wantsImmediateSave = settingPage->isImmediateSaveEnabled();
        const bool immediateFlagChanged = (wantsImmediateSave != saveImmediately);
        saveImmediately = wantsImmediateSave;
        const bool saveNow = saveImmediately || immediateFlagChanged;
        persistConfigFromUi(saveNow);
    }

    void NekoWindow::applyPendingWindowSize() {
        if (pendingWindowSizeText.isEmpty()) {
            return;
        }

        applyWindowSizeText(pendingWindowSizeText, true);
    }

    void NekoWindow::applyWindowSizeText(const QString &sizeText, bool save) {
        auto resolution = util::check::matchResolution(sizeText.toStdString());
        if (!resolution.has_value()) {
            return;
        }

        int width = std::stoi(resolution->width);
        int height = std::stoi(resolution->height);
        width = std::max(width, this->minimumWidth());
        height = std::max(height, this->minimumHeight());

        this->resize(width, height);
        settingPage->setWindowSizeDisplay(QStringLiteral("%1x%2").arg(width).arg(height));
        if (save) {
            persistConfigFromUi(saveImmediately);
        }
    }

    void NekoWindow::settingFromConfig(const ClientConfig &config) {

        // Main

        useImageBackground = (neko::strview("image") == config.main.backgroundType);
        if (useImageBackground) {
            pixmapWidget->setPixmap(config.main.background);
        } else {
            pixmapWidget->clearPixmap();
        }
        applyCentralBackground(ui::getCurrentTheme());

        auto resolution = util::check::matchResolution(config.main.windowSize);
        if (resolution) {
            this->resize(std::stoi(resolution->width), std::stoi(resolution->height));
        }

        saveImmediately = config.other.immediateSave;

        // Style
        QFont textFont(QString::fromStdString(config.style.fontFamilies));
        if (config.style.fontPointSize > 0) {
            textFont.setPointSize(static_cast<int>(config.style.fontPointSize));
        } else {
            textFont.setPointSize(10);
        }

        auto [h1Font, h2Font] = ui::computeTitleFonts(textFont);
        setupFont(textFont, h1Font, h2Font);
        settingPage->settingFromConfig(config);
        setupText();

        {
            std::string lower = config.style.theme;
            for (auto &c : lower) {
                c = static_cast<char>(::tolower(static_cast<unsigned char>(c)));
            }
            followSystemTheme = (lower == "system");
            applySystemThemeIfNeeded();
        }

        const bool logged = minecraft::auth::isLoggedIn();
        const auto playerName = minecraft::auth::getPlayerName();
        settingPage->setAuthState(logged, logged ? playerName : std::string{});

        if (config.style.blurRadius > 0 && config.style.blurRadius != 1) {
            blurEffect->setBlurRadius(static_cast<qreal>(config.style.blurRadius));
        } else {
            blurEffect->setBlurRadius(0);
        }

        if (config.style.blurEffect == "performance") {
            blurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
        } else if (config.style.blurEffect == "quality") {
            blurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
        } else {
            blurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
        }

        // Show/hide music widget based on dev settings
        if (config.dev.showMusicControl) {
            musicWidget->show();
        } else {
            musicWidget->hide();
        }
    }

    void NekoWindow::switchToPage(Page page) {
        log::info("switchToPage {}", {}, static_cast<int>(page));
        
        if (currentPage == page) return;
        
        // Determine the target page widget
        QWidget *targetPage = nullptr;
        switch (page) {
            case Page::home:
                targetPage = homePage;
                break;
            case Page::about:
                targetPage = aboutPage;
                break;
            case Page::loading:
                targetPage = loadingPage;
                break;
            case Page::news:
                targetPage = newsPage;
                break;
            case Page::setting:
                targetPage = settingPage;
                break;
            default:
                break;
        }

        if (!targetPage) return;

        // Determine animation direction based on page order
        auto pageIndex = [](Page p) -> int {
            switch (p) {
                case Page::loading: return 0;
                case Page::news: return 1;
                case Page::home: return 2;
                case Page::setting: return 3;
                case Page::about: return 4;
                default: return -1;
            }
        };

        int oldIdx = pageIndex(currentPage);
        int newIdx = pageIndex(page);
        anim::Direction direction = (oldIdx < newIdx) ? anim::Direction::Right : anim::Direction::Left;

        // FIRST: Immediately hide ALL other pages to prevent any visual artifacts
        for (auto *p : std::initializer_list<QWidget*>{homePage, aboutPage, loadingPage, newsPage, settingPage}) {
            if (p != targetPage) {
                p->hide();
            }
        }

        currentPage = page;
        resizeItems(this->width(), this->height());

        // iOS-style slide in animation
        anim::slideIn(targetPage, direction, anim::Duration::Slow);
    }

    void NekoWindow::resizeItems(int width, int height) {
        log::info("resizeItems {} {}", {}, width, height);
        pixmapWidget->resize(width, height);
        centralWidget->setGeometry(0, 0, width, height);
        const int contentX = 0;
        const int contentY = 0;
        const int contentWidth = width;
        const int contentHeight = height;
        noticeDialog->resizeItems(width, height, 0);
        inputDialog->resizeItems(width, height);
        homePage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        homePage->resizeItems(contentWidth, contentHeight);

        aboutPage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        aboutPage->resizeItems(contentWidth, contentHeight);

        loadingPage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        loadingPage->resizeItems(contentWidth, contentHeight);

        newsPage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        newsPage->resizeItems(contentWidth, contentHeight);

        settingPage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        settingPage->resizeItems(contentWidth, contentHeight);

        // Position music widget at bottom-right corner
        const int musicMargin = 10;
        const int musicWidth = musicWidget->width();
        const int musicHeight = musicWidget->height();
        musicWidget->move(width - musicWidth - musicMargin, height - musicHeight - musicMargin);
    }

    void NekoWindow::resizeEvent(QResizeEvent *event) {
        QMainWindow::resizeEvent(event);
        resizeItems(this->width(), this->height());
        settingPage->setWindowSizeDisplay(QStringLiteral("%1x%2").arg(this->width()).arg(this->height()));
    }

    void NekoWindow::closeEvent(QCloseEvent *event) {
        bus::config::updateClientConfig([this](ClientConfig &cfg) {
            settingPage->writeToConfig(cfg);
        });
        bus::config::save(app::getConfigFileName());
        QMainWindow::closeEvent(event);
        app::quit();
    }

    bool NekoWindow::event(QEvent *event) {
        return QMainWindow::event(event);
    }
} // namespace neko::ui::window