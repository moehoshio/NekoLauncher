#include "neko/ui/windows/nekoWindow.hpp"
#include "neko/ui/fonts.hpp"

#include "neko/app/app.hpp"
#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/bus/configBus.hpp"
#include "neko/minecraft/authMinecraft.hpp"

#include <neko/function/utilities.hpp>

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
          headBarWidget(new widget::HeadBarWidget(this, this)),
          pixmapWidget(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
          homePage(new page::HomePage(this)),
          loadingPage(new page::LoadingPage(this)),
          settingPage(new page::SettingPage(this))  {

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
        this->addToolBar(headBarWidget->getToolBar());

        switchToPage(Page::home);
        settingFromConfig(config);
        setupConnections();
        resizeItems(this->width(), this->height());
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
        headBarWidget->setupText();
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
        headBarWidget->raise();
        headBarWidget->getToolBar()->raise();
    }

    void NekoWindow::showInput(const InputMsg &m) {
        inputDialog->showInput(m);
        headBarWidget->raise();
        headBarWidget->getToolBar()->raise();
    }

    void NekoWindow::hideInput() {
        inputDialog->hideInput();
    }

    std::vector<std::string> NekoWindow::getLines() {
        return inputDialog->getLines();
    }

    void NekoWindow::showLoading(const LoadingMsg &m) {
        loadingPage->showLoading(m);
    }

    void NekoWindow::setupConnections() {
        connect(this, &NekoWindow::showNoticeD, noticeDialog, &dialog::NoticeDialog::showNotice);
        connect(this, &NekoWindow::showInputD, inputDialog, &dialog::InputDialog::showInput);
        connect(this, &NekoWindow::hideInputD, inputDialog, &dialog::InputDialog::hideInput);
        connect(this, &NekoWindow::getLinesD, inputDialog, &dialog::InputDialog::getLines);
        connect(this, &NekoWindow::resetNoticeStateD, noticeDialog, &dialog::NoticeDialog::resetState);
        connect(this, &NekoWindow::resetNoticeButtonsD, noticeDialog, &dialog::NoticeDialog::resetButtons);
        connect(this, &NekoWindow::switchToPageD, this, &NekoWindow::switchToPage);
        connect(this, &NekoWindow::setLoadingValueD, loadingPage, &page::LoadingPage::setLoadingValue);
        connect(this, &NekoWindow::setLoadingStatusD, loadingPage, &page::LoadingPage::setLoadingStatus);

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
    }

    void NekoWindow::onThemeChanged(const QString &themeName) {
        // Basic mapping; extend as needed for custom themes.
        const auto name = themeName.toLower();
        if (name == "dark") {
            ui::setCurrentTheme(ui::darkTheme);
        } else if (name == "home") {
            ui::setCurrentTheme(ui::homeTheme);
        } else {
            ui::setCurrentTheme(ui::lightTheme);
        }
        setupTheme(ui::getCurrentTheme());
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

            const auto result = minecraft::auth::authLogin(lines);
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

        if (config.main.useSysWindowFrame) {
            headBarWidget->hideHeadBar();
        } else {
            headBarWidget->showHeadBar();
        }
        if (config.main.headBarKeepRight) {
            headBarWidget->setHeadBarAlignmentRight(true);
        } else {
            headBarWidget->setHeadBarAlignmentRight(false);
        }

        auto resolution = util::check::matchResolution(config.main.windowSize);
        if (resolution){
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
        if (currentPage == page) {
            return;
        }

        switch (currentPage) {
            case Page::home:
                homePage->hide();
                break;
            case Page::loading:
                loadingPage->hide();
                break;
            case Page::setting:
                settingPage->hide();
                break;
            default:
                break;
        }

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
        centralWidget->resize(width, height);
        const int headBarHeight = headBarWidget->isVisible() ? headBarWidget->height() : 0;
        noticeDialog->resizeItems(width, height, headBarHeight);
        inputDialog->resizeItems(width, height);
        headBarWidget->raise();
        headBarWidget->getToolBar()->raise();
        if (this->currentPage == Page::home) {
            homePage->resizeItems(width, height);
        } else if (this->currentPage == Page::loading) {
            loadingPage->resizeItems(width, height);
        } else if (this->currentPage == Page::setting) {
            settingPage->resizeItems(width, height);
        }
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
        constexpr qreal border = 11;
        switch (event->type()) {
            case QEvent::HoverMove: {
                auto p = dynamic_cast<QHoverEvent *>(event)->pos();
                bool pointXGreaterWidthBorder = (p.x() > width() - border);
                bool pointXLessWidthBorder = (p.x() < border);
                bool pointYGreaterHeightToolBar = (p.y() > headBarWidget->height());
                bool pointYGreaterHeightBorder = (p.y() > height() - border);
                if (pointYGreaterHeightBorder) {
                    if (pointXLessWidthBorder) {
                        this->setCursor(Qt::SizeBDiagCursor);
                    } else if (pointXGreaterWidthBorder) {
                        this->setCursor(Qt::SizeFDiagCursor);
                    } else {
                        this->setCursor(Qt::SizeVerCursor);
                    }
                } else if (pointYGreaterHeightToolBar && (pointXGreaterWidthBorder || pointXLessWidthBorder)) {
                    this->setCursor(Qt::SizeHorCursor);
                } else {
                    setCursor(Qt::ArrowCursor);
                }
                break;
            }
            case QEvent::MouseButtonPress: {
                auto currentFocus = focusWidget();
                if (currentFocus) {
                    currentFocus->clearFocus();
                }
                auto p = dynamic_cast<QMouseEvent *>(event)->pos();
                bool pointXGreaterWidthBorder = (p.x() > width() - border);
                bool pointXLessWidthBorder = (p.x() < border);
                bool pointYGreaterHeightBorder = (p.y() > height() - border);
                Qt::Edges edges;
                if (pointXGreaterWidthBorder)
                    edges |= Qt::RightEdge;
                if (pointXLessWidthBorder)
                    edges |= Qt::LeftEdge;
                if (pointYGreaterHeightBorder)
                    edges |= Qt::BottomEdge;
                if (edges != 0) {
                    this->windowHandle()->startSystemResize(edges);
                }
                break;
            }
            default:
                break;
        }

        return QMainWindow::event(event);
    }
} // namespace neko::ui::window