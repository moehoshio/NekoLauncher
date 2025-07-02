#include "neko/ui/widgets/headbarWidget.hpp"
#include "neko/ui/theme.hpp"

#include <QtGui/QWindow>

namespace neko::ui {
    HeadBarWidget::HeadBarWidget(QWidget *topWin, QWidget *parent)
        : QWidget(parent),
          toolbar(this),
          spacer(toolbar),
          closeAction(toolbar),
          minimize(toolbar),
          maximize(toolbar),
          topWindow(topWin) {
        sp1 = toolbar->addSeparator();
        sp2 = toolbar->addSeparator();

        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        setHeadBarAlignmentRight();

        connect(this->toolbar, &ToolBar::requestMoveWindow,
                [=, this] {
                    topWindow->windowHandle()->startSystemMove();
                });
        connect(this->closeAction, &QAction::triggered,
                [=, this]() { QApplication::quit(); });

        connect(this->maximize, &QAction::triggered,
                [=, this] {
                    if (topWindow->windowState() == Qt::WindowMaximized)
                        topWindow->setWindowState(Qt::WindowNoState);
                    else
                        topWindow->setWindowState(Qt::WindowMaximized);
                });
        connect(this->minimize, &QAction::triggered,
                [=, this]() { topWindow->setWindowState(Qt::WindowMinimized); });
    }

    HeadBarWidget::setupStyle(const Theme &theme) {
        toolbar->setStyleSheet(
            QString("QToolBar { background-color: %1; border-radius: %2; }"
                    "QToolButton { background-color: %3; border: none; }"
                    "QToolButton:hover { background-color: %4; }")
                .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size()))
                .arg(QString::fromUtf8(theme.borderRadius.data(), theme.borderRadius.size()))
                .arg(QString::fromUtf8(theme.accentColor.data(), theme.accentColor.size()))
                .arg(QString::fromUtf8(theme.hoverColor.data(), theme.hoverColor.size())));
        toolbar->setIconSize(QSize(theme.fontSize + 8, theme.fontSize + 8));
    }

    HeadBarWidget::setupText() {
        closeAction->setText(QString::fromStdString(neko::info::translations(neko::info::lang.general.close)));
        minimize->setText(QString::fromStdString(neko::info::translations(neko::info::lang.general.minimize)));
        maximize->setText(QString::fromStdString(neko::info::translations(neko::info::lang.general.maximize)));
    }

    HeadBarWidget::showHeadBar() {
        this->show();
        toolbar->show();
        toolbar->raise();
        topWindow->setWindowFlags(
            topWindow->windowFlags() & Qt::Window | Qt::FramelessWindowHint);
    }

    HeadBarWidget::hideHeadBar() {
        this->hide();
        toolbar->hide();
        topWindow->setWindowFlags(
            topWindow->windowFlags() & Qt::Window);
    }

    HeadBarWidget::setHeadBarAlignmentRight(bool keepRight) {
        toolbar->clear();
        if (keepRight) {
            toolbar->addWidget(spacer);
            toolbar->addActions({minimize, sp1, maximize, sp2, closeAction});
        } else {
            toolbar->addActions({closeAction, sp1, maximize, sp2, minimize});
        }
    }
} // namespace neko::ui
