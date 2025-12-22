#pragma once

#include "neko/ui/page.hpp"
#include "neko/ui/theme.hpp"

#include "neko/ui/dialogs/inputDialog.hpp"
#include "neko/ui/dialogs/noticeDialog.hpp"

#include "neko/ui/widgets/musicWidget.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/ui/pages/aboutPage.hpp"
#include "neko/ui/pages/homePage.hpp"
#include "neko/ui/pages/loadingPage.hpp"
#include "neko/ui/pages/newsPage.hpp"
#include "neko/ui/pages/settingPage.hpp"

#include "neko/app/api.hpp"
#include "neko/app/clientConfig.hpp"
#include "neko/event/event.hpp"

#include <QtCore/QString>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtWidgets/QMainWindow>

class QWidget;
class QGraphicsBlurEffect;
class QTimer;

namespace neko::ui::window {

    class NekoWindow : public QMainWindow {
        Q_OBJECT
    private:
        Page currentPage;
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        QWidget *centralWidget;
        QGraphicsBlurEffect *blurEffect;
        QTimer *windowSizeApplyTimer = nullptr;
        QString pendingWindowSizeText;

        // Dialogs
        dialog::NoticeDialog *noticeDialog;
        dialog::InputDialog *inputDialog;
        // Widgets
        widget::PixmapWidget *pixmapWidget;
        widget::MusicWidget *musicWidget;
        // Pages
        page::AboutPage *aboutPage;
        page::HomePage *homePage;
        page::LoadingPage *loadingPage;
        page::NewsPage *newsPage;
        page::SettingPage *settingPage;

        bool newsPreviewMode = false;

        bool useImageBackground = false;
        bool followSystemTheme = false;
        bool saveImmediately = false;
        void applyThemeSelection(const std::string &themeName);
        void applySystemThemeIfNeeded();
        void applyCentralBackground(const Theme &theme);
        void showLangLoadWarningIfNeeded();
        void persistConfigFromUi(bool saveToFile);
        void applyPendingWindowSize();
        void applyWindowSizeText(const QString &sizeText, bool save);

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
        void setNews(std::vector<api::NewsItem> items, bool hasMore);
        void handleNewsLoadFailed(const std::string &reason);

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
        void setLoadingStatusD(const std::string &msg);
        void refreshTextD();
        void hideWindowD();
        void showWindowD();
        void quitAppD();

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
        void onWindowSizeEdited(const QString &sizeText);
        void onWindowSizeApplyRequested(const QString &sizeText);
        void onConfigChanged();
    };

} // namespace neko::ui::window
