#include "neko/ui/widgets/headBarWidget.hpp"

#include "neko/app/app.hpp"
#include "neko/app/lang.hpp"

#include <QtWidgets/QApplication>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

#include <QtGui/QAction>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QWindow>

#include <QtCore/QEvent>

namespace neko::ui::widget {

    // ===================
    // == ToolBarWidget ==
    // ===================
    
    ToolBarWidget::ToolBarWidget(QWidget *parent) : QToolBar(parent) {
        setMovable(false);
        setFloatable(false);
        setAcceptDrops(true);
        setContextMenuPolicy(Qt::PreventContextMenu);
    }

    bool ToolBarWidget::event(QEvent *event) {
        if (event->type() == QEvent::MouseButtonPress)
            emit requestMoveWindow();
        return QToolBar::event(event);
    }
    void ToolBarWidget::dragEnterEvent(QDragEnterEvent *event) {
        event->acceptProposedAction();
    }
    void ToolBarWidget::dragMoveEvent(QDragMoveEvent *event) {
        event->acceptProposedAction();
    }

    // ===================
    // == HeadBarWidget ==
    // ===================

    HeadBarWidget::HeadBarWidget(QWidget *topWin, QWidget *parent)
        : QWidget(parent),
          toolbar(new ToolBarWidget(this)),
          spacer(new QWidget(toolbar)),
          closeAction(new QAction(toolbar)),
          minimize(new QAction(toolbar)),
          maximize(new QAction(toolbar)),
          topWindow(topWin) {

        sp1 = toolbar->addSeparator();
        sp2 = toolbar->addSeparator();

        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        setupText();
        setupTheme(ui::getCurrentTheme());
        setHeadBarAlignmentRight();
        
        connect(this->toolbar, &ToolBarWidget::requestMoveWindow,
                [=, this] {
                    topWindow->windowHandle()->startSystemMove();
                });
        connect(this->closeAction, &QAction::triggered,
                [=, this]() {
                    QApplication::quit();
                    app::quit();
                });

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

    ToolBarWidget * HeadBarWidget::getToolBar() {
        return toolbar;
    }

    void HeadBarWidget::setupTheme(const Theme &theme) {
        QString styleSheet =
            QString(
                "QToolBar {"
                "background-color: %1;"
                "border: none;"
                "border-radius: 0px;"
                "}"
                "QToolButton {"
                "background-color: transparent;"
                "border: none;"
                "color: %2;"
                "}"
                "QToolButton:hover {"
                "background-color: %3;"
                "}")
                .arg(theme.colors.background.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.hover.data());

        toolbar->setStyleSheet(styleSheet);
    }

    void HeadBarWidget::setupText() {
        closeAction->setText(QString::fromStdString(lang::tr(lang::keys::button::category, lang::keys::button::close,"Close")) );
        maximize->setText(QString::fromStdString(lang::tr(lang::keys::button::category, lang::keys::button::maximize,"Maximize")) );
        minimize->setText(QString::fromStdString(lang::tr(lang::keys::button::category, lang::keys::button::minimize,"Minimize")) );
    }

    void HeadBarWidget::showHeadBar() {
        this->show();
        toolbar->show();
        toolbar->raise();
        topWindow->setWindowFlags(
            topWindow->windowFlags() & Qt::Window | Qt::FramelessWindowHint);
    }

    void HeadBarWidget::hideHeadBar() {
        this->hide();
        toolbar->hide();
        topWindow->setWindowFlags(
            topWindow->windowFlags() & Qt::Window);
    }

    void HeadBarWidget::setHeadBarAlignmentRight(bool keepRight) {
        toolbar->clear();
        if (keepRight) {
            toolbar->addWidget(spacer);
            toolbar->addActions({minimize, sp1, maximize, sp2, closeAction});
        } else {
            toolbar->addActions({closeAction, sp1, maximize, sp2, minimize});
        }
    }
} // namespace neko::ui::widget