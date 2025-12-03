#pragma once

#include "neko/ui/theme.hpp"
#include "neko/ui/page.hpp"

#include "neko/ui/widgets/headBarWidget.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"
#include "neko/ui/pages/homePage.hpp"

#include "neko/app/clientConfig.hpp"

#include <QtWidgets/QMainWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

class QWidget;
class QGraphicsBlurEffect;

namespace neko::ui::window {

    class NekoWindow : public QMainWindow {
    private:
        Page currentPage;
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        QGraphicsBlurEffect *blurEffect;

        // Widgets
        widget::HeadBarWidget *headBar;
        widget::PixmapWidget *pixmapWidget;

        QWidget *centralWidget;

        // Pages
        page::HomePage *homePage;

    public:
        NekoWindow(const ClientConfig &config);
        ~NekoWindow();
        void settingFromConfig(const ClientConfig &config);
        void switchToPage(Page page);
        void resizeItems(int width, int height);
    protected:
        void resizeEvent(QResizeEvent *event) override;
        void closeEvent(QCloseEvent *event) override;
        bool event(QEvent *event) override;
    };

} // namespace neko::ui::window
