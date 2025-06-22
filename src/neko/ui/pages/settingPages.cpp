#include "neko/ui/pages/settingPages.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/core/resources.hpp"
#include "neko/schema/clientconfig.hpp"

#include "neko/network/network.hpp"

#include "neko/function/exec.hpp"
#include "neko/function/info.hpp"

#include "nlohmann/json.hpp"

#include <QtGui/QValidator>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
#include <QtWidgets/QVBoxLayout>

namespace neko::ui {

    // --- SettingPageOne ---
    SettingPageOne::SettingPageOne(QWidget *parent)
        : QWidget(parent),
          accountGroup(new QGroupBox(this)),
          accountGroupLayout(new QVBoxLayout(accountGroup)),
          accountLogInOutLayoutWidget(new QWidget(accountGroup)),
          accountLogInOutLayout(new QHBoxLayout(accountLogInOutLayoutWidget)),
          accountLogInOutInfoText(new QLabel(accountLogInOutLayoutWidget)),
          accountLogInOutButton(new QPushButton(accountLogInOutLayoutWidget)) {
        accountLogInOutLayout->addWidget(accountLogInOutInfoText);
        accountLogInOutLayout->addWidget(accountLogInOutButton);
        accountLogInOutLayoutWidget->setLayout(accountLogInOutLayout);
    }

    void SettingPageOne::setupConnects(std::function<void(const InputMsg &)> inputDialog, std::function<void(const HintMsg &)> hintDialog) {

        connect(accountLogInOutButton, &QPushButton::clicked, [&, this]() {
            auto logoutFunc = [=, this]() {
                core::getThreadPool().enqueue([] {
                    neko::ClientConfig cfg(core::getConfigObj());

                    auto url = network::NetworkBase::buildUrl(network::NetworkBase::Api::Authlib::invalidate, network::NetworkBase::Api::Authlib::host);
                    nlohmann::json json = {
                        {"accessToken", cfg.minecraft.accessToken}};
                    auto data = json.dump();

                    network::Network net;
                    network::RequestConfig reqConfig;
                    reqConfig.setUrl(url)
                        .setMethod(network::RequestType::Post)
                        .setData(data)
                        .setHeader("Content-Type: application/json")
                        .setRequestId("logout-" + exec::generateRandomString(10));
                    (void)net.execute(reqConfig);

                    cfg.minecraft.account = "";
                    cfg.minecraft.playerName = "";
                    cfg.minecraft.accessToken = "";
                    cfg.minecraft.uuid = "";

                    cfg.save(core::getConfigObj(), info::app::getConfigFileName());
                });
                isAccountLogIn = false;

                accountLogInOutButton->setText(QString::fromStdString(info::lang::translations(info::lang::lang.general.login)));
                accountLogInOutInfoText->setText(QString::fromStdString(info::lang::translations(info::lang::lang.general.notLogin)));
            };

            // logout
            if (isAccountLogIn) {
                hintDialog({info::lang::translations(info::lang::lang.title.logoutConfirm),
                            info::lang::translations(info::lang::lang.general.logoutConfirm),
                            "",
                            std::vector<std::string>{
                                info::lang::lang.general.ok,
                                info::lang::lang.general.cancel},
                            [=, this](neko::uint32 checkId) {
                                if (checkId == 0) { // 0 is the index for "OK"
                                    logoutFunc();
                                }
                            }});
                return;
            }

            auto registerFunc = [=, this]() {
                try {
                    neko::launcherMinecraftAuthlibAndPrefetchedCheck();
                } catch (const std::exception &e) {
                    hintDialog({info::lang::translations(info::lang::lang.title.error),
                                info::lang::translations(info::lang::lang.error.exception),
                                e.what(),
                                1});
                    return;
                }

                neko::ClientConfig cfg(core::getConfigObj());
                nlohmann::json authlibData = nlohmann::json::parse(exec::base64Decode(cfg.minecraft.authlibPrefetched));

                if (authlibData.contains("meta") && authlibData["meta"].contains("links") && authlibData["meta"]["links"].contains("register")) {
                    std::string url = authlibData["meta"]["links"]["register"];
                    QDesktopServices::openUrl(QUrl(QString::fromStdString(url)));
                }
            };

            auto loginFunc = [=, this]() {
                showInput({info::lang::translations(info::lang::lang.title.inputLogin),
                           "",
                           "",
                           {info::lang::translations(info::lang::lang.general.username), info::lang::translations(info::lang::lang.general.password)},
                           [=, this](bool check) {
                               if (!check) {
                                   hideInput();
                                   return;
                               }

                               auto inData = getInput();
                               if (inData.size() != 2) {
                                   showHint({info::lang::translations(info::lang::lang.title.inputNotEnoughParameters), info::lang::translations(info::lang::lang.general.notEnoughParameters), "", 1});
                                   return;
                               }
                               auto hintFunc = [=, this](const ui::hintMsg &m) {
                                   emit this->showHintD(m);
                               };
                               auto callBack = [=, this](const std::string &name) {
                                   emit this->loginStatusChangeD(name);
                               };
                               exec::getThreadObj().enqueue([=, this] {
                                   if (neko::authLogin(inData, hintFunc, callBack) == neko::State::Completed) {
                                       emit this->hideInputD();
                                   }
                               });
                           }});
            };
        });
    }

    void SettingPageOne::setupStyle(const Theme &theme) {
        this->setAttribute(Qt::WA_TranslucentBackground, true);
        this->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        accountGroup->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        accountLogInOutInfoText->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        accountLogInOutButton->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
    }
    void SettingPageOne::setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font) {
        accountGroup->setFont(h2Font);
        accountLogInOutInfoText->setFont(text);
        accountLogInOutButton->setFont(text);
    }

    void SettingPageOne::setupSize() {
        accountLogInOutButton->setMinimumHeight(50);
    }

    void SettingPageOne::resizeItems(int windowWidth, int windowHeight) {

        int groupW = windowWidth * 0.8;
        int groupH = windowHeight * 0.2;
        int groupX = windowWidth * 0.1;
        int groupY = windowHeight * 0.05;

        accountGroup->setGeometry(groupX, groupY, groupW, groupH);
        accountGroup->setMinimumSize(groupW, groupH);
        accountGroup->setMaximumSize(groupW, groupH);

        accountLogInOutLayoutWidget->setGeometry(0, 0, groupW, groupH);
    }

    // --- SettingPageTwo ---
    SettingPageTwo::SettingPageTwo(QWidget *parent)
        : QWidget(parent),
          pageScrollArea(new QScrollArea(this)),
          scrollContent(new QWidget()),
          scrollLayout(new QVBoxLayout(scrollContent)),
          generalGroup(new QGroupBox(scrollContent)),
          generalGroupLayout(new QVBoxLayout(generalGroup)),
          langSelectLayoutWidget(new QWidget(generalGroup)),
          langSelectLayout(new QHBoxLayout(langSelectLayoutWidget)),
          langSelectText(new QLabel(langSelectLayoutWidget)),
          langSelectBox(new QComboBox(langSelectLayoutWidget)),
          bgGroup(new QGroupBox(scrollContent)),
          bgGroupLayout(new QVBoxLayout(bgGroup)),
          bgSelectLayoutWidget(new QWidget(bgGroup)),
          bgSelectLayout(new QHBoxLayout(bgSelectLayoutWidget)),
          bgSelectText(new QLabel(bgSelectLayoutWidget)),
          bgSelectButtonGroup(new QButtonGroup(bgSelectLayoutWidget)),
          bgSelectRadioNone(new QRadioButton(bgSelectLayoutWidget)),
          bgSelectRadioImage(new QRadioButton(bgSelectLayoutWidget)),
          bgInputLayoutWidget(new QWidget(bgGroup)),
          bgInputLayout(new QHBoxLayout(bgInputLayoutWidget)),
          bgInputText(new QLabel(bgInputLayoutWidget)),
          bgInputLineEdit(new QLineEdit(bgInputLayoutWidget)),
          bgInputToolButton(new QToolButton(bgInputLayoutWidget)),
          styleGroup(new QGroupBox(scrollContent)),
          styleGroupLayout(new QVBoxLayout(styleGroup)),
          styleBlurEffectSelectLayoutWidget(new QWidget(styleGroup)),
          styleBlurEffectSelectLayout(new QHBoxLayout(styleBlurEffectSelectLayoutWidget)),
          styleBlurEffectSelectText(new QLabel(styleBlurEffectSelectLayoutWidget)),
          styleBlurEffectButtonGroup(new QButtonGroup(styleBlurEffectSelectLayoutWidget)),
          styleBlurEffectSelectRadioPerformance(new QRadioButton(styleBlurEffectSelectLayoutWidget)),
          styleBlurEffectSelectRadioQuality(new QRadioButton(styleBlurEffectSelectLayoutWidget)),
          styleBlurEffectSelectRadioAnimation(new QRadioButton(styleBlurEffectSelectLayoutWidget)),
          styleBlurEffectRadiusLayoutWidget(new QWidget(styleGroup)),
          styleBlurEffectRadiusLayout(new QHBoxLayout(styleBlurEffectRadiusLayoutWidget)),
          styleBlurEffectRadiusText(new QLabel(styleBlurEffectRadiusLayoutWidget)),
          styleBlurEffectRadiusSlider(new QSlider(styleBlurEffectRadiusLayoutWidget)),
          styleBlurEffectRadiusSpacing(new QWidget(styleBlurEffectRadiusLayoutWidget)),
          stylePointSizeEditLayoutWidget(new QWidget(styleGroup)),
          stylePointSizeEditLayout(new QHBoxLayout(stylePointSizeEditLayoutWidget)),
          stylePointSizeEditText(new QLabel(stylePointSizeEditLayoutWidget)),
          stylePointSizeEditLine(new QLineEdit(stylePointSizeEditLayoutWidget)),
          stylePointSizeValidator(new QIntValidator(1, 99, stylePointSizeEditLayoutWidget)),
          stylePointSizeEditFontBox(new QFontComboBox(stylePointSizeEditLayoutWidget)),
          winGroup(new QGroupBox(scrollContent)),
          winGroupLayout(new QVBoxLayout(winGroup)),
          winSelectLayoutWidget(new QWidget(winGroup)),
          winSelectLayout(new QHBoxLayout(winSelectLayoutWidget)),
          winSysFrameCheckBox(new QCheckBox(winSelectLayoutWidget)),
          winBarKeepRightCheckBox(new QCheckBox(winSelectLayoutWidget)),
          winSizeEditLayoutWidget(new QWidget(winGroup)),
          winSizeEditLayout(new QHBoxLayout(winSizeEditLayoutWidget)),
          winSizeEditText(new QLabel(winSizeEditLayoutWidget)),
          winSizeEditWidth(new QLineEdit(winSizeEditLayoutWidget)),
          winSizeEditTextX(new QLabel(winSizeEditLayoutWidget)),
          winSizeEditHeight(new QLineEdit(winSizeEditLayoutWidget)),
          lcGroup(new QGroupBox(scrollContent)),
          lcGroupLayout(new QVBoxLayout(lcGroup)),
          lcWindowSetLayoutWidget(new QWidget(lcGroup)),
          lcWindowSetLayout(new QHBoxLayout(lcWindowSetLayoutWidget)),
          lcWindowSetText(new QLabel(lcWindowSetLayoutWidget)),
          lcWindowSetBox(new QComboBox(lcWindowSetLayoutWidget)),
          netGroup(new QGroupBox(scrollContent)),
          netGroupLayout(new QVBoxLayout(netGroup)),
          netProxyLayoutWidget(new QWidget(netGroup)),
          netProxyLayout(new QHBoxLayout(netProxyLayoutWidget)),
          netProxyEnable(new QCheckBox(netProxyLayoutWidget)),
          netProxyEdit(new QLineEdit(netProxyLayoutWidget)),
          netThreadLayoutWidget(new QWidget(netGroup)),
          netThreadLayout(new QHBoxLayout(netThreadLayoutWidget)),
          netThreadNotAutoEnable(new QCheckBox(netThreadLayoutWidget)),
          netThreadSetNums(new QLineEdit(netThreadLayoutWidget)),
          netThreadSetNumsValidator(new QIntValidator(1, 256, netThreadLayoutWidget)),
          moreGroup(new QGroupBox(scrollContent)),
          moreGroupLayout(new QVBoxLayout(moreGroup)),
          moreTempLayoutWidget(new QWidget(moreGroup)),
          moreTempLayout(new QHBoxLayout(moreTempLayoutWidget)),
          moreTempText(new QLabel(moreTempLayoutWidget)),
          moreTempEdit(new QLineEdit(moreTempLayoutWidget)),
          moreTempTool(new QToolButton(moreTempLayoutWidget)) {
        // Set up the layout and widgets
        for (const auto &it : info::lang::getLanguages()) {
            langSelectBox->addItem(it.c_str());
        }
        langSelectLayout->addWidget(langSelectText);
        langSelectLayout->addWidget(langSelectBox);

        langSelectLayoutWidget->setLayout(langSelectLayout);

        generalGroupLayout->addWidget(langSelectLayoutWidget);

        generalGroup->setLayout(generalGroupLayout);

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

        styleBlurEffectButtonGroup->addButton(styleBlurEffectSelectRadioPerformance, 1);
        styleBlurEffectButtonGroup->addButton(styleBlurEffectSelectRadioQuality, 2);
        styleBlurEffectButtonGroup->addButton(styleBlurEffectSelectRadioAnimation, 3);

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

        lcGroup->setLayout(lcGroupLayout);

        lcGroupLayout->addWidget(lcWindowSetLayoutWidget);
        lcWindowSetLayoutWidget->setLayout(lcWindowSetLayout);

        lcWindowSetLayout->addWidget(lcWindowSetText);
        lcWindowSetLayout->addWidget(lcWindowSetBox);

        lcWindowSetBox->addItem("");
        lcWindowSetBox->addItem("");
        lcWindowSetBox->addItem("");

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

        scrollLayout->addWidget(generalGroup);
        scrollLayout->addWidget(bgGroup);
        scrollLayout->addWidget(styleGroup);
        scrollLayout->addWidget(winGroup);
        scrollLayout->addWidget(lcGroup);
        scrollLayout->addWidget(netGroup);
        scrollLayout->addWidget(moreGroup);

        scrollContent->setLayout(scrollLayout);

        pageScrollArea->setWidget(scrollContent);
        pageScrollArea->setWidgetResizable(true);
        pageScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        pageScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    void SettingPageTwo::resizeItems(int windowWidth, int windowHeight) {

        int scrollW = windowWidth;
        int scrollH = windowHeight;

        pageScrollArea->setGeometry(0, 0, scrollW, scrollH);
        scrollContent->setGeometry(0, 0, scrollW, scrollH * 2);

        // Example: 調整部分重要元件寬度
        styleBlurEffectRadiusSlider->setMaximumWidth(windowWidth * 0.5);
        stylePointSizeEditLine->setMaximumWidth(windowWidth * 0.5);
        stylePointSizeEditFontBox->setMaximumWidth(windowWidth * 0.32);

        // 分組高度調整
        for (auto group : std::vector<QWidget *>{generalGroup, moreGroup, lcGroup}) {
            group->setMinimumHeight(std::max<double>(110, windowHeight * 0.18));
            group->setMaximumHeight(std::max<double>(680, windowHeight * 0.5));
        }
        for (auto group : std::vector<QWidget *>{bgGroup, winGroup, netGroup}) {
            group->setMinimumHeight(std::max<double>(220, windowHeight * 0.35));
            group->setMaximumHeight(std::max<double>(680, windowHeight * 0.75));
        }
        for (auto group : std::vector<QWidget *>{styleGroup}) {
            group->setMinimumHeight(std::max<double>(330, windowHeight * 0.5));
            group->setMaximumHeight(std::max<double>(900, windowHeight * 0.9));
        }
        // 內部 layout widget 基本尺寸
        for (auto widget : std::vector<QWidget *>{langSelectLayoutWidget, bgSelectLayoutWidget, bgInputLayoutWidget, winSelectLayoutWidget, winSizeEditLayoutWidget, styleBlurEffectRadiusLayoutWidget, styleBlurEffectSelectLayoutWidget, stylePointSizeEditLayoutWidget, lcWindowSetLayoutWidget, netProxyLayoutWidget, netThreadLayoutWidget, moreTempLayoutWidget}) {
            widget->setBaseSize(windowWidth * 0.7, windowHeight * 0.2);
        }
    }

    // --- SettingPageThree ---
    SettingPageThree::SettingPageThree(QWidget *parent)
        : QWidget(parent),
          pageScrollArea(new QScrollArea(this)),
          scrollContent(new QWidget()),
          scrollLayout(new QVBoxLayout(scrollContent)),
          devOptGroup(new QGroupBox(scrollContent)),
          devOptGroupLayout(new QVBoxLayout(devOptGroup)),
          devOptCheckLayoutWidget(new QWidget(devOptGroup)),
          devOptCheckLayout(new QHBoxLayout(devOptCheckLayoutWidget)),
          devOptEnable(new QCheckBox(devOptCheckLayoutWidget)),
          devOptDebug(new QCheckBox(devOptCheckLayoutWidget)),
          devOptTls(new QCheckBox(devOptCheckLayoutWidget)),
          devOptLoadingPage(new QCheckBox(devOptCheckLayoutWidget)),
          devOptHintPage(new QCheckBox(devOptCheckLayoutWidget)),
          devServerInputLayoutWidget(new QWidget(devOptGroup)),
          devServerInputLayout(new QHBoxLayout(devServerInputLayoutWidget)),
          devServerAuto(new QCheckBox(devServerInputLayoutWidget)),
          devServerEdit(new QLineEdit(devServerInputLayoutWidget)) {
        devOptCheckLayout->addWidget(devOptEnable);
        devOptCheckLayout->addWidget(devOptDebug);
        devOptCheckLayout->addWidget(devOptTls);
        devOptCheckLayout->addWidget(devOptLoadingPage);
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

        devOptLoadingPage->hide();
        devOptHintPage->hide();
    }

    // --- SettingPage ---
    SettingPage::SettingPage(QWidget *parent)
        : QWidget(parent),
          m_tabWidget(new QTabWidget(this)),
          m_page1(new SettingPageOne(this)),
          m_page2(new SettingPageTwo(this)),
          m_page3(new SettingPageThree(this)),
          m_scrollArea(new QScrollArea),
          m_scrollAreaLayout(new QVBoxLayout(this)),
          m_closeButton(new QToolButton(m_tabWidget)) {
        this->setLayout(m_scrollAreaLayout);

        m_scrollArea->setWidget(m_tabWidget);
        m_scrollArea->setWidgetResizable(true);
        m_scrollAreaLayout->addWidget(m_scrollArea);

        m_tabWidget->addTab(m_page1, info::lang::translations(info::lang::lang.general.account).c_str());
        m_tabWidget->addTab(m_page2, info::lang::translations(info::lang::lang.general.general).c_str());
        m_tabWidget->addTab(m_page3, "more");

        m_tabWidget->setCornerWidget(m_closeButton, Qt::TopRightCorner);
        m_closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DockWidgetCloseButton));

        m_closeButton->setMinimumSize(25, 25);
    }
    void SettingPage::setupStyle(const Theme &theme) {
        m_tabWidget->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        if (m_page1)
            m_page1->setupStyle(theme);
        if (m_page2)
            m_page2->setupStyle(theme);
        if (m_page3)
            m_page3->setupStyle(theme);
    }
    void SettingPage::setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font) {
        if (m_page1)
            m_page1->setupFont(text, h1Font, h2Font);
        if (m_page2)
            m_page2->setupFont(text, h1Font, h2Font);
        if (m_page3)
            m_page3->setupFont(text, h1Font, h2Font);
    }
    void SettingPage::resizeItems(int windowWidth, int windowHeight) {
        if (!isVisible())
            return;

        m_scrollArea->setGeometry(0, 0, windowWidth, windowHeight);
        m_tabWidget->setGeometry(0, 0, windowWidth, windowHeight);

        if (m_page1)
            m_page1->resizeItems(windowWidth, windowHeight);
        if (m_page2)
            m_page2->resizeItems(windowWidth, windowHeight);
        if (m_page3)
            m_page3->resizeItems(windowWidth, windowHeight);
    }

} // namespace neko::ui
