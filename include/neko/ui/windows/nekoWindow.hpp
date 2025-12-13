#pragma once

#include "neko/ui/theme.hpp"
#include "neko/ui/page.hpp"

#include "neko/ui/dialogs/noticeDialog.hpp"
#include "neko/ui/dialogs/inputDialog.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/ui/pages/homePage.hpp"
#include "neko/ui/pages/loadingPage.hpp"
#include "neko/ui/pages/settingPage.hpp"

#include "neko/app/clientConfig.hpp"

#include <QtWidgets/QMainWindow>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtCore/QString>

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
        widget::PixmapWidget *pixmapWidget;
        // Pages
        page::HomePage *homePage;
        page::LoadingPage *loadingPage;
        page::SettingPage *settingPage;

        bool useImageBackground = false;
        void applyCentralBackground(const Theme &theme);
        void showLangLoadWarningIfNeeded();

    public:
        NekoWindow(const ClientConfig &config);
        ~NekoWindow();
        void settingFromConfig(const ClientConfig &config);
        void switchToPage(Page page);
        void resizeItems(int width, int height);
        void setupTheme(const Theme &theme);
        void setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font);
        void setupText();
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
        void showLoadingD(const LoadingMsg &m);
        void hideInputD();
        std::vector<std::string> getLinesD();
        void resetNoticeStateD();
        void resetNoticeButtonsD();
        void switchToPageD(Page page);
        void setLoadingValueD(int value);
        void setLoadingStatusD(const std::string& msg);
        void refreshTextD();

    public slots:
        void onThemeChanged(const QString &themeName);
        void onFontPointSizeChanged(int pointSize);
        void onFontFamiliesChanged(const QString &families);
        void onBlurEffectChanged(const QString &effect);
        void onBlurRadiusChanged(int radius);
        void onBackgroundTypeChanged(const QString &type);
        void onBackgroundPathChanged(const QString &path);
        void onLanguageChanged(const QString &langCode);
		void onLoginRequested();
		void onLogoutRequested();
        
    };

} // namespace neko::ui::window
