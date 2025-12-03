#include "neko/ui/windows/nekoWindow.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/app/app.hpp"
#include "neko/bus/configBus.hpp"

#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

#include <QtGui/QHoverEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWindow>

namespace neko::ui::window {

    NekoWindow::NekoWindow(const ClientConfig &config)
        : headBar(new widget::HeadBarWidget(this, this)),
          pixmapWidget(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
          homePage(new page::HomePage(this)),
          centralWidget(new QWidget(this)) {

        if (config.main.useSysWindowFrame) {
            headBar->hideHeadBar();
        } else {
            headBar->showHeadBar();
        }

        // Setup widget stacking order
        pixmapWidget->lower();
        centralWidget->raise();
        homePage->raise();

        // Setup themes
        homePage->setupTheme(ui::homeTheme);

        pixmapWidget->setPixmap(config.main.background);


        this->setMinimumSize(800, 420);
        this->setMaximumSize(scrSize);
        this->setCentralWidget(centralWidget);
        this->setWindowTitle(lc::AppName.data());
        this->setWindowIcon(QIcon(lc::AppIconPath.data()));
        this->addToolBar(headBar->getToolBar());

        switchToPage(Page::home);

        resizeItems(this->width(), this->height());
    }
    NekoWindow::~NekoWindow() = default;

    void NekoWindow::switchToPage(Page page) {
        if (currentPage == page) {
            return;
        }

        switch (currentPage) {
            case Page::home:
                homePage->hide();
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
            default:
                break;
        }
    }

    void NekoWindow::resizeItems(int width, int height) {
        pixmapWidget->resize(width, height);
        centralWidget->resize(width, height);
        homePage->resizeItems(width, height);
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
                bool pointYGreaterHeightToolBar = (p.y() > headBar->height());
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