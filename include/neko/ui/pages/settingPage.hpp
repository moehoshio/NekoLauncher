#pragma once

#include "neko/app/clientConfig.hpp"
#include "neko/ui/theme.hpp"

#include <QtCore/QString>
#include <QtGui/QFont>
#include <QtWidgets/QWidget>

#include <string>

class QLabel;
class QPushButton;
class QTabWidget;
class QGroupBox;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QFontComboBox;
class QSlider;
class QVBoxLayout;
class QToolButton;
class QScrollArea;

namespace neko::ui::page {

    class SettingPage : public QWidget {
        Q_OBJECT
    private:
        QTabWidget *tabWidget;
        std::string themeDir;
        bool suppressSignals = false;

        QScrollArea *authScroll;

        // Auth tab
        QWidget *authTab;
        QLabel *authStatusLabel;
        QPushButton *authButton;

        // Main settings tab
        QScrollArea *mainScroll;
        QWidget *mainTab;
        QGroupBox *mainGroup;
        QComboBox *languageCombo;
        QComboBox *backgroundTypeCombo;
        QLineEdit *backgroundPathEdit;
        QToolButton *backgroundBrowseBtn;
        QLineEdit *windowSizeEdit;
        QComboBox *launcherMethodCombo;

        QGroupBox *styleGroup;
        QComboBox *themeCombo;
        QPushButton *editThemeBtn;
        QComboBox *animationCombo;
        QComboBox *blurEffectCombo;
        QSlider *blurRadiusSlider;
        QSpinBox *fontPointSizeSpin;
        QFontComboBox *fontFamiliesCombo;

        QGroupBox *networkGroup;
        QSpinBox *threadSpin;
        QCheckBox *proxyCheck;
        QLineEdit *proxyEdit;

        QGroupBox *otherGroup;
        QCheckBox *immediateSaveCheck;
        QLineEdit *customTempDirEdit;
        QToolButton *customTempDirBrowseBtn;
        QToolButton *closeTabButton;

        QGroupBox *minecraftGroup;
        QLineEdit *javaPathEdit;
        QToolButton *javaPathBrowseBtn;
        QComboBox *downloadSourceCombo;
        QLineEdit *customResolutionEdit;
        QLineEdit *joinServerAddressEdit;
        QSpinBox *joinServerPortSpin;

        // Advanced tab
        QScrollArea *advancedScroll;
        QWidget *advancedTab;
        QGroupBox *devGroup;
        QCheckBox *devEnableCheck;
        QCheckBox *devDebugCheck;
        QCheckBox *devLogViewerCheck;
        QCheckBox *devServerCheck;
        QLineEdit *devServerEdit;
        QCheckBox *devTlsCheck;
        QLabel *devPreviewLabel;
        QPushButton *devShowNoticeBtn;
        QPushButton *devShowInputBtn;
        QPushButton *devShowLoadingBtn;
        QPushButton *devShowNewsBtn;

        std::string authStatusText;

        void buildUi();
        void setupCombos();
        void refreshThemeList();
        void applyGroupStyle(const Theme &theme);
        void retranslateUi();
        void updateEditThemeState();
        static bool isBuiltinTheme(const QString &name);

    public:
        SettingPage(QWidget *parent = nullptr);

        void setupTheme(const Theme &theme);
        void setupFont(QFont text, QFont h1Font, QFont h2Font);
        void setupText();
        void resizeItems(int windowWidth, int windowHeight);
        void setWindowSizeDisplay(const QString &size);
        bool isImmediateSaveEnabled() const;

        void settingFromConfig(const neko::ClientConfig &cfg);
        void writeToConfig(neko::ClientConfig &cfg) const;
        void setAuthState(bool loggedIn, const std::string &statusText);

        QString getBackgroundPath() const;

    signals:
        void closeRequested();
        void languageChanged(const QString &langCode);
        void proxyModeChanged(bool useSystemProxy);
        void devServerModeChanged(bool useDefaultServer);
        void loginRequested();
        void logoutRequested();
        void themeChanged(const QString &themeName);
        void fontPointSizeChanged(int pointSize);
        void fontFamiliesChanged(const QString &families);
        void blurEffectChanged(const QString &effect);
        void animationStyleChanged(const QString &style);
        void blurRadiusChanged(int radius);
        void backgroundTypeChanged(const QString &type);
        void backgroundPathChanged(const QString &path);
        void windowSizeEdited(const QString &sizeText);
        void windowSizeApplyRequested(const QString &sizeText);
        void launcherMethodChanged(const QString &method);
        void javaPathChanged(const QString &path);
        void downloadSourceChanged(const QString &source);
        void customResolutionChanged(const QString &resolution);
        void joinServerAddressChanged(const QString &address);
        void joinServerPortChanged(int port);
        void customTempDirChanged(const QString &dir);
        void threadCountChanged(int threads);
        void proxyValueChanged(bool useSystemProxy, const QString &value);
        void configChanged();
        void showNoticePreviewRequested();
        void showInputPreviewRequested();
        void showLoadingPreviewRequested();
        void showNewsPreviewRequested();
    };

} // namespace neko::ui::page
