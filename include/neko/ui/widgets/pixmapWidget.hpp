#pragma once

#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

#include <filesystem>
#include <string_view>

namespace neko::ui {

    class PixmapWidget : public QWidget {
        QPixmap bg;
        Qt::AspectRatioMode aspectMode;

    public:
        PixmapWidget(Qt::AspectRatioMode aspect = Qt::KeepAspectRatioByExpanding, QWidget *parent = nullptr);
        PixmapWidget(const QPixmap &pixmap, Qt::AspectRatioMode aspect = Qt::KeepAspectRatioByExpanding, QWidget *parent = nullptr);

        void setAspectMode(Qt::AspectRatioMode mode);

        bool setPixmap(const QPixmap &pix);

        bool setPixmap(std::string_view fileName);

        void clearPixmap();

    protected:
        void paintEvent(QPaintEvent *event) override;
    };
} // namespace neko::ui
