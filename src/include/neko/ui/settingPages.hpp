#pragma once
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QTabWidget>

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QToolButton>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>

#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>

#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>

#include <QtWidgets/QGraphicsBlurEffect>

#include <QtGui/QIntValidator>
#include <QtGui/QValidator>

#include <QtGui/QScreen>
#include <QtGui/QWindow>


namespace neko::ui {
    struct SettingPageOne : public QWidget {

        QGroupBox *accountGroup;
        QVBoxLayout *accountGroupLayout;

        // login&out
        QWidget *accountLogInOutLayoutWidget;
        QHBoxLayout *accountLogInOutLayout;
        QLabel *accountLogInOutInfoText;
        QPushButton *accountLogInOutButton;

        SettingPageOne(QWidget *parent = nullptr);
    };

    struct SettingPageTwo : public QWidget {

        QScrollArea *pageScrollArea;
        QWidget *scrollContent;
        QVBoxLayout *scrollLayout;

        // general group
        QGroupBox *generalGroup;
        QVBoxLayout *generalGroupLayout;

        // lang select
        QWidget *langSelectLayoutWidget;
        QHBoxLayout *langSelectLayout;
        QLabel *langSelectText;
        QComboBox *langSelectBox;
        // general group

        // bg group
        QGroupBox *bgGroup;
        QVBoxLayout *bgGroupLayout;

        // type select
        QWidget *bgSelectLayoutWidget;
        QHBoxLayout *bgSelectLayout;
        QLabel *bgSelectText;
        QButtonGroup *bgSelectButtonGroup;
        QRadioButton *bgSelectRadioNone;
        QRadioButton *bgSelectRadioImage;

        // input
        QWidget *bgInputLayoutWidget;
        QHBoxLayout *bgInputLayout;
        QLabel *bgInputText;
        QLineEdit *bgInputLineEdit;
        QToolButton *bgInputToolButton;
        // bg group

        // style group
        QGroupBox *styleGroup;
        QVBoxLayout *styleGroupLayout;

        QWidget *styleBlurEffectSelectLayoutWidget;
        QHBoxLayout *styleBlurEffectSelectLayout;
        QLabel *styleBlurEffectSelectText;
        QButtonGroup *styleBulrEffectButtonGroup;
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
        // style group

        // window group
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
        // window group

        // launcher group
        QGroupBox *lcGroup;
        QVBoxLayout *lcGroupLayout;

        QWidget *lcWindowSetLayoutWidget;
        QHBoxLayout *lcWindowSetLayout;
        QLabel *lcWindowSetText;
        QComboBox *lcWindowSetBox;

        // launcher group

        // network group
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

        // network group

        // more group
        QGroupBox *moreGroup;
        QVBoxLayout *moreGroupLayout;

        QWidget *moreTempLayoutWidget;
        QHBoxLayout *moreTempLayout;
        QLabel *moreTempText;
        QLineEdit *moreTempEdit;
        QToolButton *moreTempTool;
        // more group

        SettingPageTwo(QWidget *parent = nullptr);
    };

    struct SettingPageThree : public QWidget {
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
        SettingPageThree(QWidget *parent = nullptr);
    };

    struct Setting : public QWidget {
        Setting(QWidget *parent = nullptr);

        QTabWidget *tabWidget;
        SettingPageOne *page1;
        SettingPageTwo *page2;
        SettingPageThree *page3;
        QToolButton *closeButton;
        QScrollArea *scrollArea;
        QVBoxLayout *scrollAreaLayout;
    };
} // namespace neko::ui
