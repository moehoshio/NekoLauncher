#include "neko/ui/windows/nekoWindow.hpp"
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
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QHoverEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWindow>

#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

namespace neko::ui::window {

    NekoWindow::NekoWindow(const ClientConfig &config)
                : centralWidget(new QWidget(this)),
                    blurEffect(new QGraphicsBlurEffect(this)),
                    noticeDialog(new dialog::NoticeDialog(this)),
                    inputDialog(new dialog::InputDialog(this)),
                    pixmapWidget(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
                    homePage(new page::HomePage(this)),
                    loadingPage(new page::LoadingPage(this)),
                    settingPage(new page::SettingPage(this)) {

        // Setup widget stacking order
        pixmapWidget->lower();
        centralWidget->raise();
        homePage->raise();
        loadingPage->raise();
        settingPage->raise();
        inputDialog->raise();
        noticeDialog->raise();

        homePage->hide();
        loadingPage->hide();
        noticeDialog->hide();
        inputDialog->hide();
        settingPage->hide();

        centralWidget->setObjectName("centralWidget");
        centralWidget->setAttribute(Qt::WA_StyledBackground, true);

        // Setup themes
        setupTheme(ui::getCurrentTheme());
        setupText();
        pixmapWidget->setGraphicsEffect(blurEffect);

        this->setMinimumSize(800, 420);
        // Remove hard max size so native maximize stays available
        this->setCentralWidget(centralWidget);
        this->setWindowTitle(lc::AppName.data());
        this->setWindowIcon(QIcon(lc::AppIconPath.data()));
        // Use the native window frame; no custom head bar.

        switchToPage(Page::loading);
        settingFromConfig(config);
        setupConnections();
        resizeItems(this->width(), this->height());
        showLoading({
            .type = ui::LoadingMsg::Type::OnlyRaw,
            .process = lang::tr(lang::keys::loading::category, lang::keys::loading::starting, "Starting...")
        });
    }
    NekoWindow::~NekoWindow() = default;

    void NekoWindow::setupTheme(const Theme &theme) {
        homePage->setupTheme(theme);
        loadingPage->setupTheme(theme);
        settingPage->setupTheme(theme);
        noticeDialog->setupTheme(theme);
        inputDialog->setupTheme(theme);
        applyCentralBackground(theme);
    }

    void NekoWindow::setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font) {
        homePage->setupFont(textFont, h1Font, h2Font);
        loadingPage->setupFont(textFont, h1Font, h2Font);
        settingPage->setupFont(textFont, h1Font, h2Font);
        noticeDialog->setupFont(textFont, h2Font);
        inputDialog->setupFont(textFont, h2Font);
    }

    void NekoWindow::setupText() {
        homePage->setupText();
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

        connect(settingPage, &page::SettingPage::closeRequested, this, [this]() {
            switchToPage(Page::home);
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
    }

    void NekoWindow::onThemeChanged(const QString &themeName) {
        const auto nameStd = themeName.toStdString();
        auto themeOpt = ui::themeio::loadThemeByName(nameStd, lc::ThemeFolderName.data());
        if (!themeOpt.has_value()) {
            themeOpt = ui::lightTheme;
        }

        // Avoid mid-apply repaints for a smoother switch
        this->setUpdatesEnabled(false);
        ui::setCurrentTheme(*themeOpt);
        setupTheme(ui::getCurrentTheme());
        this->setUpdatesEnabled(true);
        this->update();
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
    }

    void NekoWindow::switchToPage(Page page) {
        // Always hide all pages first to avoid overlapping views when switching.
        homePage->hide();
        loadingPage->hide();
        settingPage->hide();

        currentPage = page;

        switch (page) {
            case Page::home:
                homePage->show();
                homePage->raise();
                break;
            case Page::loading:
                loadingPage->show();
                loadingPage->raise();
                break;
            case Page::setting:
                settingPage->show();
                settingPage->raise();
                break;
            default:
                break;
        }

        resizeItems(this->width(), this->height());
    }

    void NekoWindow::resizeItems(int width, int height) {
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

        loadingPage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        loadingPage->resizeItems(contentWidth, contentHeight);

        settingPage->setGeometry(contentX, contentY, contentWidth, contentHeight);
        settingPage->resizeItems(contentWidth, contentHeight);
    }

    void NekoWindow::resizeEvent(QResizeEvent *event) {
        QMainWindow::resizeEvent(event);
        resizeItems(this->width(), this->height());
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