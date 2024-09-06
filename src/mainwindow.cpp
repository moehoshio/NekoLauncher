#include "mainwindow.h"
#include "autoinit.h"
#include "exec.h"
#include "info.h"
#include <filesystem>
namespace ui {

    MainWindow::SettingPageOne::SettingPageOne(QWidget *parent) : QWidget(parent) {
        accountGroup = new QGroupBox(this);
        accountGroupLayout = new QVBoxLayout(accountGroup);

        accountLogInOutLayoutWidget = new QWidget(accountGroup);
        accountLogInOutLayout = new QHBoxLayout(accountLogInOutLayoutWidget);
        accountLogInOutInfoText = new QLabel(accountLogInOutLayoutWidget);
        accountLogInOutButton = new QPushButton(accountLogInOutLayoutWidget);

        accountLogInOutLayout->addWidget(accountLogInOutInfoText);
        accountLogInOutLayout->addWidget(accountLogInOutButton);
        accountLogInOutLayoutWidget->setLayout(accountLogInOutLayout);
    };

    MainWindow::SettingPageTwo::SettingPageTwo(QWidget *parent) : QWidget(parent) {
        scrollContent = new QWidget();
        pageScrollArea = new QScrollArea(this);
        scrollLayout = new QVBoxLayout(scrollContent);

        bgGroup = new QGroupBox(scrollContent);
        bgGroupLayout = new QVBoxLayout(bgGroup);

        bgSelectLayoutWidget = new QWidget(bgGroup);
        bgSelectLayout = new QHBoxLayout(bgSelectLayoutWidget);
        bgSelectText = new QLabel(bgSelectLayoutWidget);
        bgSelectButtonGroup = new QButtonGroup(bgSelectLayoutWidget);
        bgSelectRadioNone = new QRadioButton(bgSelectLayoutWidget);
        bgSelectRadioImage = new QRadioButton(bgSelectLayoutWidget);

        bgInputLayoutWidget = new QWidget(bgGroup);
        bgInputLayout = new QHBoxLayout(bgInputLayoutWidget);
        bgInputText = new QLabel(bgInputLayoutWidget);
        bgInputLineEdit = new QLineEdit(bgInputLayoutWidget);
        bgInputToolButton = new QToolButton(bgInputLayoutWidget);

        styleGroup = new QGroupBox(scrollContent);
        styleGroupLayout = new QVBoxLayout(styleGroup);
        styleBlurEffectSelectLayoutWidget = new QWidget(styleGroup);
        styleBlurEffectSelectLayout = new QHBoxLayout(styleBlurEffectSelectLayoutWidget);
        styleBulrEffectButtonGroup = new QButtonGroup(styleBlurEffectSelectLayoutWidget);
        styleBlurEffectSelectText = new QLabel(styleBlurEffectSelectLayoutWidget);
        styleBlurEffectSelectRadioAnimation = new QRadioButton(styleBlurEffectSelectLayoutWidget);
        styleBlurEffectSelectRadioPerformance = new QRadioButton(styleBlurEffectSelectLayoutWidget);
        styleBlurEffectSelectRadioQuality = new QRadioButton(styleBlurEffectSelectLayoutWidget);

        styleBlurEffectRadiusLayoutWidget = new QWidget(styleGroup);
        styleBlurEffectRadiusLayout = new QHBoxLayout(styleBlurEffectRadiusLayoutWidget);

        styleBlurEffectRadiusText = new QLabel(styleBlurEffectRadiusLayoutWidget);
        styleBlurEffectRadiusSlider = new QSlider(styleBlurEffectRadiusLayoutWidget);
        styleBlurEffectRadiusSpacing = new QWidget(styleBlurEffectRadiusLayoutWidget);

        stylePointSizeEditLayoutWidget = new QWidget(styleGroup);
        stylePointSizeEditLayout = new QHBoxLayout(stylePointSizeEditLayoutWidget);
        stylePointSizeEditText = new QLabel(stylePointSizeEditLayoutWidget);
        stylePointSizeEditLine = new QLineEdit(stylePointSizeEditLayoutWidget);
        stylePointSizeValidator = new QIntValidator(1, 99, stylePointSizeEditLayoutWidget);
        stylePointSizeEditFontBox = new QFontComboBox(stylePointSizeEditLayoutWidget);

        winGroup = new QGroupBox(scrollContent);
        winGroupLayout = new QVBoxLayout(winGroup);

        winSelectLayoutWidget = new QWidget(winGroup);
        winSelectLayout = new QHBoxLayout(winSelectLayoutWidget);
        winSysFrameCheckBox = new QCheckBox(winSelectLayoutWidget);
        winBarKeepRightCheckBox = new QCheckBox(winSelectLayoutWidget);

        winSizeEditLayoutWidget = new QWidget(winGroup);
        winSizeEditLayout = new QHBoxLayout(winSizeEditLayoutWidget);
        winSizeEditText = new QLabel(winSizeEditLayoutWidget);
        winSizeEditTextX = new QLabel(winSizeEditLayoutWidget);
        winSizeEditWidth = new QLineEdit(winSizeEditLayoutWidget);
        winSizeEditHeight = new QLineEdit(winSizeEditLayoutWidget);

        netGroup = new QGroupBox(scrollContent);
        netGroupLayout = new QVBoxLayout(netGroup);

        netProxyLayoutWidget = new QWidget(netGroup);
        netProxyLayout = new QHBoxLayout(netProxyLayoutWidget);
        netProxyEnable = new QCheckBox(netProxyLayoutWidget);
        netProxyEdit = new QLineEdit(netProxyLayoutWidget);

        netThreadLayoutWidget = new QWidget(netGroup);
        netThreadLayout = new QHBoxLayout(netThreadLayoutWidget);
        netThreadNotAutoEnable = new QCheckBox(netThreadLayoutWidget);
        netThreadSetNums = new QLineEdit(netThreadLayoutWidget);
        netThreadSetNumsValidator = new QIntValidator(1, 256, netThreadLayoutWidget);

        moreGroup = new QGroupBox(scrollContent);
        moreGroupLayout = new QVBoxLayout(moreGroup);

        moreTempLayoutWidget = new QWidget(moreGroup);
        moreTempLayout = new QHBoxLayout(moreTempLayoutWidget);
        moreTempText = new QLabel(moreTempLayoutWidget);
        moreTempEdit = new QLineEdit(moreTempLayoutWidget);
        moreTempTool = new QToolButton(moreTempLayoutWidget);

        bgSelectButtonGroup->addButton(bgSelectRadioNone, 1);
        bgSelectButtonGroup->addButton(bgSelectRadioImage, 2);

        bgSelectLayout->addWidget(bgSelectText);
        bgSelectLayout->addWidget(bgSelectRadioNone);
        bgSelectLayout->addWidget(bgSelectRadioImage);

        bgSelectLayoutWidget->setLayout(bgSelectLayout);

        bgInputLayout->addWidget(bgInputText);
        bgInputLayout->addWidget(bgInputLineEdit);
        bgInputLayout->addWidget(bgInputToolButton);

        bgInputLayoutWidget->setLayout(bgInputLayout);

        bgGroupLayout->addWidget(bgSelectLayoutWidget);
        bgGroupLayout->addWidget(bgInputLayoutWidget);

        bgGroup->setLayout(bgGroupLayout);

        styleGroupLayout->addWidget(styleBlurEffectSelectLayoutWidget);
        styleGroupLayout->addWidget(styleBlurEffectRadiusLayoutWidget);
        styleGroupLayout->addWidget(stylePointSizeEditLayoutWidget);

        styleBulrEffectButtonGroup->addButton(styleBlurEffectSelectRadioPerformance, 1);
        styleBulrEffectButtonGroup->addButton(styleBlurEffectSelectRadioQuality, 2);
        styleBulrEffectButtonGroup->addButton(styleBlurEffectSelectRadioAnimation, 3);

        styleBlurEffectSelectLayout->addWidget(styleBlurEffectSelectText);
        styleBlurEffectSelectLayout->addWidget(styleBlurEffectSelectRadioPerformance);
        styleBlurEffectSelectLayout->addWidget(styleBlurEffectSelectRadioQuality);
        styleBlurEffectSelectLayout->addWidget(styleBlurEffectSelectRadioAnimation);
        styleBlurEffectSelectLayoutWidget->setLayout(styleBlurEffectSelectLayout);
        styleGroup->setLayout(styleGroupLayout);

        styleBlurEffectRadiusLayout->addWidget(styleBlurEffectRadiusText);
        styleBlurEffectRadiusLayout->addWidget(styleBlurEffectRadiusSlider);
        styleBlurEffectRadiusLayout->addWidget(styleBlurEffectRadiusSpacing);
        styleBlurEffectRadiusSlider->setOrientation(Qt::Horizontal);
        styleBlurEffectRadiusSlider->setRange(0, 22);

        stylePointSizeEditLayoutWidget->setLayout(stylePointSizeEditLayout);
        stylePointSizeEditLayout->addWidget(stylePointSizeEditText);
        stylePointSizeEditLayout->addWidget(stylePointSizeEditLine);
        stylePointSizeEditLayout->addWidget(stylePointSizeEditFontBox);
        stylePointSizeEditLine->setValidator(stylePointSizeValidator);

        winGroup->setLayout(winGroupLayout);

        winGroupLayout->addWidget(winSelectLayoutWidget);
        winGroupLayout->addWidget(winSizeEditLayoutWidget);

        winSelectLayout->addWidget(winSysFrameCheckBox);
        winSelectLayout->addWidget(winBarKeepRightCheckBox);
        winSelectLayoutWidget->setLayout(winSelectLayout);

        winSizeEditLayout->addWidget(winSizeEditText);
        winSizeEditLayout->addWidget(winSizeEditWidth);
        winSizeEditLayout->addWidget(winSizeEditTextX);
        winSizeEditLayout->addWidget(winSizeEditHeight);

        netGroup->setLayout(netGroupLayout);

        netGroupLayout->addWidget(netProxyLayoutWidget);
        netGroupLayout->addWidget(netThreadLayoutWidget);

        netProxyLayoutWidget->setLayout(netProxyLayout);

        netProxyLayout->addWidget(netProxyEnable);
        netProxyLayout->addWidget(netProxyEdit);
        netProxyEdit->hide();

        netThreadLayout->addWidget(netThreadNotAutoEnable);
        netThreadLayout->addWidget(netThreadSetNums);
        netThreadLayoutWidget->setLayout(netThreadLayout);

        netThreadSetNums->setValidator(netThreadSetNumsValidator);
        netThreadSetNums->hide();

        moreGroup->setLayout(moreGroupLayout);
        moreGroupLayout->addWidget(moreTempLayoutWidget);

        moreTempLayoutWidget->setLayout(moreTempLayout);
        moreTempLayout->addWidget(moreTempText);
        moreTempLayout->addWidget(moreTempEdit);
        moreTempLayout->addWidget(moreTempTool);

        scrollLayout->addWidget(bgGroup);
        scrollLayout->addWidget(styleGroup);
        scrollLayout->addWidget(winGroup);
        scrollLayout->addWidget(netGroup);
        scrollLayout->addWidget(moreGroup);

        scrollContent->setLayout(scrollLayout);

        pageScrollArea->setWidget(scrollContent);
        pageScrollArea->setWidgetResizable(true);
        pageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    ui::MainWindow::SettingPageThree::SettingPageThree(QWidget *parent) : QWidget(parent) {
        scrollContent = new QWidget();
        pageScrollArea = new QScrollArea(this);
        scrollLayout = new QVBoxLayout(scrollContent);

        devOptGroup = new QGroupBox(scrollContent);
        devOptGroupLayout = new QVBoxLayout(devOptGroup);

        devOptCheckLayoutWidget = new QWidget(devOptGroup);
        devOptCheckLayout = new QHBoxLayout(devOptCheckLayoutWidget);
        devOptEnable = new QCheckBox(devOptCheckLayoutWidget);
        devOptDebug = new QCheckBox(devOptCheckLayoutWidget);
        devOptTls = new QCheckBox(devOptCheckLayoutWidget);
        devOptUpdatePage = new QCheckBox(devOptCheckLayoutWidget);
        devOptHintPage = new QCheckBox(devOptCheckLayoutWidget);

        devServerInputLayoutWidget = new QWidget(devOptGroup);
        devServerInputLayout = new QHBoxLayout(devServerInputLayoutWidget);
        devServerAuto = new QCheckBox(devServerInputLayoutWidget);
        devServerEdit = new QLineEdit(devServerInputLayoutWidget);

        devOptCheckLayout->addWidget(devOptEnable);
        devOptCheckLayout->addWidget(devOptDebug);
        devOptCheckLayout->addWidget(devOptTls);
        devOptCheckLayout->addWidget(devOptUpdatePage);
        devOptCheckLayout->addWidget(devOptHintPage);
        devOptCheckLayoutWidget->setLayout(devOptCheckLayout);

        devServerInputLayout->addWidget(devServerAuto);
        devServerInputLayout->addWidget(devServerEdit);
        devServerInputLayoutWidget->setLayout(devServerInputLayout);

        devOptGroupLayout->addWidget(devOptCheckLayoutWidget);
        devOptGroupLayout->addWidget(devServerInputLayoutWidget);
        devOptGroup->setLayout(devOptGroupLayout);

        scrollLayout->addWidget(devOptGroup);
        scrollContent->setLayout(scrollLayout);

        pageScrollArea->setWidget(scrollContent);
        pageScrollArea->setWidgetResizable(true);
        pageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    };

    MainWindow::Setting::Setting(QWidget *parent) : QWidget(parent) {
        tabWidget = new QTabWidget(this);
        page1 = new SettingPageOne(this);
        page2 = new SettingPageTwo(this);
        page3 = new SettingPageThree(this);
        scrollArea = new QScrollArea;
        scrollAreaLayout = new QVBoxLayout(this);
        closeButton = new QToolButton(tabWidget);

        this->setLayout(scrollAreaLayout);

        scrollArea->setWidget(tabWidget);
        scrollArea->setWidgetResizable(true);
        scrollAreaLayout->addWidget(scrollArea);

        tabWidget->addTab(page1, "帳號");

        tabWidget->addTab(page2, "一般");

        tabWidget->addTab(page3, "more");

        tabWidget->setCornerWidget(closeButton, Qt::TopRightCorner);

        closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DockWidgetCloseButton));
    };

    MainWindow::LoadingPage::LoadingPage(QWidget *parent) : QWidget(parent) {
        textLayoutWidget = new QWidget(this);
        textLayout = new QVBoxLayout(textLayoutWidget);
        poster = new pixmapWidget(this);
        process = new QLabel(this);
        progressBar = new QProgressBar(this);
        loadingLabel = new QLabel(this);
        loadingMv = new QMovie("./img/loading.gif");
        titleH1 = new QLabel(textLayoutWidget);
        titleH2 = new QLabel(textLayoutWidget);
        text = new QLabel(textLayoutWidget);

        textLayout->addWidget(titleH1);
        textLayout->addWidget(titleH2);
        textLayout->addWidget(text);
        textLayoutWidget->setLayout(textLayout);

        progressBar->setTextVisible(true);
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setOrientation(Qt::Horizontal);
        progressBar->setInvertedAppearance(false);
        
        text->setWordWrap(true);
        process->setWordWrap(true);
        text->setOpenExternalLinks(true);

        loadingMv->start();
        loadingLabel->setAttribute(Qt::WA_NoSystemBackground);
        loadingLabel->setMovie(loadingMv);
        loadingLabel->setScaledContents(true);
        poster->lower();

        for (auto it : std::vector<QLabel *>{titleH1, titleH2, text}) {
            it->setAlignment(Qt::AlignCenter);
        }
        showLoad({loadMsg::OnlyRaw});
    }

    MainWindow::HeadBar::HeadBar(QWidget *parent) : QWidget(parent) {

        toolbar = new ToolBar(this);
        close_ = new QAction(QIcon::fromTheme("window-close"), "close", toolbar);

        minimize = new QAction(QIcon::fromTheme("window-minimize"), "minimize", toolbar);

        maximize = new QAction(QIcon::fromTheme("window-maximize"), "maximize", toolbar);

        sp1 = toolbar->addSeparator();
        sp2 = toolbar->addSeparator();
    }
    MainWindow::HintWindow::HintWindow(QWidget *parent) : QWidget(parent) {
        poster = new pixmapWidget(this);
        centralWidget = new QWidget(this);
        centralWidgetLayout = new QVBoxLayout(centralWidget);
        title = new QLabel(centralWidget);
        line = new QFrame(centralWidget);
        button = new QPushButton(centralWidget);
        dialogButton = new QDialogButtonBox(centralWidget);
        msg = new QLabel(centralWidget);

        centralWidgetLayout->addWidget(title);
        centralWidgetLayout->addWidget(line);
        centralWidgetLayout->addWidget(msg);
        centralWidgetLayout->addWidget(button);
        centralWidgetLayout->addWidget(dialogButton);
        centralWidgetLayout->setStretchFactor(msg, 2);
        centralWidget->setLayout(centralWidgetLayout);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        msg->setWordWrap(true);
        button->hide();
        dialogButton->hide();
        poster->lower();
        dialogButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        dialogButton->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
    }

    void MainWindow::resizeItem() {

        bgWidget->setGeometry(-12, -12, width() + 50, height() + 20);

        int h1;
        if (setting->page2->winSysFrameCheckBox->isChecked())
            h1 = this->height();
        else
            h1 = this->height() - headbar->height();

        if (hintWidget->isVisible()) {
            hintWidget->resize(width(), h1);
            hintWidget->poster->setGeometry(0, 0, this->width(), this->height());
            hintWidget->centralWidget->setGeometry(width() * 0.225, h1 * 0.255, width() * 0.55, h1 * 0.49);
        }

        switch (state) {
            case pageState::index:
                index->resize(size());

                index->versionText->setGeometry(QRect(8, h1 - std::max<double>(h1 * 0.01, 65), std::max<double>(width() * 0.12, 180), std::max<double>(h1 * 0.1, 65)));
                index->startButton->setGeometry(QRect(this->size().width() * 0.33, h1 * 0.78, this->size().width() * 0.36, h1 * 0.18));
                index->menuButton->setGeometry(QRect(this->size().width() * 0.82, h1 * 0.82, this->size().width() * 0.15, h1 * 0.1));

                break;
            case pageState::setting: {
                setting->resize(size());
                setting->page1->accountGroup->setGeometry(QRect(setting->page1->width() * 0.1, setting->page1->height() * 0.05, width() * 0.8, height() * 0.2));
                setting->page1->accountLogInOutLayoutWidget->setGeometry(QRect(0, 0, width() * 0.8, height() * 0.2));
                setting->page2->setGeometry(QRect(0, 0, width(), (setting->page2->winSysFrameCheckBox->isChecked()) ? height() : height() - headbar->height()));
                setting->page2->scrollContent->setGeometry(QRect(0, 0, width(), (setting->page2->winSysFrameCheckBox->isChecked()) ? height() * 2 : height() * 2 - headbar->height()));
                setting->page2->pageScrollArea->setGeometry(QRect(0, 0, width(), (setting->page2->winSysFrameCheckBox->isChecked()) ? height() - 8 : height() - 8 - headbar->height()));
                setting->page3->scrollContent->setGeometry(QRect(0, 0, width(), height() * 2));
                setting->page3->pageScrollArea->setGeometry(QRect(0, 0, width(), height() - 8));
                setting->page2->styleBlurEffectRadiusSlider->setMaximumWidth(width() * 0.5);
                setting->page2->stylePointSizeEditLine->setMaximumWidth(width() * 0.5);
                setting->page2->stylePointSizeEditFontBox->setMaximumWidth(width() * 0.32);

                for (auto setOneTabGroupMMSize : std::vector<QWidget *>{setting->page2->moreGroup}) {
                    setOneTabGroupMMSize->setMinimumHeight(std::max<double>(110, height() * 0.18));
                    setOneTabGroupMMSize->setMaximumHeight(std::max<double>(680, height() * 0.5));
                }
                for (auto setTwoTabGroupMMSize : std::vector<QWidget *>{setting->page2->bgGroup, setting->page2->winGroup, setting->page2->netGroup, setting->page3->devOptGroup}) {
                    setTwoTabGroupMMSize->setMinimumHeight(std::max<double>(220, height() * 0.35));
                    setTwoTabGroupMMSize->setMaximumHeight(std::max<double>(680, height() * 0.75));
                }
                for (auto setThreeTabGroupMMSize : std::vector<QWidget *>{setting->page2->styleGroup}) {
                    setThreeTabGroupMMSize->setMinimumHeight(std::max<double>(330, height() * 0.5));
                    setThreeTabGroupMMSize->setMaximumHeight(std::max<double>(900, height() * 0.9));
                }
                for (auto setOptBSize : std::vector<QWidget *>{setting->page2->bgSelectLayoutWidget, setting->page2->bgInputLayoutWidget, setting->page2->winSelectLayoutWidget, setting->page2->winSizeEditLayoutWidget, setting->page2->styleBlurEffectRadiusLayoutWidget, setting->page2->styleBlurEffectSelectLayoutWidget, setting->page2->stylePointSizeEditLayoutWidget, setting->page2->netProxyLayoutWidget, setting->page2->netThreadLayoutWidget, setting->page2->moreTempLayoutWidget, setting->page3->devOptCheckLayoutWidget, setting->page3->devServerInputLayoutWidget}) {
                    setOptBSize->setBaseSize(width() * 0.7, height() * 0.2);
                }

                break;
            }
            case pageState::loading: {
                loading->resize(size());

                int lwh = std::max<double>(width() * 0.07, h1 * 0.08);
                loading->loadingLabel->setGeometry(3, h1 * 0.78, lwh, lwh);
                loading->textLayoutWidget->setGeometry(width() * 0.25, h1 * 0.15, width() * 0.5, h1 * 0.6);
                loading->progressBar->setGeometry(width() * 0.25, h1 * 0.85, width() * 0.5, h1 * 0.08);
                loading->poster->setGeometry(0, 0, width(), h1);
                loading->process->setGeometry(5, h1 * 0.88, width() * 0.3, h1 * 0.1);

                break;
            }
            default:
                break;
        }
    }
    void MainWindow::resizeEvent(QResizeEvent *event) {

        resizeItem();
        QWidget::resizeEvent(event);
    }

    void MainWindow::setupSize() {
        this->resize(scrSize.width() * 0.45, scrSize.height() * 0.45);

        for (auto setSubWindowSize : std::vector<QWidget *>{
                 widget, setting, hintWidget}) {
            setSubWindowSize->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
            setSubWindowSize->setMaximumSize(scrSize);
        }

        setting->closeButton->setMinimumSize(25, 25);

        setting->page2->bgInputToolButton->setMinimumSize(30, 30);
        setting->page2->moreTempTool->setMinimumSize(30, 30);
        for (auto groupAddSpacing : std::vector<QVBoxLayout *>{setting->page2->bgGroupLayout, setting->page2->winGroupLayout, setting->page2->styleGroupLayout, setting->page2->netGroupLayout, setting->page3->devOptGroupLayout}) {
            groupAddSpacing->addSpacing(12);
        }

        for (auto setMinHeight50 : std::vector<QWidget *>{setting->page1->accountLogInOutButton, setting->page2->bgInputLineEdit, setting->page2->winSizeEditWidth, setting->page2->winSizeEditHeight, setting->page2->styleBlurEffectRadiusSlider, setting->page2->stylePointSizeEditLine, setting->page2->stylePointSizeEditFontBox, setting->page2->netProxyEdit, setting->page2->netThreadSetNums, setting->page2->moreTempEdit, setting->page3->devServerEdit, hintWidget->button}) {
            setMinHeight50->setMinimumHeight(50);
        }
    }

    void MainWindow::setupStyle() {
        this->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(238, 130, 238, 255), stop:0.33 rgba(155,120,236,255) , stop:0.75 rgba(79,146,245,255),stop:1 rgba(40,198, 177,255));");
        headbar->toolbar->setStyleSheet("QToolBar { background-color: rgba(245, 245, 245,230); }"
                                        "QToolButton {background-color: rgba(245, 245, 245,230);}"
                                        "QToolButton:hover {background-color: rgba(155, 155, 155,180);}");

        loading->setStyleSheet("background-color: rgba(150,150,150,100);");
        setting->setStyleSheet("background-color: rgba(150,150,150,100);");
        hintWidget->setStyleSheet("background-color: rgba(150,150,150,100);");
        hintWidget->centralWidget->setStyleSheet("background-color: rgba(235,235,235,200);border-radius: 22%;");
        hintWidget->button->setStyleSheet("QPushButton {border: 2px solid white; background-color: rgba(235,235,235,255);}"
                                          "QPushButton:hover {border: 2px solid rgba(150,150,150,200); background-color: rgba(180,180,180,210);}");
        hintWidget->dialogButton->setStyleSheet("QPushButton {width: 80%; height: 40%; border: 2px solid white; background-color: rgba(235,235,235,255);}"
                                                "QPushButton:hover {border: 2px solid rgba(150,150,150,200); background-color: rgba(180,180,180,210);}");
        loading->progressBar->setStyleSheet("QProgressBar {border: 2px solid #fff;border-radius: 10px;text-align: center;}"
                                            "QProgressBar::chunk {background-color: rgba(120,120,120,188);}");
        loading->textLayoutWidget->setStyleSheet("background-color: rgba(155,155,155,120);border-radius: 22%;");

        index->startButton->setStyleSheet("QPushButton { background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba( 248,248,255,105), stop:0.51 rgba(150,188,215,165), stop:1 rgba( 248,248,255,100));  border-radius: 30%;}"
                                          "QPushButton:hover { color: rgba(245,245,245,235); background-color: rgba(129, 129, 129, 205);}");

        index->menuButton->setStyleSheet("QPushButton {background-color: rgba(240, 240, 240, 225); border-radius: 20%;}"
                                         "QPushButton:hover {background-color: rgba( 255,255,255,238);}"
                                         "QPushButton:checked {background-color: rgba( 155,155,155,138);}");
        setting->tabWidget->setStyleSheet(
            "QTabWidget { background-color: rgba(255,255,255,0)}"
            "QTabWidget::pane {border: 1px solid #C2C7CB; position: absolute; top: -0.5em;}"
            "QTabWidget::tab-bar { alignment: center; padding: 10px }"
            "QTabBar::tab { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #E1E1E1, stop: 0.4 #DDDDDD, stop: 0.5 #D8D8D8, stop: 1.0 #D3D3D3); border: 2px solid #C4C4C3;border-bottom-color: #C2C7CB; border-top-left-radius: 4px; border-top-right-radius: 4px; min-width: 8ex; padding: 2px;}"
            "QTabBar::tab:selected, QTabBar::tab:hover { background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #fafafa, stop: 0.4 #f4f4f4, stop: 0.5 #e7e7e7, stop: 1.0 #fafafa); }"
            "QTabBar::tab:selected { border-color: #9B9B9B; border-bottom-color: #C2C7CB;  }");
        setting->closeButton->setStyleSheet("QToolButton {border: 5px; background-color: rgba(255,255,255,0);}"
                                            "QToolButton::hover {background-color: rgba(150,150,150,200);}");
        setting->page1->accountLogInOutButton->setStyleSheet("QPushButton {background-color: rgba(235,235,235,215);border-radius: 25%;}"
                                                             "QPushButton:hover {background-color: rgba( 255,255,255,238);}");

        for (auto toolButton : std::vector<QWidget *>{
                 setting->page2->bgInputToolButton, setting->page2->moreTempTool}) {
            toolButton->setStyleSheet("border-radius: 9%;");
        }

        for (auto setGroupBg : std::vector<QWidget *>{
                 setting->page2->bgGroup, setting->page2->styleGroup, setting->page2->winGroup, setting->page2->netGroup, setting->page2->moreGroup, setting->page3->devOptGroup}) {
            setGroupBg->setStyleSheet("QGroupBox { border: transparent; background-color: rgba(255,255,255,0);}");
        }

        for (auto setTabOptBg : std::vector<QWidget *>{
                 setting->page2->bgSelectLayoutWidget, setting->page2->bgInputLayoutWidget, setting->page2->styleBlurEffectSelectLayoutWidget, setting->page2->winSelectLayoutWidget, setting->page2->winSizeEditLayoutWidget, setting->page2->styleBlurEffectRadiusLayoutWidget, setting->page2->stylePointSizeEditLayoutWidget, setting->page2->netProxyLayoutWidget, setting->page2->netThreadLayoutWidget, setting->page2->moreTempLayoutWidget, setting->page3->devOptCheckLayoutWidget, setting->page3->devServerInputLayoutWidget}) {
            setTabOptBg->setStyleSheet("background-color: rgba(235,235,235,200);border-radius: 22%;");
        }
        setting->page2->styleBlurEffectRadiusSlider->setStyleSheet("QSlider::handle:horizontal {background-color: rgba(140, 199, 255,255);}");

        setupTranslucentBackground();
    }
    void MainWindow::setupTranslucentBackground() {
        for (auto setTranslucentBackground : std::vector<QWidget *>{widget, bgWidget, index, setting->tabWidget, setting->page1, setting->page2, setting->page3, setting->page2->pageScrollArea, setting->page2->scrollContent, setting->page3->pageScrollArea, setting->page3->scrollContent}) {
            setTranslucentBackground->setAttribute(Qt::WA_TranslucentBackground, true);
        }
        for (auto transparentWidgets : std::vector<QWidget *>{headbar, loading, widget, index, setting->page1, setting->page2, setting->page3, setting->page2->pageScrollArea, setting->page2->scrollContent, setting->page2->bgSelectText, setting->page2->bgSelectRadioNone, setting->page2->bgSelectRadioImage, setting->page2->bgInputText, setting->page2->winBarKeepRightCheckBox, setting->page2->winSysFrameCheckBox, setting->page2->winSizeEditText, setting->page2->winSizeEditTextX, setting->page2->styleBlurEffectSelectText, setting->page2->styleBlurEffectSelectRadioPerformance, setting->page2->styleBlurEffectSelectRadioQuality, setting->page2->styleBlurEffectSelectRadioAnimation, setting->page2->styleBlurEffectRadiusText, setting->page2->netProxyEnable, setting->page2->netThreadNotAutoEnable, setting->page2->styleBlurEffectRadiusSpacing, setting->page2->stylePointSizeEditText, setting->page2->moreTempText, setting->page3->devOptEnable, setting->page3->devOptDebug, setting->page3->devOptTls, setting->page3->devServerAuto, loading->titleH1, loading->titleH2, loading->text, loading->process, hintWidget->title, hintWidget->msg, loading->process}) {
            transparentWidgets->setStyleSheet("background-color: rgba(255,255,255,0);");
        }
    }

    void MainWindow::setupText() {

        index->startButton->setText("START");
        index->menuButton->setText("MENU");
        index->versionText->setText("core: v0.0.0.1\nres: v1.0.0.1");

        loading->process->setText("loading...");
        loading->titleH1->setText("TITLE");
        loading->titleH2->setText("H2");
        loading->text->setText("123\n456\n789\n110\n112\n");
        loading->process->setText("loading..");
        loading->progressBar->setFormat("%v/%m");
        setting->page3->devOptUpdatePage->setText("update");
        setting->page3->devOptHintPage->setText("hint");
        hintWidget->title->setText("Title");
        hintWidget->msg->setText("123\n456\n789\n110\n112\n");
        hintWidget->button->setText("確認");

        setting->page1->accountLogInOutButton->setText("logout");
        setting->page1->accountLogInOutInfoText->setText("i@i.ii");

        setting->page2->bgGroup->setTitle("background");
        setting->page2->styleGroup->setTitle("style");
        setting->page2->winGroup->setTitle("window");
        setting->page2->netGroup->setTitle("network");
        setting->page2->moreGroup->setTitle("more");
        setting->page3->devOptGroup->setTitle("dev");

        setting->page2->bgSelectRadioNone->setText("none");
        setting->page2->bgSelectRadioImage->setText("image");
        setting->page2->bgSelectText->setText("type");
        setting->page2->bgInputText->setText("setting");
        setting->page2->bgInputToolButton->setText("...");

        setting->page2->styleBlurEffectSelectText->setText("blurHint");
        setting->page2->styleBlurEffectSelectRadioAnimation->setText("Animation");
        setting->page2->styleBlurEffectSelectRadioPerformance->setText("Performance");
        setting->page2->styleBlurEffectSelectRadioQuality->setText("Quality");
        setting->page2->styleBlurEffectRadiusText->setText("blurValue");
        setting->page2->stylePointSizeEditText->setText("font");

        setting->page2->winSizeEditText->setText("size");
        setting->page2->winSizeEditTextX->setText("X");
        setting->page2->winSysFrameCheckBox->setText("useSystemWindowFrame");
        setting->page2->winBarKeepRightCheckBox->setText("barKeepRight");

        setting->page2->netProxyEnable->setText("proxy");
        setting->page2->netThreadNotAutoEnable->setText("notAutoSetThreadNums");

        setting->page2->moreTempText->setText("customTempDir");
        setting->page2->moreTempTool->setText("...");

        setting->page3->devOptEnable->setText("enable");
        setting->page3->devOptDebug->setText("debug");
        setting->page3->devOptTls->setText("tls");
        setting->page3->devServerAuto->setText("autoServer");

        setting->page2->bgInputLineEdit->setPlaceholderText("img/bg.png");
        setting->page2->stylePointSizeEditLine->setPlaceholderText("point size");
        setting->page2->winSizeEditWidth->setPlaceholderText("width");
        setting->page2->winSizeEditHeight->setPlaceholderText("height");
        setting->page2->netProxyEdit->setPlaceholderText("default use System proxy ,e.g http://127.0.0.1:1089 ");
        setting->page2->moreTempEdit->setPlaceholderText("temp");

        setting->closeButton->setToolTip("close");
    }

    void MainWindow::setupFont(neko::Config config) {

        f.setPointSize(config.style.fontPointSize);
        setting->page2->stylePointSizeEditLine->setText(QString::number(config.style.fontPointSize));
        if (!std::string(config.style.fontFamilies).empty()) {
            f.setFamilies(QList<QString>{config.style.fontFamilies});
            setting->page2->stylePointSizeEditFontBox->setEditText(config.style.fontFamilies);
        }
        autoSetText(f);
    }
    void MainWindow::setTextFont(QFont text, QFont h2, QFont h1) {

        for (auto normal : std::vector<QWidget *>{
                 hintWidget->msg, loading->process, loading->text, index->versionText, setting->tabWidget, setting->page1->accountLogInOutInfoText, setting->page1->accountLogInOutButton, setting->page2->bgSelectText, setting->page2->bgSelectRadioNone, setting->page2->bgSelectRadioImage, setting->page2->bgInputText, setting->page2->bgInputLineEdit, setting->page2->styleBlurEffectSelectText, setting->page2->styleBlurEffectSelectRadioPerformance, setting->page2->styleBlurEffectSelectRadioQuality, setting->page2->styleBlurEffectSelectRadioAnimation, setting->page2->styleBlurEffectRadiusText, setting->page2->stylePointSizeEditText, setting->page2->stylePointSizeEditLine, setting->page2->stylePointSizeEditFontBox, setting->page2->winSysFrameCheckBox, setting->page2->winBarKeepRightCheckBox, setting->page2->winSizeEditText, setting->page2->winSizeEditWidth, setting->page2->winSizeEditTextX, setting->page2->winSizeEditHeight, setting->page2->netProxyEnable, setting->page2->netProxyEdit, setting->page2->netThreadNotAutoEnable, setting->page2->netThreadSetNums, setting->page2->moreTempText, setting->page2->moreTempEdit, setting->page3->devOptEnable, setting->page3->devOptDebug, setting->page3->devOptTls, setting->page3->devServerAuto, setting->page3->devServerEdit}) {
            normal->setFont(text);
        }

        for (auto h2Title : std::vector<QWidget *>{
                 hintWidget->title, loading->titleH2, index->menuButton, setting->page1->accountGroup, setting->page2->bgGroup, setting->page2->styleGroup, setting->page2->winGroup, setting->page2->netGroup, setting->page2->moreGroup, setting->page3->devOptGroup}) {
            h2Title->setFont(h2);
        }

        for (auto h1Title : std::vector<QWidget *>{
                 loading->titleH1, index->startButton}) {
            h1Title->setFont(h1);
        }
    }
    void MainWindow::autoSetText(QFont text) {
        QFont h1, h2;
        h1.setPointSize(text.pointSize() * 1.8);
        h2.setPointSize(text.pointSize() * 1.2);
        h1.setFamilies(text.families());
        h2.setFamilies(text.families());
        h1.setBold(true);
        setTextFont(text, h2, h1);
    };

    void MainWindow::setupConnect() {
        connect(index->menuButton, &QPushButton::clicked, [=, this]() {
            if (state != pageState::setting) {
                oldState = state;
                state = pageState::setting;
            }
            updatePage(state, oldState);
        });
        QObject::connect(setting->closeButton, &QToolButton::clicked, [=, this]() {
            auto temp = oldState;
            oldState = state;
            state = temp;
            updatePage(state, oldState);
        });
        connect(setting->page2->bgInputLineEdit, &QLineEdit::editingFinished, [=, this]() {
            bgWidget->setPixmap(setting->page2->bgInputLineEdit->text().toStdString().c_str());
        });
        connect(setting->page2->bgInputToolButton, &QPushButton::clicked, [=, this]() {
            auto file = QFileDialog::getOpenFileName(this);
            if (!file.isEmpty()) {
                setting->page2->bgInputLineEdit->setText(file);
                emit setting->page2->bgInputLineEdit->editingFinished();
            }
        });
        connect(
            setting->page2->moreTempEdit, &QLineEdit::editingFinished, [=, this]() {
                neko::info::setTemp(setting->page2->moreTempEdit->text().toStdString().c_str());
            });
        connect(
            setting->page2->moreTempTool, &QPushButton::clicked, [=, this]() {
                auto dir = QFileDialog::getExistingDirectory(this);
                if (!dir.isEmpty()) {
                    setting->page2->moreTempEdit->setText(dir);
                    emit setting->page2->moreTempEdit->editingFinished();
                }
            });

        connect(setting->page2->bgSelectButtonGroup, &QButtonGroup::buttonClicked, [=, this]() {
            int id = setting->page2->bgSelectButtonGroup->checkedId();

            switch (id) {
                case 1:
                    bgWidget->hide();
                    break;
                case 2:
                    bgWidget->show();
                    bgWidget->setPixmap(setting->page2->bgInputLineEdit->text());

                    break;
                default:
                    break;
            }
        });
        connect(setting->page2->styleBulrEffectButtonGroup, &QButtonGroup::buttonClicked, [=, this]() {
            int id = setting->page2->styleBulrEffectButtonGroup->checkedId();
            switch (id) {
                case 1:
                    m_pBlurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
                    break;
                case 2:
                    m_pBlurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
                    break;
                case 3:
                    m_pBlurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
                    break;
                default:
                    break;
            }
        });
        connect(setting->page2->styleBlurEffectRadiusSlider, &QSlider::valueChanged, [&, this](int val) {
            if (val != 1) {
                blurVal = val;
                m_pBlurEffect->setBlurRadius(blurVal);
            }
        });

        connect(setting->page2->stylePointSizeEditLine, &QLineEdit::editingFinished, [=, this]() {
            auto size = setting->page2->stylePointSizeEditLine->text().toUInt();
            if (size != 0) {

                this->f.setPointSize(size);
                autoSetText(f);
            }
        });

        connect(setting->page2->stylePointSizeEditFontBox, &QFontComboBox::currentFontChanged, [=, this](const QFont &font) {
            this->f.setFamilies(font.families());
            autoSetText(f);
        });

        connect(setting->page2->netProxyEnable, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                setting->page2->netProxyEdit->show();
            } else {
                setting->page2->netProxyEdit->hide();
            }
        });
        connect(setting->page2->netThreadNotAutoEnable, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                setting->page2->netThreadSetNums->show();
            } else {
                setting->page2->netThreadSetNums->hide();
            }
        });

        connect(setting->page2->winSysFrameCheckBox, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                this->setWindowFlags(Qt::Window);
                headbar->hide();
                headbar->toolbar->hide();
            } else {
                this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
                headbar->show();
                headbar->toolbar->show();
            }
            this->show();
        });
        connect(setting->page2->winBarKeepRightCheckBox, &QCheckBox::toggled, [=, this](bool checkd) {
            headbar->toolbar->clear();
            if (checkd) {
                QWidget *spacer = new QWidget(headbar->toolbar);
                spacer->setStyleSheet("background-color: rgba(245,245,245,230)");
                spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                headbar->toolbar->addWidget(spacer);
                headbar->toolbar->addActions(
                    {headbar->minimize, headbar->sp1, headbar->maximize, headbar->sp2, headbar->close_});
            } else {
                headbar->toolbar->addActions(
                    {headbar->close_, headbar->sp1, headbar->maximize, headbar->sp2, headbar->minimize});
            }
        });

        connect(setting->page2->winSizeEditWidth, &QLineEdit::editingFinished, [=, this]() {
            auto text = setting->page2->winSizeEditWidth->text();
            if (text.isEmpty())
                return;

            unsigned int val = text.toUInt();
            if (val != 0) {
                this->resize(val, size().height());
                this->resizeItem();
            }
        });
        connect(setting->page2->winSizeEditHeight, &QLineEdit::editingFinished, [=, this]() {
            auto text = setting->page2->winSizeEditHeight->text();
            if (text.isEmpty())
                return;

            unsigned int val = text.toUInt();
            if (val != 0) {
                this->resize(size().width(), val);
                this->resizeItem();
            }
        });

        connect(setting->page3->devServerAuto, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                setting->page3->devServerEdit->hide();
            } else {
                setting->page3->devServerEdit->show();
            }
        });
        connect(headbar->toolbar, &ToolBar::request_move_window,
                [=, this] {
                    this->windowHandle()->startSystemMove();
                });
        connect(headbar->close_, &QAction::triggered,
                [=, this]() { QApplication::quit(); });
        connect(headbar->maximize, &QAction::triggered,
                [=, this] {
                    if (windowState() == Qt::WindowMaximized)
                        setWindowState(Qt::WindowNoState);
                    else
                        setWindowState(Qt::WindowMaximized);
                });
        connect(headbar->minimize, &QAction::triggered,
                [=, this]() { setWindowState(Qt::WindowMinimized); });

        connect(setting->page3->devOptUpdatePage, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                oldState = state;
                state = pageState::loading;
                updatePage(state, oldState);
            }
        });
        connect(setting->page3->devOptHintPage, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                hintWidget->show();
                hintWidget->dialogButton->show();
                resizeItem();
            }
        });
        connect(hintWidget->button, &QPushButton::clicked, [=, this]() {
            hintWidget->hide();
        });
        connect(hintWidget->dialogButton, &QDialogButtonBox::clicked, [=, this]() {
            hintWidget->hide();
        });
        connect(this, &MainWindow::showPageD, this, &MainWindow::showPage);
        connect(this, &MainWindow::showHintD, this, &MainWindow::showHint);
        connect(this, &MainWindow::showLoadD, this, &MainWindow::showLoad);
        connect(this, &MainWindow::setLoadingNowD, this, &MainWindow::setLoadingNow);
        connect(this, &MainWindow::setLoadingValD, this, &MainWindow::setLoadingVal);
    }

    void MainWindow::setupBase(neko::Config config) {

        this->setCentralWidget(widget);
        this->setAcceptDrops(true);
        this->setWindowTitle("Neko Launcher");
        this->setAttribute(Qt::WA_Hover);
        this->addToolBar(headbar->toolbar);
        if (!config.main.useSysWindowFrame) {
            this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
            this->setAttribute(Qt::WA_Hover);
            headbar->show();
        } else {
            headbar->hide();
            headbar->toolbar->hide();
        }

        QIcon icon;
        icon.addFile(QString::fromUtf8("img/ico.png"), QSize(256, 256), QIcon::Normal, QIcon::Off);
        this->setWindowIcon(icon);
        if (std::string("none") == config.main.bgType) {
            setting->page2->bgSelectRadioNone->setChecked(true);
        } else {
            bgWidget->setPixmap(config.main.bg);
            setting->page2->bgSelectRadioImage->setChecked(true);
        }

        setting->page2->bgInputLineEdit->setText(config.main.bg);
        if (config.style.blurValue > 22)
            blurVal = 22;
        else if (config.style.blurValue == 1 || config.style.blurValue < 0)
            blurVal = 0;
        else
            blurVal = config.style.blurValue;

        bgWidget->setGraphicsEffect(m_pBlurEffect);
        setting->page2->styleBlurEffectRadiusSlider->setValue(blurVal);
        m_pBlurEffect->setBlurRadius(blurVal);

        switch (config.style.blurHint) {
            case 2:
                setting->page2->styleBlurEffectSelectRadioQuality->setChecked(true);
                m_pBlurEffect->setBlurHints(QGraphicsBlurEffect::QualityHint);
                break;
            case 3:
                setting->page2->styleBlurEffectSelectRadioAnimation->setChecked(true);
                m_pBlurEffect->setBlurHints(QGraphicsBlurEffect::AnimationHint);
                break;
            case 1:
            default:
                setting->page2->styleBlurEffectSelectRadioPerformance->setChecked(true);
                m_pBlurEffect->setBlurHints(QGraphicsBlurEffect::PerformanceHint);
                break;
        }

        setting->page2->winSysFrameCheckBox->setChecked(config.main.useSysWindowFrame);
        setting->page2->winBarKeepRightCheckBox->setChecked(config.main.barKeepRight);

        if (config.net.thread > 0) {
            setting->page2->netThreadNotAutoEnable->setChecked(true);
            setting->page2->netThreadSetNums->setText(QString::number(config.net.thread));
            setting->page2->netThreadSetNums->show();
        }

        if (std::string("true") == config.net.proxy || exec::isProxyAddress(config.net.proxy)) {
            setting->page2->netProxyEnable->setChecked(true);
            setting->page2->netProxyEdit->show();
            if (exec::isProxyAddress(config.net.proxy))
                setting->page2->netProxyEdit->setText(config.net.proxy);
        }

        if (std::filesystem::is_directory(config.more.temp))
            setting->page2->moreTempEdit->setText(config.more.temp);

        setting->page3->devOptEnable->setChecked(config.dev.enable);
        setting->page3->devOptDebug->setChecked(config.dev.debug);
        setting->page3->devOptTls->setChecked(config.dev.tls);

        if (std::string("auto") == config.dev.server) {
            setting->page3->devServerAuto->setChecked(true);
            setting->page3->devServerEdit->hide();
        } else {
            setting->page3->devServerAuto->setChecked(false);
            setting->page3->devServerEdit->setText(config.dev.server);
            setting->page3->devServerEdit->show();
        }

        state = pageState::loading;
        oldState = pageState::loading;

        index->hide();
        setting->hide();
        hintWidget->raise();
        hintWidget->hide();
        headbar->raise();
        bgWidget->lower();
    }

    MainWindow::MainWindow(neko::Config config) {

        headbar = new HeadBar(this);
        hintWidget = new HintWindow(this);
        bgWidget = new pixmapWidget(this);
        m_pBlurEffect = new QGraphicsBlurEffect;
        widget = new QWidget(this);
        loading = new LoadingPage(widget);
        index = new Index(widget);
        setting = new Setting(widget);

        setupBase(config);
        setupSize();

        setupText();
        setupFont(config);

        setupStyle();
        setupConnect();

        auto sizeRes = exec::matchSizesV(config.main.windowSize);
        if (config.main.windowSize && sizeRes.size() == 3) {
            setting->page2->winSizeEditWidth->setText(sizeRes[1].c_str());
            setting->page2->winSizeEditHeight->setText(sizeRes[2].c_str());
            this->resize(std::stoi(sizeRes[1]), std::stoi(sizeRes[2]));
            resizeItem();
        }

        emit setting->page2->winBarKeepRightCheckBox->toggled(config.main.barKeepRight);
    }

    void MainWindow::updatePage(MainWindow::pageState state, MainWindow::pageState oldState) {
        switch (state) {
            case pageState::index:
                index->show();
                index->raise();
                break;
            case pageState::setting:
                setting->show();
                setting->raise();
                break;
            case pageState::loading:
                loading->show();
                loading->raise();
                break;
            default:
                break;
        }
        resizeItem();
        if (state == oldState)
            return;

        switch (oldState) {
            case pageState::index:
                index->hide();
                break;
            case pageState::setting:
                setting->hide();
                break;
            case pageState::loading:
                loading->hide();
                break;
            default:
                break;
        }
    }
    void MainWindow::closeEvent(QCloseEvent *event) {
        neko::Config cfg(exec::getConfigObj());
        std::string bgText = setting->page2->bgInputLineEdit->text().toStdString();
        switch (setting->page2->bgSelectButtonGroup->checkedId()) {
            case 1:
                cfg.main.bgType = "none";
                break;
            case 2:
                cfg.main.bgType = "image";

                cfg.main.bg = bgText.c_str();
                break;
            default:
                break;
        };
        std::string windowSize = setting->page2->winSizeEditWidth->text().toStdString() + "x" + setting->page2->winSizeEditHeight->text().toStdString();
        cfg.main.windowSize = windowSize.c_str();
        cfg.main.useSysWindowFrame = setting->page2->winSysFrameCheckBox->isChecked();
        cfg.main.barKeepRight = setting->page2->winBarKeepRightCheckBox->isChecked();
        cfg.style.blurHint = setting->page2->styleBulrEffectButtonGroup->checkedId();
        cfg.style.blurValue = setting->page2->styleBlurEffectRadiusSlider->value();
        cfg.style.fontPointSize = setting->page2->stylePointSizeEditLine->text().toInt();
        std::string fontFamiliesText = setting->page2->stylePointSizeEditFontBox->currentText().toStdString();
        cfg.style.fontFamilies = fontFamiliesText.c_str();

        if (setting->page2->netThreadNotAutoEnable->isChecked())
            cfg.net.thread = setting->page2->netThreadSetNums->text().toInt();
        else
            cfg.net.thread = 0;

        std::string proxyText = setting->page2->netProxyEdit->text().toStdString();
        if (setting->page2->netProxyEnable->isChecked())
            cfg.net.proxy = (setting->page2->netProxyEdit->text().isEmpty()) ? "true" : proxyText.c_str();
        else
            cfg.net.proxy = "";
        std::string tempText = setting->page2->moreTempEdit->text().toStdString();
        cfg.more.temp = tempText.c_str();
        cfg.dev.enable = setting->page3->devOptEnable->isChecked();
        cfg.dev.debug = setting->page3->devOptDebug->isChecked();
        cfg.dev.tls = setting->page3->devOptTls->isChecked();
        std::string serverText = setting->page3->devServerEdit->text().toStdString();
        if (setting->page3->devServerAuto->isChecked())
            cfg.dev.server = "auto";
        else
            cfg.dev.server = serverText.c_str();

        neko::configInfoPrint(cfg);

        neko::Config::save(exec::getConfigObj(), "config.ini", cfg);
    }
    bool MainWindow::event(QEvent *event) {
        constexpr qreal border = 11;
        switch (event->type()) {

            case QEvent::HoverMove: {
                auto p = dynamic_cast<QHoverEvent *>(event)->pos();
                bool pointXGreaterWidthBorder = (p.x() > width() - border);
                bool pointXLessWidthBorder = (p.x() < border);
                bool pointYGreaterHeightToolBar = (p.y() > headbar->toolbar->height());
                bool pointYGreaterHeightBorder = (p.y() > height() - border);
                if (pointYGreaterHeightBorder) {
                    if (pointXLessWidthBorder) {
                        this->setCursor(Qt::SizeBDiagCursor);
                    } else if (pointXGreaterWidthBorder) {
                        this->setCursor(Qt::SizeFDiagCursor);
                    } else {
                        this->setCursor(Qt::SizeVerCursor);
                    }
                } else if (pointYGreaterHeightToolBar && (pointXGreaterWidthBorder || pointXLessWidthBorder)) {
                    // Set cursor shape for horizontal resizing...
                    this->setCursor(Qt::SizeHorCursor);
                } else {
                    setCursor(Qt::ArrowCursor);
                }
                break;
            }
            case QEvent::MouseButtonPress: {
                auto p = dynamic_cast<QMouseEvent *>(event)->pos();
                bool pointXGreaterWidthBorder = (p.x() > width() - border);
                bool pointXLessWidthBorder = (p.x() < border);
                bool pointYGreaterHeightBorder = (p.y() > height() - border);
                Qt::Edges edges;
                if (pointXGreaterWidthBorder)
                    edges |= Qt::RightEdge;
                if (pointXLessWidthBorder)
                    edges |= Qt::LeftEdge;
                if (pointYGreaterHeightBorder)
                    edges |= Qt::BottomEdge;
                if (edges != 0) {
                    this->windowHandle()->startSystemResize(edges);
                }
                break;
            }
            default:
                break;
        }

        return QMainWindow::event(event);
    }

} // namespace ui