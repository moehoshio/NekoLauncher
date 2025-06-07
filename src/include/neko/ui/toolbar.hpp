#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

namespace neko::ui {
    
    class ToolBar : public QToolBar {
        Q_OBJECT
    public:
        explicit ToolBar(QWidget *parent = nullptr) : QToolBar(parent) {
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
} // namespace neko::ui
