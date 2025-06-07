#include <QtWidgets/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

namespace neko::ui {

    class PixmapWidget : public QWidget {
        QPixmap bg;

    public:
        PixmapWidget(QWidget *parent = nullptr) : QWidget(parent) {};
        PixmapWidget(const QPixmap &pixmap, QWidget *parent = nullptr) : QWidget(parent) {
            setPixmap(pixmap);
        }

        inline void setPixmap(const QPixmap &pix) {
            if (pix.isNull())
                return;
            bg = pix;
        }
        inline void setPixmap(const char *fileName) {
            if (std::string(fileName) == "")
                return;

            bg.load(fileName);
        }
        inline void paintEvent(QPaintEvent *event) override {
            QPainter painter(this);

            painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

            if (!bg.isNull())
                painter.drawPixmap(rect(), bg);

            QWidget::paintEvent(event);
        }
    };
} // namespace neko::ui
