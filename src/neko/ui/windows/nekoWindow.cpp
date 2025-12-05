#include "neko/ui/windows/nekoWindow.hpp"
#include "neko/ui/fonts.hpp"

#include "neko/app/app.hpp"
#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/bus/configBus.hpp"

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
          loadingPage(new page::LoadingPage(this)) {

        // Setup widget stacking order
        pixmapWidget->lower();
        centralWidget->raise();
        homePage->raise();
        loadingPage->raise();
        inputDialog->raise();
        noticeDialog->raise();

        homePage->hide();
        loadingPage->hide();
        noticeDialog->hide();
        inputDialog->hide();

        centralWidget->setObjectName("centralWidget");
        centralWidget->setAttribute(Qt::WA_StyledBackground, true);

        // Setup themes
        setupTheme(ui::getCurrentTheme());
        pixmapWidget->setGraphicsEffect(blurEffect);

        this->setMinimumSize(800, 420);
        // Remove hard max size so native maximize stays available
        this->setCentralWidget(centralWidget);
        this->setWindowTitle(lc::AppName.data());
        this->setWindowIcon(QIcon(lc::AppIconPath.data()));
        this->addToolBar(headBarWidget->getToolBar());

        switchToPage(Page::home);
        inputDialog->showInput({.title = "Test Input Dialog",
                                .message = "This is a test of the input dialog. Please enter some text below:",
                                .posterPath = "",
                                .lineText = {"1", "2", "3"},
                                .callback = [this](bool confirmed) {
                                    log::info("Input dialog confirmed: {}", {}, confirmed);
                                    if (!confirmed) {
                                        emit this->hideInputD();
                                        return;
                                    }
                                }});
        // loadingPage->showLoading(
        //     {.type = LoadingMsg::Type::All,
        //      .process = lang::tr(lang::keys::update::category, lang::keys::update::checkingForUpdates),
        //      .h1 = "Welcome to NekoLauncher",
        //      .h2 = "Starting up...",
        //      .message = "Preparing the launcher...",
        //      .posterPath = "img/loading_bg.png",
        //      .loadingIconPath = "img/loading.gif",
        //      .speed = 100,
        //      .progressVal = 0,
        //      .progressMax = 100});

        settingFromConfig(config);
        setupConnections();
        resizeItems(this->width(), this->height());
    }
    NekoWindow::~NekoWindow() = default;

    void NekoWindow::setupTheme(const Theme &theme) {
        homePage->setupTheme(theme);
        loadingPage->setupTheme(theme);
        noticeDialog->setupTheme(theme);
        inputDialog->setupTheme(theme);
        applyCentralBackground(ui::homeTheme);
    }

    void NekoWindow::setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font) {
        homePage->setupFont(textFont, h1Font, h2Font);
        loadingPage->setupFont(textFont, h1Font, h2Font);
        noticeDialog->setupFont(textFont, h2Font);
        inputDialog->setupFont(textFont, h2Font);
    }

    void NekoWindow::applyCentralBackground(const Theme &theme) {
        if (useImageBackground) {
            centralWidget->setStyleSheet(" #centralWidget { background-color: transparent; }");
            centralWidget->setAutoFillBackground(false);
        } else {
            centralWidget->setStyleSheet(QString(" #centralWidget { background-color: %1; }")
                                             .arg(theme.colors.background.data()));
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
    }

    void NekoWindow::settingFromConfig(const ClientConfig &config) {

        QFont textFont(config.style.fontFamilies);
        if (config.style.fontPointSize > 0) {
            textFont.setPointSize(static_cast<int>(config.style.fontPointSize));
        } else {
            textFont.setPointSize(10);
        }

        auto [h1Font, h2Font] = ui::computeTitleFonts(textFont);
        setupFont(textFont, h1Font, h2Font);

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

        useImageBackground = (neko::strview("image") == config.main.backgroundType);
        if (useImageBackground) {
            pixmapWidget->setPixmap(config.main.background);
        } else {
            pixmapWidget->clearPixmap();
        }
        applyCentralBackground(ui::homeTheme);

        if (config.style.blurRadius > 0) {
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
        }
    }

    void NekoWindow::resizeEvent(QResizeEvent *event) {
        QMainWindow::resizeEvent(event);
        resizeItems(this->width(), this->height());
    }

    void NekoWindow::closeEvent(QCloseEvent *event) {
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