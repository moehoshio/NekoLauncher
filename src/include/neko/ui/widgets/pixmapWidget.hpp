#pragma once

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QPainter>
#include <QtWidgets/QWidget>

#include <filesystem>
#include <string_view>

namespace neko::ui {

    class PixmapWidget : public QWidget {
        QPixmap bg;
        Qt::AspectRatioMode aspectMode;

    public:
        PixmapWidget(Qt::AspectRatioMode aspect = Qt::KeepAspectRatioByExpanding, QWidget *parent = nullptr)
            : QWidget(parent), aspectMode(aspect) {}
        PixmapWidget(const QPixmap &pixmap, Qt::AspectRatioMode aspect = Qt::KeepAspectRatioByExpanding, QWidget *parent = nullptr)
            : QWidget(parent), aspectMode(aspect) {
            setPixmap(pixmap);
        }

        void setAspectMode(Qt::AspectRatioMode mode) {
            if (aspectMode != mode) {
                aspectMode = mode;
                update();
            }
        }

        bool setPixmap(const QPixmap &pix) {
            if (pix.isNull())
                return false;
            bg = pix;
            update();
            return true;
        }

        bool setPixmap(std::string_view fileName) {
            if (fileName.empty() || !std::filesystem::exists(fileName))
                return false;
            bool ok = bg.load(QString::fromUtf8(fileName.data(), fileName.size()));
            if (ok)
                update();
            return ok;
        }

        void clearPixmap() {
            if (!bg.isNull()) {
                bg = QPixmap();
                update();
            }
        }

    protected:
        void paintEvent(QPaintEvent *event) override {
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
    };
} // namespace neko::ui
