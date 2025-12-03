#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <filesystem>

namespace neko::ui::widget {

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

    bool PixmapWidget::setPixmap(const QPixmap &pixmap) {
        if (pixmap.isNull()){
            return false;
        }
        bg = pixmap;
        update();
        return true;
    }

    bool PixmapWidget::setPixmap(neko::strview fileName) {
        if (fileName.empty() || !std::filesystem::exists(fileName))
            return false;
        bool isOk = bg.load(fileName.data());
        if (isOk){
            update();
        }
        return isOk;
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
        QWidget::paintEvent(event);
    }

} // namespace neko::ui::widget
