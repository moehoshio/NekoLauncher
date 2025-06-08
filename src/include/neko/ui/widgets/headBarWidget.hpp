#pragma once

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

    struct HeadBarWidget : public QWidget {
        ToolBarWidget *toolbar;
        QAction *close_;
        QAction *minimize;
        QAction *maximize;
        QAction *sp1;
        QAction *sp2;
        QWidget *spacer;
        HeadBarWidget(QWidget *parent = nullptr);
    };
} // namespace neko::ui
