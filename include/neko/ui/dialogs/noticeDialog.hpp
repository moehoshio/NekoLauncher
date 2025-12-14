#pragma once

#include "neko/ui/uiMsg.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtWidgets/QWidget>

#include <deque>
#include <vector>

class QFrame;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;
class QEvent;
class QEvent;

namespace neko::ui::dialog {

    class NoticeDialog : public QWidget {
        Q_OBJECT
    private:
        QWidget *centralWidget;
        widget::PixmapWidget *poster;
        QVBoxLayout *centralWidgetLayout;
        QFrame *line;
        std::vector<QPushButton *> buttons;
        QLabel *title;
        QLabel *msg;
        QWidget *buttonContainer;
        QHBoxLayout *buttonLayout;
        std::deque<NoticeMsg> noticeQueue;
        bool showing = false;

        void presentNextNotice();
        void finishCurrent();

    public:
        NoticeDialog(QWidget *parent = nullptr);

        void showNotice(const NoticeMsg &m);

        void setupFont(QFont font,QFont titleFont);
        void setupTheme(const Theme &theme);

        void resetState();
        void resetButtons();

        void resizeItems(int windowWidth, int windowHeight, int topPadding = 0);
    signals:
        void showNoticeD(const NoticeMsg &m);
        void resetStateD();
        void resetButtonsD();
    };
} // namespace neko::ui::dialog
