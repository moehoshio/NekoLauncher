#pragma once

#include <neko/schema/types.hpp>

#include <QtWidgets/QWidget>

class QPaintEvent;
class QPixmap;

namespace neko::ui::widget {

    class PixmapWidget : public QWidget {
        QPixmap bg;
        Qt::AspectRatioMode aspectMode;

    public:
        PixmapWidget(Qt::AspectRatioMode aspect = Qt::KeepAspectRatioByExpanding, QWidget *parent = nullptr);
        PixmapWidget(const QPixmap &pixmap, Qt::AspectRatioMode aspect = Qt::KeepAspectRatioByExpanding, QWidget *parent = nullptr);

        void setAspectMode(Qt::AspectRatioMode mode);

        bool setPixmap(const QPixmap &pixmap);
        bool setPixmap(neko::strview fileName);

        void clearPixmap();

    protected:
        void paintEvent(QPaintEvent *event) override;
    };
} // namespace neko::ui::widget
