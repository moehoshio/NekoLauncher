#pragma once

#include "neko/ui/theme.hpp"
#include "neko/ui/page.hpp"

#include "neko/ui/dialogs/noticeDialog.hpp"
#include "neko/ui/dialogs/inputDialog.hpp"

#include "neko/ui/widgets/headBarWidget.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/ui/pages/homePage.hpp"
#include "neko/ui/pages/loadingPage.hpp"

#include "neko/app/clientConfig.hpp"

#include <QtWidgets/QMainWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>

class QWidget;
class QGraphicsBlurEffect;

namespace neko::ui::window {

    class NekoWindow : public QMainWindow {
        Q_OBJECT
    private:
        Page currentPage;
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        QWidget *centralWidget;
        QGraphicsBlurEffect *blurEffect;

        // Dialogs
        dialog::NoticeDialog *noticeDialog;
        dialog::InputDialog *inputDialog;
        // Widgets
        widget::HeadBarWidget *headBarWidget;
        widget::PixmapWidget *pixmapWidget;
        // Pages
        page::HomePage *homePage;
        page::LoadingPage *loadingPage;

    public:
        NekoWindow(const ClientConfig &config);
        ~NekoWindow();
        void settingFromConfig(const ClientConfig &config);
        void switchToPage(Page page);
        void resizeItems(int width, int height);
        void setupTheme(const Theme &theme);
        void setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font);
        void setupConnections();
        void showNotice(const NoticeMsg &m);
        void showInput(const InputMsg &m);
        void hideInput();
        std::vector<std::string> getLines();
        void showLoading(const LoadingMsg &m);
    protected:
        void resizeEvent(QResizeEvent *event) override;
        void closeEvent(QCloseEvent *event) override;
        bool event(QEvent *event) override;

    signals:
        void showNoticeD(const NoticeMsg &m);
        void showInputD(const InputMsg &m);
        void hideInputD();
        std::vector<std::string> getLinesD();
        void resetNoticeStateD();
        void resetNoticeButtonsD();
        void switchToPageD(Page page);
        void setLoadingValueD(int value);
        void setLoadingStatusD(const std::string& msg);
        
    };

} // namespace neko::ui::window
