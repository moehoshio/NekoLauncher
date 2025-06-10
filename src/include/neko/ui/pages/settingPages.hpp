#pragma once

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

    public:
        explicit SettingPageOne(QWidget *parent = nullptr);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);
        void resizeItems(int windowWidth, int windowHeight);
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
        QWidget *lcWindowSetLayoutWidget;
        QHBoxLayout *lcWindowSetLayout;
        QLabel *lcWindowSetText;
        QComboBox *lcWindowSetBox;

        QGroupBox *netGroup;
        QVBoxLayout *netGroupLayout;
        QWidget *netProxyLayoutWidget;
        QHBoxLayout *netProxyLayout;
        QCheckBox *netProxyEnable;
        QLineEdit *netProxyEdit;
        QWidget *netThreadLayoutWidget;
        QHBoxLayout *netThreadLayout;
        QCheckBox *netThreadNotAutoEnable;
        QLineEdit *netThreadSetNums;
        QValidator *netThreadSetNumsValidator;

        QGroupBox *moreGroup;
        QVBoxLayout *moreGroupLayout;
        QWidget *moreTempLayoutWidget;
        QHBoxLayout *moreTempLayout;
        QLabel *moreTempText;
        QLineEdit *moreTempEdit;
        QToolButton *moreTempTool;

    public:
        explicit SettingPageTwo(QWidget *parent = nullptr);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);
        void resizeItems(int windowWidth, int windowHeight);
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
        QCheckBox *devOptHintPage;
        QWidget *devServerInputLayoutWidget;
        QHBoxLayout *devServerInputLayout;
        QCheckBox *devServerAuto;
        QLineEdit *devServerEdit;

    public:
        explicit SettingPageThree(QWidget *parent = nullptr);

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);
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
        explicit SettingPage(QWidget *parent = nullptr);

        SettingPageOne *page1() const;
        SettingPageTwo *page2() const;
        SettingPageThree *page3() const;
        QTabWidget *tabWidget() const;
        QToolButton *closeButton() const;

        void setupStyle(const Theme &theme);
        void setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font);
        void resizeItems(int windowWidth, int windowHeight);
    };

} // namespace neko::ui
