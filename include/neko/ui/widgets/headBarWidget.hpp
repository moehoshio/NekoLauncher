#pragma once
#include "neko/ui/theme.hpp"

#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

class QAction;
class QDragEnterEvent;
class QDragMoveEvent;
class QEvent;

namespace neko::ui::widget {

    class ToolBarWidget : public QToolBar {
        Q_OBJECT
    public:
        explicit ToolBarWidget(QWidget *parent = nullptr);

        bool event(QEvent *event) override;
        void dragEnterEvent(QDragEnterEvent *event) override;
        void dragMoveEvent(QDragMoveEvent *event) override;

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
        QWidget *topWindow;
    public:
        HeadBarWidget(QWidget *topWin, QWidget *parent = nullptr);

        ToolBarWidget * getToolBar();
        void setupTheme(const Theme& theme);
        void setupText();
        void showHeadBar();
        void hideHeadBar();
        void setHeadBarAlignmentRight(bool keepRight = false);
    };
} // namespace neko::ui
