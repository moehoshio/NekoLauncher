#pragma once
#include "neko/ui/theme.hpp"

#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include <QtWidgets/QAction>

#include <QtWidgets/QDragEnterEvent>
#include <QtWidgets/QDragMoveEvent>

#include <QtCore/QEvent>

namespace neko::ui {

    class ToolBarWidget : public QToolBar {
        Q_OBJECT
    public:
        explicit ToolBarWidget(QWidget *parent = nullptr) : QToolBar(parent) {
            setMovable(false);
            setFloatable(false);
            setAcceptDrops(true);
            setContextMenuPolicy(Qt::PreventContextMenu);
        }

        bool event(QEvent *event) override {
            if (event->type() == QEvent::MouseButtonPress)
                emit requestMoveWindow();
            return QToolBar::event(event);
        }

        void dragEnterEvent(QDragEnterEvent *event) override {
            event->acceptProposedAction();
        }

        void dragMoveEvent(QDragMoveEvent *event) override {
            event->acceptProposedAction();
        }

    signals:
        void requestMoveWindow();
    };

    class HeadBarWidget : public QWidget {
    private:
        ToolBarWidget *toolbar;
        QAction *closeAction;
        QAction *minimize;
        QAction *maximize;
        QAction *sp1;
        QAction *sp2;
        QWidget *spacer;
    public:
        // Ensure the correct parent object; the parent must be a top-level window!
        HeadBarWidget(QWidget *parent);

        void setupStyle(const Theme& theme);
        void setupText();
        // Ensure the correct parent object; it sets the parent's window flags to show and hide the system window frame.
        void showHeadBar();
        void hideHeadBar();
        void setHeadBarAlignmentRight(bool keepRight = false);
    };
} // namespace neko::ui
