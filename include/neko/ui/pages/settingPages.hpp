#pragma once

#include "neko/ui/uiMsg.hpp"

#include <QtGui/QValidator>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <functional>
#include <atomic>

namespace neko::ui {

    class SettingPageOne : public QWidget {
        Q_OBJECT
    private:
        QGroupBox *accountGroup;
        QVBoxLayout *accountGroupLayout;
        QWidget *accountLogInOutLayoutWidget;
        QHBoxLayout *accountLogInOutLayout;
        QLabel *accountLogInOutInfoText;
        QPushButton *accountLogInOutButton;

        std::atomic<bool> isLogIn;

    public:
        explicit SettingPageOne(neko::ClientConfig cfg, QWidget *parent = nullptr);

        void setupConfig(neko::ClientConfig cfg);
        void setupConnects(std::function<void(const HintMsg&)> hintDialog,std::function<void(const InputMsg&)> inputDialog,std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);
        void setupText();

        void setupSize();
        void resizeItems(int windowWidth, int windowHeight);

        bool isAccountLogIn() const {
            return isLogIn.load();
        }
    signals:
        void setAccountLogInD(std::string name);
    };

    class SettingPageTwo : public QWidget {
        Q_OBJECT
    private:
        QScrollArea *pageScrollArea;
        QWidget *scrollContent;
        QVBoxLayout *scrollLayout;

        QGroupBox *generalGroup;
        QVBoxLayout *generalGroupLayout;
        QWidget *langSelectLayoutWidget;
        QHBoxLayout *langSelectLayout;
        QLabel *langSelectText;
        QComboBox *langSelectBox;

        QGroupBox *bgGroup;
        QVBoxLayout *bgGroupLayout;
        QWidget *bgSelectLayoutWidget;
        QHBoxLayout *bgSelectLayout;
        QLabel *bgSelectText;
        QButtonGroup *bgSelectButtonGroup;
        QRadioButton *bgSelectRadioNone;
        QRadioButton *bgSelectRadioImage;
        QWidget *bgInputLayoutWidget;
        QHBoxLayout *bgInputLayout;
        QLabel *bgInputText;
        QLineEdit *bgInputLineEdit;
        QToolButton *bgInputToolButton;

        QGroupBox *styleGroup;
        QVBoxLayout *styleGroupLayout;
        QWidget *styleBlurEffectSelectLayoutWidget;
        QHBoxLayout *styleBlurEffectSelectLayout;
        QLabel *styleBlurEffectSelectText;
        QButtonGroup *styleBlurEffectButtonGroup;
        QRadioButton *styleBlurEffectSelectRadioPerformance;
        QRadioButton *styleBlurEffectSelectRadioQuality;
        QRadioButton *styleBlurEffectSelectRadioAnimation;
        QWidget *styleBlurEffectRadiusLayoutWidget;
        QHBoxLayout *styleBlurEffectRadiusLayout;
        QLabel *styleBlurEffectRadiusText;
        QSlider *styleBlurEffectRadiusSlider;
        QWidget *styleBlurEffectRadiusSpacing;
        QWidget *stylePointSizeEditLayoutWidget;
        QHBoxLayout *stylePointSizeEditLayout;
        QLabel *stylePointSizeEditText;
        QLineEdit *stylePointSizeEditLine;
        QValidator *stylePointSizeValidator;
        QFontComboBox *stylePointSizeEditFontBox;

        QGroupBox *winGroup;
        QVBoxLayout *winGroupLayout;
        QWidget *winSelectLayoutWidget;
        QHBoxLayout *winSelectLayout;
        QCheckBox *winSysFrameCheckBox;
        QCheckBox *winBarKeepRightCheckBox;
        QWidget *winSizeEditLayoutWidget;
        QHBoxLayout *winSizeEditLayout;
        QLabel *winSizeEditText;
        QLineEdit *winSizeEditWidth;
        QLabel *winSizeEditTextX;
        QLineEdit *winSizeEditHeight;

        QGroupBox *lcGroup;
        QVBoxLayout *lcGroupLayout;
        QWidget *lcMethodSetLayoutWidget;
        QHBoxLayout *lcMethodSetLayout;
        QLabel *lcMethodSetText;
        QComboBox *lcMethodSetBox;

        QGroupBox *netGroup;
        QVBoxLayout *netGroupLayout;
        QWidget *netProxyLayoutWidget;
        QHBoxLayout *netProxyLayout;
        QCheckBox *netProxyEnable;
        QLineEdit *netProxyEdit;
        QWidget *netThreadLayoutWidget;
        QHBoxLayout *netThreadLayout;
        QCheckBox *netThreadAutoEnable;
        QLineEdit *netThreadSetNums;
        QValidator *netThreadSetNumsValidator;

        QGroupBox *moreGroup;
        QVBoxLayout *moreGroupLayout;
        QWidget *moreTempLayoutWidget;
        QHBoxLayout *moreTempLayout;
        QLabel *moreTempText;
        QLineEdit *moreTempEdit;
        QToolButton *moreTempTool;
        std::atomic<bool> moreTempDialogOpen = false;

    public:
        explicit SettingPageTwo(neko::ClientConfig cfg, QWidget *parent = nullptr);
        
        void setupConfig(neko::ClientConfig cfg);

        void setupConnects(std::function<void(const HintMsg&)> hintDialog, std::function<void(const InputMsg&)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);
        void setupText();
        

        void setupSize();
        void resizeItems(int windowWidth, int windowHeight);

    signals:
        void languageChanged(const std::string &lang);
        
        void backgroundSelectChanged(int selectType);
        void backgroundImageChanged(const std::string& filePath);

        void styleBlurEffectChanged(int effect);
        void styleBlurEffectRadiusChanged(int radius);
        void styleFontChanged(int size, const QFont &font);

        void winSysFrameChanged(bool enabled);
        void winBarKeepRightChanged(bool enabled);
        void winSizeChanged(int width, int height);
    };


    class SettingPageThree : public QWidget {
        Q_OBJECT
    private:
        QScrollArea *pageScrollArea;
        QWidget *scrollContent;
        QVBoxLayout *scrollLayout;
        QGroupBox *devOptGroup;
        QVBoxLayout *devOptGroupLayout;
        QWidget *devOptCheckLayoutWidget;
        QHBoxLayout *devOptCheckLayout;
        QCheckBox *devOptEnable;
        QCheckBox *devOptDebug;
        QCheckBox *devOptTls;
        QCheckBox *devOptLoadingPage;
        QCheckBox *devOptHintDialog;
        QWidget *devServerInputLayoutWidget;
        QHBoxLayout *devServerInputLayout;
        QCheckBox *devServerAuto;
        QLineEdit *devServerEdit;

    public:
        explicit SettingPageThree(neko::ClientConfig cfg, QWidget *parent = nullptr);

        void setupConfig(neko::ClientConfig cfg);

        void setupConnects(std::function<void(const HintMsg&)> hintDialog, std::function<void(const InputMsg&)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);

        void setupSize();
        void resizeItems(int windowWidth, int windowHeight);
    };

    class SettingPage : public QWidget {
        Q_OBJECT
    private:
        QTabWidget *m_tabWidget;
        SettingPageOne *m_page1;
        SettingPageTwo *m_page2;
        SettingPageThree *m_page3;
        QToolButton *m_closeButton;
        QScrollArea *m_scrollArea;
        QVBoxLayout *m_scrollAreaLayout;

    public:
        explicit SettingPage(neko::ClientConfig cfg, QWidget *parent = nullptr);

        void setupConfig(neko::ClientConfig cfg);

        void setupConnects(std::function<void(const HintMsg&)> hintDialog, std::function<void(const InputMsg&)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);

        void setupSize();
        void resizeItems(int windowWidth, int windowHeight);

    signals:
        void closeRequested(); /**< Signal emitted when the close button is clicked */

        // SettingPageOne signals forwards
        void setAccountLogInD(std::string name);

        // SettingPageTwo signals forwards
        void languageChanged(const std::string &lang);
        void backgroundSelectChanged(int selectType);
        void backgroundImageChanged(const std::string& filePath);
        void styleBlurEffectChanged(int effect);
        void styleBlurEffectRadiusChanged(int radius);
        void styleFontChanged(int size, const QFont &font);
        void winSysFrameChanged(bool enabled);
        void winBarKeepRightChanged(bool enabled);
        void winSizeChanged(int width, int height);
    };

} // namespace neko::ui
