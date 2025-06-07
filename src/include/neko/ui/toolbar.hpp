#include <QtWidgets/QWidget>
#include <QtWidgets/QToolBar>

namespace neko::ui {
    class ToolBar : public QToolBar {
        Q_OBJECT
    public:
        ToolBar(QWidget *parent = nullptr) : QToolBar(parent) {
            setMovable(false);
            setFloatable(false);

            setAcceptDrops(true);

            setContextMenuPolicy(Qt::PreventContextMenu);
        }
        inline bool event(QEvent *event) override {
            if (event->type() == QEvent::MouseButtonPress)
                emit requestMoveWindow();

            return QToolBar::event(event);
        }
        inline void dragEnterEvent(QDragEnterEvent *event) override {
            event->acceptProposedAction();
        }

        inline void dragMoveEvent(QDragMoveEvent *p_event) override {
            p_event->acceptProposedAction();
        }
    signals:
        // Signal to request moving the window when the toolbar is clicked
        // This is used in MainWindow
        void requestMoveWindow();
    };
} // namespace neko::ui
