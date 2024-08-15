#include "mainwindow.h"

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
        netThreadAutoEnable = new QCheckBox(netThreadLayoutWidget);
        netThreadSetNums = new QLineEdit(netThreadLayoutWidget);
        netThreadSetNumsValidator = new QIntValidator(1, 256, netThreadLayoutWidget);

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
        styleBlurEffectRadiusSlider->setValue(10);
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

        netThreadLayout->addWidget(netThreadAutoEnable);
        netThreadLayout->addWidget(netThreadSetNums);
        netThreadLayoutWidget->setLayout(netThreadLayout);

        netThreadSetNums->setValidator(netThreadSetNumsValidator);
        netThreadSetNums->hide();

        scrollLayout->addWidget(bgGroup);
        scrollLayout->addWidget(styleGroup);
        scrollLayout->addWidget(winGroup);
        scrollLayout->addWidget(netGroup);

        scrollContent->setLayout(scrollLayout);

        pageScrollArea->setWidget(scrollContent);
        pageScrollArea->setWidgetResizable(true);
        pageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    }

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
    void MainWindow::resizeItem() {

        bgWidget->setGeometry(-12, -12, width() + 50, height() + 20);

        switch (state) {
            case pageState::index:

                index->resize(this->size());
                index->versionText->setGeometry(QRect(5, this->size().height() - std::max<double>(height() * 0.1, 60), std::max<double>(width() * 0.12, 160), std::max<double>(height() * 0.1, 60)));
                index->startButton->setGeometry(QRect(this->size().width() * 0.33, this->size().height() * 0.78, this->size().width() * 0.36, this->size().height() * 0.18));
                index->menuButton->setGeometry(QRect(this->size().width() * 0.82, this->size().height() * 0.82, this->size().width() * 0.15, this->size().height() * 0.1));

                break;
            case pageState::setting: {
                setting->resize(this->size());
                setting->page1->accountGroup->setGeometry(QRect(setting->page1->width() * 0.1, setting->page1->height() * 0.05, width() * 0.8, height() * 0.2));
                setting->page1->accountLogInOutLayoutWidget->setGeometry(QRect(0, 0, width() * 0.8, height() * 0.2));
                setting->page2->setGeometry(QRect(0, 0, width(), height()));
                setting->page2->scrollContent->setGeometry(QRect(0, 0, width(), height() * 2));
                setting->page2->pageScrollArea->setGeometry(QRect(0, 0, width(), height() - 8));
                setting->page2->styleBlurEffectRadiusSlider->setMaximumWidth(width() * 0.5);
                setting->page2->stylePointSizeEditLine->setMaximumWidth(width() * 0.5);
                setting->page2->stylePointSizeEditFontBox->setMaximumWidth(width() * 0.32);

                for (auto setTwoTabGroupMMSize : std::vector<QWidget *>{setting->page2->bgGroup, setting->page2->winGroup, setting->page2->netGroup}) {
                    setTwoTabGroupMMSize->setMinimumHeight(std::max<double>(220, height() * 0.35));
                    setTwoTabGroupMMSize->setMaximumHeight(std::max<double>(680, height() * 0.75));
                }
                for (auto setThreeTabGroupMMSize : std::vector<QWidget *>{setting->page2->styleGroup}) {
                    setThreeTabGroupMMSize->setMinimumHeight(std::max<double>(330, height() * 0.5));
                    setThreeTabGroupMMSize->setMaximumHeight(std::max<double>(900, height() * 0.9));
                }
                for (auto setOptBSize : std::vector<QWidget *>{setting->page2->bgSelectLayoutWidget, setting->page2->bgInputLayoutWidget, setting->page2->winSelectLayoutWidget, setting->page2->winSizeEditLayoutWidget, setting->page2->styleBlurEffectRadiusLayoutWidget, setting->page2->styleBlurEffectSelectLayoutWidget, setting->page2->stylePointSizeEditLayoutWidget, setting->page2->netProxyLayoutWidget, setting->page2->netThreadLayoutWidget}) {
                    setOptBSize->setBaseSize(width() * 0.7, height() * 0.2);
                }

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
        bgWidget->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
        widget->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
        widget->setMaximumSize(scrSize);

        setting->setMinimumSize(scrSize.width() * 0.35, scrSize.height() * 0.35);
        setting->setMaximumSize(scrSize);

        setting->closeButton->setMinimumSize(25, 25);

        setting->page2->bgInputToolButton->setMinimumSize(30, 30);

        for (auto groupAddSpacing : std::vector<QVBoxLayout *>{setting->page2->bgGroupLayout, setting->page2->winGroupLayout, setting->page2->styleGroupLayout, setting->page2->netGroupLayout}) {
            groupAddSpacing->addSpacing(12);
        }

        for (auto setMinHeight50 : std::vector<QWidget *>{setting->page1->accountLogInOutButton, setting->page2->bgInputLineEdit, setting->page2->winSizeEditWidth, setting->page2->winSizeEditHeight, setting->page2->styleBlurEffectRadiusSlider, setting->page2->stylePointSizeEditLine, setting->page2->stylePointSizeEditFontBox, setting->page2->netProxyEdit, setting->page2->netThreadSetNums}) {
            setMinHeight50->setMinimumHeight(50);
        }
    }

    void MainWindow::setupStyle() {
        this->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0 rgba(238, 130, 238, 255), stop:0.33 rgba(155,120,236,255) , stop:0.75 rgba(79,146,245,255),stop:1 rgba(40,198, 177,255));");

        setting->setStyleSheet("background-color: rgba(150,150,150,100);");

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

        setting->page2->bgInputToolButton->setStyleSheet("border-radius: 9%;");

        for (auto setGroupBg : std::vector<QWidget *>{
                 setting->page2->bgGroup, setting->page2->styleGroup, setting->page2->winGroup, setting->page2->netGroup}) {
            setGroupBg->setStyleSheet("QGroupBox { border: transparent; background-color: rgba(255,255,255,0);}");
        }

        for (auto setTabOptBg : std::vector<QWidget *>{
                 setting->page2->bgSelectLayoutWidget, setting->page2->bgInputLayoutWidget, setting->page2->styleBlurEffectSelectLayoutWidget, setting->page2->winSelectLayoutWidget, setting->page2->winSizeEditLayoutWidget, setting->page2->styleBlurEffectRadiusLayoutWidget, setting->page2->stylePointSizeEditLayoutWidget, setting->page2->netProxyLayoutWidget, setting->page2->netThreadLayoutWidget}) {
            setTabOptBg->setStyleSheet("background-color: rgba(235,235,235,200);border-radius: 22%;");
        }
        setting->page2->styleBlurEffectRadiusSlider->setStyleSheet("QSlider::handle:horizontal {background-color: rgba(140, 199, 255,255);}");

        setupTranslucentBackground();
    }
    void MainWindow::setupTranslucentBackground() {
        std::vector<QWidget *> vec{widget, bgWidget, index, setting->tabWidget, setting->page1, setting->page2, setting->page3, setting->page2->pageScrollArea, setting->page2->scrollContent};
        for (auto it : vec) {
            it->setAttribute(Qt::WA_TranslucentBackground, true);
        }
        for (auto it : std::vector<QWidget *>{widget, index, setting->page1, setting->page2, setting->page3, setting->page2->pageScrollArea, setting->page2->scrollContent, setting->page2->bgSelectText, setting->page2->bgSelectRadioNone, setting->page2->bgSelectRadioImage, setting->page2->bgInputText, setting->page2->winBarKeepRightCheckBox, setting->page2->winSysFrameCheckBox, setting->page2->winSizeEditText, setting->page2->winSizeEditTextX, setting->page2->styleBlurEffectSelectText, setting->page2->styleBlurEffectSelectRadioPerformance, setting->page2->styleBlurEffectSelectRadioQuality, setting->page2->styleBlurEffectSelectRadioAnimation, setting->page2->styleBlurEffectRadiusText, setting->page2->netProxyEnable, setting->page2->netThreadAutoEnable, setting->page2->styleBlurEffectRadiusSpacing, setting->page2->stylePointSizeEditText}) {
            it->setStyleSheet("background-color: rgba(255,255,255,0);");
        }
    }

    void MainWindow::setupText() {

        index->startButton->setText("START");
        index->menuButton->setText("MENU");
        index->versionText->setText("core: v0.0.0.1\nres: v1.0.0.1");
        setting->page1->accountLogInOutButton->setText("logout");
        setting->page1->accountLogInOutInfoText->setText("i@i.ii");

        setting->page2->bgGroup->setTitle("background");
        setting->page2->styleGroup->setTitle("style");
        setting->page2->winGroup->setTitle("window");
        setting->page2->netGroup->setTitle("network");

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
        setting->page2->netThreadAutoEnable->setText("autoSetThreadNums");

        setting->page2->bgInputLineEdit->setPlaceholderText("img/bg.png");
        setting->page2->stylePointSizeEditLine->setPlaceholderText("point size");
        setting->page2->winSizeEditWidth->setPlaceholderText("width");
        setting->page2->winSizeEditHeight->setPlaceholderText("height");
        setting->page2->netProxyEdit->setPlaceholderText("e.g http://127.0.0.1:1089");

        setting->closeButton->setToolTip("close");
    }

    void MainWindow::setupFont() {
        f.setPointSize(10);
        autoSetText(f);
    }
    void MainWindow::setTextFont(QFont text, QFont h2, QFont h1) {

        for (auto normal : std::vector<QWidget *>{
                 index->versionText, setting->tabWidget, setting->page1->accountLogInOutInfoText, setting->page1->accountLogInOutButton, setting->page2->bgSelectText, setting->page2->bgSelectRadioNone, setting->page2->bgSelectRadioImage, setting->page2->bgInputText, setting->page2->bgInputLineEdit, setting->page2->styleBlurEffectSelectText, setting->page2->styleBlurEffectSelectRadioPerformance, setting->page2->styleBlurEffectSelectRadioQuality, setting->page2->styleBlurEffectSelectRadioAnimation, setting->page2->styleBlurEffectRadiusText, setting->page2->stylePointSizeEditText, setting->page2->stylePointSizeEditLine, setting->page2->stylePointSizeEditFontBox, setting->page2->winSysFrameCheckBox, setting->page2->winBarKeepRightCheckBox, setting->page2->winSizeEditText, setting->page2->winSizeEditWidth, setting->page2->winSizeEditTextX, setting->page2->winSizeEditHeight, setting->page2->netProxyEnable, setting->page2->netProxyEdit, setting->page2->netThreadAutoEnable, setting->page2->netThreadSetNums}) {
            normal->setFont(text);
        }

        for (auto h2Title : std::vector<QWidget *>{
                 index->menuButton, setting->page1->accountGroup, setting->page2->bgGroup, setting->page2->styleGroup, setting->page2->winGroup, setting->page2->netGroup}) {
            h2Title->setFont(h2);
        }

        for (auto h1Title : std::vector<QWidget *>{
                 index->startButton}) {
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

        connect(setting->page2->bgSelectButtonGroup, &QButtonGroup::buttonClicked, [=, this]() {
            int id = setting->page2->bgSelectButtonGroup->checkedId();

            switch (id) {
                case 1:
                    setting->page2->bgSelectText->setText("none");

                    bgWidget->hide();
                    break;
                case 2:
                    setting->page2->bgSelectText->setText("image");

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
        connect(setting->page2->styleBlurEffectRadiusSlider, &QSlider::valueChanged, this, [&](int val) {
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

        connect(setting->page2->stylePointSizeEditFontBox,&QFontComboBox::currentFontChanged,[=,this](const QFont &font){
            
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
        connect(setting->page2->netThreadAutoEnable, &QCheckBox::toggled, [=, this](bool checkd) {
            if (checkd) {
                setting->page2->netThreadSetNums->show();
            } else {
                setting->page2->netThreadSetNums->hide();
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
    }

    MainWindow::MainWindow() {
        bgWidget = new pixmapWidget(this);
        m_pBlurEffect = new QGraphicsBlurEffect;
        widget = new QWidget(this);

        index = new Index(widget);
        setting = new Setting(widget);
        this->setCentralWidget(widget);
        this->setAcceptDrops(true);
        m_pBlurEffect->setBlurRadius(10);
        bgWidget->setGraphicsEffect(m_pBlurEffect);
        bgWidget->setPixmap("img/bg1.png");
        setupSize();
        setupText();

        setupStyle();
        setupConnect();

        setting->hide();

        state = pageState::index;
        oldState = pageState::index;

        bgWidget->lower();
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
            default:
                break;
        }
    }
} // namespace ui