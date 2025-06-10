#include "neko/ui/widgets/pixmapWidget.hpp"

namespace neko::ui {

    PixmapWidget::PixmapWidget(Qt::AspectRatioMode aspect, QWidget *parent)
        : QWidget(parent), aspectMode(aspect) {}

    PixmapWidget::PixmapWidget(const QPixmap &pixmap, Qt::AspectRatioMode aspect, QWidget *parent)
        : QWidget(parent), aspectMode(aspect) {
        setPixmap(pixmap);
    }

    void PixmapWidget::setAspectMode(Qt::AspectRatioMode mode) {
        if (aspectMode != mode) {
            aspectMode = mode;
            update();
        }
    }

    bool PixmapWidget::setPixmap(const QPixmap &pix) {
        if (pix.isNull())
            return false;
        bg = pix;
        update();
        return true;
    }

    bool PixmapWidget::setPixmap(std::string_view fileName) {
        if (fileName.empty() || !std::filesystem::exists(fileName))
            return false;
        bool ok = bg.load(QString::fromUtf8(fileName.data(), fileName.size()));
        if (ok)
            update();
        return ok;
    }

    void PixmapWidget::clearPixmap() {
        if (!bg.isNull()) {
            bg = QPixmap();
            update();
        }
    }

    void PixmapWidget::paintEvent(QPaintEvent *event) {
        QPainter painter(this);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        if (!bg.isNull()) {
            QSize widgetSize = size();
            QSize pixmapSize = bg.size();
            QSize scaledSize = pixmapSize.scaled(widgetSize, aspectMode);

            // Center the pixmap
            QPoint topLeft((widgetSize.width() - scaledSize.width()) / 2,
                           (widgetSize.height() - scaledSize.height()) / 2);
            QRect targetRect(topLeft, scaledSize);

            painter.drawPixmap(targetRect, bg);
        }
        // QWidget::paintEvent(event);
    }

} // namespace neko::ui
