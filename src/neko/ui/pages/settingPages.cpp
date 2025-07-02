#include "neko/ui/pages/settingPages.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/core/resources.hpp"
#include "neko/schema/clientconfig.hpp"

#include "neko/event/event.hpp"

#include "neko/function/info.hpp"
#include "neko/function/utilities.hpp"

#include "neko/minecraft/account.hpp"

#include "neko/system/platform.hpp"

#include "neko/network/network.hpp"

#include "nlohmann/json.hpp"

#include <QtGui/QValidator>

#include <QtCore/QElapsedTimer>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
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
    SettingPageOne::SettingPageOne(neko::ClientConfig cfg, QWidget *parent)
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

        setupConfig(cfg);
    }

    void SettingPageOne::setupConfig(neko::ClientConfig cfg) {
        isLogIn = util::logic::allTrue(
            cfg.minecraft.playerName,
            cfg.minecraft.account,
            cfg.minecraft.accessToken,
            cfg.minecraft.uuid);
        using namespace neko::info;
        accountLogInOutButton->setText(QString::fromStdString(isLogIn.load() ? lang::tr(lang::Keys::Button::logout) : lang::tr(lang::Keys::Button::login)));
        accountLogInOutInfoText->setText(QString::fromStdString(isLogIn.load() ? cfg.minecraft.playerName : lang::tr(lang::Keys::Auth::notLogin)));
    }

    void SettingPageOne::setupConnects(std::function<void(const HintMsg &)> hintDialog, std::function<void(const InputMsg &)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput) {

        connect(accountLogInOutButton, &QPushButton::clicked, [&, this] {
            using namespace neko::info;
            auto logoutFunc = [=, this]() {
                core::getThreadPool().enqueue([] {
                    neko::minecraft::account::authLogout();
                });
                isLogIn = false;

                accountLogInOutButton->setText(QString::fromStdString(lang::tr(lang::Keys::Button::login)));
                accountLogInOutInfoText->setText(QString::fromStdString(lang::tr(lang::Keys::Auth::notLogin)));
            };

            // logout
            if (isLogIn.load()) {
                hintDialog(ui::HintMsg{
                    .title = lang::tr(lang::Keys::Title::logoutConfirm),
                    .msg = lang::tr(lang::Keys::Auth::logoutConfirm),
                    .poster = "",
                    .buttonText = {lang::tr(lang::Keys::Button::ok),
                                   lang::tr(lang::Keys::Button::cancel)},
                    .callback = [logoutFunc](neko::uint32 checkId) {
                        if (checkId == 0) { // 0 is the index for "OK"
                            logoutFunc();
                        }
                    }});
                return;
            }

            auto registerFunc = [] {
                neko::ClientConfig cfg(core::getConfigObj());
                try {
                    neko::minecraft::account::launcherMinecraftAuthlibAndPrefetchedCheck();

                    nlohmann::json authlibData = nlohmann::json::parse(util::base64::base64Decode(cfg.minecraft.authlibPrefetched));

                    std::string url = authlibData["meta"]["links"]["register"].get<std::string>();
                    QDesktopServices::openUrl(QUrl(QString::fromStdString(url)));
                } catch (const ex::NetworkError &e) {
                    hintDialog(ui::HintMsg{
                        .title = lang::tr(lang::Keys::Title::networkError),
                        .msg = lang::tr(lang::Keys::Network::networkError),
                        .poster = "",
                        .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                    return;
                } catch (const ex::Parse &e) {
                    hintDialog(ui::HintMsg{
                        .title = lang::tr(lang::Keys::Title::parseError),
                        .msg = lang::tr(lang::Keys::Error::jsonParse),
                        .poster = "",
                        .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                    return;
                } catch (const nlohmann::json::parse_error &e) {
                    hintDialog(ui::HintMsg{
                        .title = lang::tr(lang::Keys::Title::parseError),
                        .msg = lang::tr(lang::Keys::Error::jsonParse),
                        .poster = "",
                        .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                    return;
                } catch (const nlohmann::json::out_of_range &e) {
                    hintDialog(ui::HintMsg{
                        .title = lang::tr(lang::Keys::Title::error),
                        .msg = lang::tr(lang::Keys::Auth::noRegisterLink),
                        .poster = "",
                        .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                    return;
                } catch (const std::exception &e) {
                    hintDialog(ui::HintMsg{
                        .title = lang::tr(lang::Keys::Title::error),
                        .msg = e.what(),
                        .poster = "",
                        .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                    return;
                }
            };

            auto loginFunc = [=, this]() {
                inputDialog(ui::InputMsg{
                    .title = lang::tr(lang::Keys::Title::inputLogin),
                    .msg = lang::tr(lang::Keys::Auth::needLogin),
                    .poster = "",
                    .lineText = {
                        lang::tr(lang::Keys::General::username),
                        lang::tr(lang::Keys::General::password)},
                    .callback = [=](bool check) {
                        if (!check) {
                            hideInput();
                            return;
                        }
                        auto inData = getInputLines();
                        for (const auto &line : inData) {
                            if (line.empty()) {
                                hintDialog(ui::HintMsg{
                                    .title = lang::tr(lang::Keys::Title::inputNotEnoughParameters),
                                    .msg = lang::tr(lang::Keys::General::notEnoughParameters),
                                    .poster = "",
                                    .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                                return;
                            }
                        }

                        auto callBack = [this](const std::string &name) {
                            emit this->setAccountLogInD(name);
                        };
                        core::getThreadPool().enqueue([=, this] {
                            auto result = minecraft::account::authLogin(inData);
                            if (!result.error.empty()) {
                                hintDialog(ui::HintMsg{
                                    .title = lang::tr(lang::Keys::Title::error),
                                    .msg = result.error,
                                    .poster = "",
                                    .buttonText = {lang::tr(lang::Keys::Button::ok)}});
                                return;
                            }
                            callBack(result.name);
                        });
                    }});
            };

            // login or register
            hintDialog(ui::HintMsg{
                .title = lang::tr(lang::Keys::Title::loginOrRegister),
                .msg = lang::tr(lang::Keys::Auth::loginOrRegister),
                .poster = "",
                .buttonText = {lang::tr(lang::Keys::Button::login),
                               lang::tr(lang::Keys::Button::Register),
                               lang::tr(lang::Keys::Button::cancel)},
                .callback = [=](neko::uint32 checkId) {
                    if (checkId == 0) { // 0 is the index for "Login"
                        loginFunc();
                    } else if (checkId == 1) { // 1 is the index for "Register"
                        registerFunc();
                    }
                }});
        }); // Connect the account login/logout button

        connect(this, &SettingPageOne::setAccountLogInD, [=, this](std::string name) {
            isLogIn = true;
            accountLogInOutButton->setText(QString::fromStdString(info::lang::tr(info::lang::Keys::Button::logout)));
            accountLogInOutInfoText->setText(QString::fromStdString(name));
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

    void SettingPageOne::setupText() {
        using namespace neko::info;
        neko::ClientConfig cfg(core::getConfigObj());
        std::string accountInfo = cfg.minecraft.account ? cfg.minecraft.playerName : "";
        std::string accessTokenInfo = cfg.minecraft.accessToken ? cfg.minecraft.accessToken : "";
        isLogIn = !accountInfo.empty() && !accessTokenInfo.empty();
        if (isLogIn.load()) {
            accountLogInOutInfoText->setText(QString::fromStdString(accountInfo));
            accountLogInOutButton->setText(QString::fromStdString(lang::tr(lang::Keys::Button::logout)));
        } else {
            // Not logged in
            accountLogInOutInfoText->setText(QString::fromStdString(lang::tr(lang::Keys::Auth::notLogin)));
            accountLogInOutButton->setText(QString::fromStdString(lang::tr(lang::Keys::Button::login)));
        }
    }

    void SettingPageOne::setupSize() {
        accountLogInOutButton->setMinimumHeight(50);
    }

    void SettingPageOne::resizeItems(int windowWidth, int windowHeight) {
        if (!isVisible()) {
            return;
        }

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
    SettingPageTwo::SettingPageTwo(neko::ClientConfig cfg, QWidget *parent)
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
          lcMethodSetLayoutWidget(new QWidget(lcGroup)),
          lcMethodSetLayout(new QHBoxLayout(lcMethodSetLayoutWidget)),
          lcMethodSetText(new QLabel(lcMethodSetLayoutWidget)),
          lcMethodSetBox(new QComboBox(lcMethodSetLayoutWidget)),
          netGroup(new QGroupBox(scrollContent)),
          netGroupLayout(new QVBoxLayout(netGroup)),
          netProxyLayoutWidget(new QWidget(netGroup)),
          netProxyLayout(new QHBoxLayout(netProxyLayoutWidget)),
          netProxyEnable(new QCheckBox(netProxyLayoutWidget)),
          netProxyEdit(new QLineEdit(netProxyLayoutWidget)),
          netThreadLayoutWidget(new QWidget(netGroup)),
          netThreadLayout(new QHBoxLayout(netThreadLayoutWidget)),
          netThreadAutoEnable(new QCheckBox(netThreadLayoutWidget)),
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
            langSelectBox->addItem(QString::fromStdString(it));
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
        styleBlurEffectRadiusSlider->setRange(0, 99);

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
        winSizeEditWidth->setValidator(new QIntValidator(1, 9999, winSizeEditLayoutWidget));
        winSizeEditHeight->setValidator(new QIntValidator(1, 9999, winSizeEditLayoutWidget));

        lcGroup->setLayout(lcGroupLayout);

        lcGroupLayout->addWidget(lcMethodSetLayoutWidget);
        lcMethodSetLayoutWidget->setLayout(lcMethodSetLayout);

        lcMethodSetLayout->addWidget(lcMethodSetText);
        lcMethodSetLayout->addWidget(lcMethodSetBox);

        lcMethodSetBox->addItem("");
        lcMethodSetBox->addItem("");
        lcMethodSetBox->addItem("");

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

        setupConfig(cfg);
    }

    void SettingPageTwo::setupConfig(neko::ClientConfig cfg) {

        langSelectBox->setCurrentText(cfg.main.lang);

        bgSelectRadioNone->setChecked(cfg.main.background == std::string("none"));
        bgSelectRadioImage->setChecked(cfg.main.background != std::string("none"));
        bgInputLineEdit->setText(cfg.main.background);

        styleBlurEffectSelectRadioPerformance->setChecked(cfg.style.blurEffect == std::string("performance"));
        styleBlurEffectSelectRadioQuality->setChecked(cfg.style.blurEffect == std::string("quality"));
        styleBlurEffectSelectRadioAnimation->setChecked(cfg.style.blurEffect == std::string("animation"));
        styleBlurEffectRadiusSlider->setValue(cfg.style.blurRadius);

        stylePointSizeEditLine->setText(QString::number(cfg.style.fontPointSize));
        stylePointSizeEditFontBox->setCurrentFont(QFont(cfg.style.fontFamilies));

        winSysFrameCheckBox->setChecked(cfg.main.useSysWindowFrame);
        winBarKeepRightCheckBox->setChecked(cfg.main.headBarKeepRight);
        auto res = util::check::matchResolution(cfg.main.windowSize);
        if (res.has_value()) {
            winSizeEditWidth->setText(QString::number(res->width));
            winSizeEditHeight->setText(QString::number(res->height));
        }

        lcMethodSetBox->setCurrentIndex(static_cast<int>(cfg.main.launcherMethod));

        netProxyEnable->setChecked();
        netProxyEdit->setText(QString::fromStdString(cfg.net.proxy));

        netThreadAutoEnable->setChecked(cfg.net.threadAutoEnable);
        netThreadSetNums->setText(QString::number(cfg.net.threadNums));
    }

    void SettingPageTwo::setupConnects(std::function<void(const HintMsg &)> hintDialog, std::function<void(const InputMsg &)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput) {
        static QElapsedTimer inputHintTimer;
        auto inputHint = [&inputHintTimer, hintDialog](const std::string &title, const std::string &msg) {
            if (!hintDialog)
                return;
            if (!inputHintTimer.isValid() || inputHintTimer.elapsed() > 2000) {
                inputHintTimer.restart();
                hintDialog(ui::HintMsg{
                    .title = title,
                    .msg = msg,
                    .poster = "",
                    .buttonText = {info::lang::tr(info::lang::Keys::Button::ok)}});
            }
        };

        connect(langSelectBox, &QComboBox::currentTextChanged, [this](const QString &language) {
            std::string lang = language.toStdString();
            info::lang::language(lang);
            core::getThreadPool().enqueue([lang] {
                neko::ClientConfig cfg(core::getConfigObj());
                cfg.main.lang = lang.c_str();
                cfg.save(core::getConfigObj());
            });
            emit this->languageChanged(lang);
        });

        connect(bgSelectButtonGroup, &QButtonGroup::buttonClicked, [this](QAbstractButton *button) {
            int id = bgSelectButtonGroup->id(button);
            emit this->backgroundSelectChanged(id);
        });

        connect(bgInputLineEdit, &QLineEdit::editingFinished, [this]() {
            emit this->backgroundImageChanged(bgInputLineEdit->text().toStdString());
        });

        connect(bgInputToolButton, &QToolButton::clicked, [this]() {
            QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "", tr("Images (*.png *.jpg *.jpeg *.bmp *.webp)"));
            if (!fileName.isEmpty()) {
                bgInputLineEdit->setText(fileName);
                emit this->backgroundImageChanged(fileName.toStdString());
            }
        });

        connect(styleBlurEffectButtonGroup, &QButtonGroup::buttonClicked, [this](QAbstractButton *button) {
            int id = styleBlurEffectButtonGroup->id(button);
            emit this->styleBlurEffectChanged(id);
        });

        connect(styleBlurEffectRadiusSlider, &QSlider::valueChanged, [this](int value) {
            emit this->styleBlurEffectRadiusChanged(value);
        });

        connect(stylePointSizeEditLine, &QLineEdit::editingFinished, [=, this]() {
            bool ok;
            int size = stylePointSizeEditLine->text().toInt(&ok);
            if (ok) {
                emit this->styleFontChanged(size, stylePointSizeEditFontBox->currentFont());
            } else {
                inputHint(
                    info::lang::tr(info::lang::Keys::Title::error),
                    info::lang::tr(info::lang::Keys::Error::invalidInput));
            }
        });

        connect(stylePointSizeEditFontBox, &QFontComboBox::currentFontChanged, [this](const QFont &font) {
            emit this->styleFontChanged(stylePointSizeEditLine->text().toInt(), font);
        });

        connect(winSysFrameCheckBox, &QCheckBox::toggled, [this](bool checked) {
            emit this->winSysFrameChanged(checked);
        });

        connect(winBarKeepRightCheckBox, &QCheckBox::toggled, [this](bool checked) {
            emit this->winBarKeepRightChanged(checked);
        });

        connect(winSizeEditWidth, &QLineEdit::editingFinished, [=, this]() {
            bool ok;
            int width = winSizeEditWidth->text().toInt(&ok);
            if (ok) {
                emit this->winSizeChanged(width, winSizeEditHeight->text().toInt());
            } else {
                inputHint(
                    info::lang::tr(info::lang::Keys::Title::error),
                    info::lang::tr(info::lang::Keys::Error::invalidInput));
            }
        });

        connect(winSizeEditHeight, &QLineEdit::editingFinished, [=, this]() {
            bool ok;
            int height = winSizeEditHeight->text().toInt(&ok);
            if (ok) {
                emit this->winSizeChanged(winSizeEditWidth->text().toInt(), height);
            } else {
                inputHint(
                    info::lang::tr(info::lang::Keys::Title::error),
                    info::lang::tr(info::lang::Keys::Error::invalidInput));
            }
        });

        connect(netProxyEnable, &QCheckBox::toggled, [this](bool checked) {
            netProxyEdit->setVisible(checked);
            std::string proxy;
            if (checked) {
                std::string proxyText = netProxyEdit->text().toStdString();
                if (!proxyText.empty() && util::check::isProxyAddress(proxyText))
                    proxy = proxyText;
                else
                    proxy = "true";
            } else {
                proxy = "";
            }
            network::NetworkBase::globalConfig.proxy = proxy;
        });

        connect(netProxyEdit, &QLineEdit::editingFinished, [this]() {
            std::string proxyText = netProxyEdit->text().toStdString();
            if (!proxyText.empty() && util::check::isProxyAddress(proxyText)) {
                network::NetworkBase::globalConfig.proxy = proxyText;
            }
        });

        connect(netThreadAutoEnable, &QCheckBox::toggled, [=, this](bool checked) {
            netThreadSetNums->setVisible(!checked);

            neko::ClientConfig cfg(core::getConfigObj());
            if (checked) {
                if (cfg.net.thread != 0) {
                    cfg.net.thread = 0;
                    cfg.save(core::getConfigObj());
                    inputHint(
                        info::lang::tr(info::lang::Keys::Title::incomplete),
                        info::lang::tr(info::lang::Keys::General::incompleteApplied));
                }
            } else {
                bool ok;
                auto threadCount = netThreadSetNums->text().toUInt(&ok);
                if (ok) {
                    cfg.net.thread = static_cast<long>(threadCount);
                    cfg.save(core::getConfigObj());
                    inputHint(
                        info::lang::tr(info::lang::Keys::Title::incomplete),
                        info::lang::tr(info::lang::Keys::General::incompleteApplied));
                } else {
                    inputHint(
                        info::lang::tr(info::lang::Keys::Title::error),
                        info::lang::tr(info::lang::Keys::General::invalidInput));
                }
            }
        });

        connect(netThreadSetNums, &QLineEdit::editingFinished, [this]() {
            bool ok;
            auto threadCount = netThreadSetNums->text().toUInt(&ok);
            if (ok) {
                neko::ClientConfig cfg(core::getConfigObj());
                cfg.net.thread = static_cast<long>(threadCount);
                cfg.save(core::getConfigObj());
            } else {
                inputHint(
                    info::lang::tr(info::lang::Keys::Title::error),
                    info::lang::tr(info::lang::Keys::General::invalidInput));
            }
        });

        connect(moreTempEdit, &QLineEdit::editingFinished, [=, this]() {
            std::string tempPath = moreTempEdit->text().toStdString();
            if (!tempPath.empty() && std::filesystem::is_directory(tempPath)) {
                neko::ClientConfig cfg(core::getConfigObj());
                system::temporaryFolder(dir);
                cfg.other.tempFolder = dir.c_str();
                cfg.save(core::getConfigObj());
            } else {
                inputHint(
                    info::lang::tr(info::lang::Keys::Title::error),
                    info::lang::tr(info::lang::Keys::General::invalidInput));
            }
        });

        connect(moreTempTool, &QToolButton::clicked, [=, this]() {
            std::string dir = QFileDialog::getExistingDirectory(this).toStdString();
            moreTempEdit->setText(QString::fromStdString(dir));
            emit this->moreTempEdit->editingFinished()
        });
    }

    void SettingPageTwo::resizeItems(int windowWidth, int windowHeight) {

        int scrollW = windowWidth;
        int scrollH = windowHeight;

        pageScrollArea->setGeometry(0, 0, scrollW, scrollH);
        scrollContent->setGeometry(0, 0, scrollW, scrollH * 2);

        styleBlurEffectRadiusSlider->setMaximumWidth(windowWidth * 0.5);
        stylePointSizeEditLine->setMaximumWidth(windowWidth * 0.5);
        stylePointSizeEditFontBox->setMaximumWidth(windowWidth * 0.32);

        // group widget height adjustment
        for (auto group : std::array{generalGroup, moreGroup, lcGroup}) {
            group->setMinimumHeight(std::max<double>(110, windowHeight * 0.18));
            group->setMaximumHeight(std::max<double>(680, windowHeight * 0.5));
        }
        for (auto group : std::array{bgGroup, winGroup, netGroup}) {
            group->setMinimumHeight(std::max<double>(220, windowHeight * 0.35));
            group->setMaximumHeight(std::max<double>(680, windowHeight * 0.75));
        }
        for (auto group : std::array{styleGroup}) {
            group->setMinimumHeight(std::max<double>(330, windowHeight * 0.5));
            group->setMaximumHeight(std::max<double>(900, windowHeight * 0.9));
        }
        // layout widget height adjustment
        for (auto widget : std::array{langSelectLayoutWidget, bgSelectLayoutWidget, bgInputLayoutWidget, winSelectLayoutWidget, winSizeEditLayoutWidget, styleBlurEffectRadiusLayoutWidget, styleBlurEffectSelectLayoutWidget, stylePointSizeEditLayoutWidget, lcMethodSetLayoutWidget, netProxyLayoutWidget, netThreadLayoutWidget, moreTempLayoutWidget}) {
            widget->setBaseSize(windowWidth * 0.7, windowHeight * 0.2);
        }
    }

    void SettingPageTwo::setupStyle(const Theme &theme) {
        this->setAttribute(Qt::WA_TranslucentBackground, true);
        this->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        scrollContent->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        scrollLayout->setContentsMargins(0, 0, 0, 0);
        scrollLayout->setSpacing(0);
        scrollLayout->setAlignment(Qt::AlignTop);

        for (auto it : std::array{langSelectText, bgSelectText, bgInputText, styleBlurEffectSelectText, styleBlurEffectRadiusText, stylePointSizeEditText, winSizeEditText, winSizeEditTextX, lcMethodSetText, netProxyEnable->text(), netThreadAutoEnable->text(), moreTempText}) {
            it->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        }
        for (auto it : std::array{langSelectBox, bgInputLineEdit, bgInputToolButton, styleBlurEffectRadiusSlider, stylePointSizeEditLine, stylePointSizeEditFontBox, winSizeEditWidth, winSizeEditHeight, lcMethodSetBox, netProxyEdit, netThreadSetNums, moreTempEdit}) {
            it->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        }
        for (auto it : std::array{bgSelectRadioNone, bgSelectRadioImage, styleBlurEffectSelectRadioPerformance, styleBlurEffectSelectRadioQuality, styleBlurEffectSelectRadioAnimation, winSysFrameCheckBox, winBarKeepRightCheckBox}) {
            it->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        }
        for (auto it : std::array{generalGroup, bgGroup, styleGroup, winGroup, lcGroup, netGroup, moreGroup}) {
            it->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        }
        for (auto it : std::array{langSelectLayoutWidget, bgSelectLayoutWidget, bgInputLayoutWidget, styleBlurEffectSelectLayoutWidget, styleBlurEffectRadiusLayoutWidget,
                                  stylePointSizeEditLayoutWidget, winSelectLayoutWidget, winSizeEditLayoutWidget,
                                  lcMethodSetLayoutWidget, netProxyLayoutWidget, netThreadLayoutWidget, moreTempLayoutWidget}) {
            it->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        }
    }

    void SettingPageTwo::setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font) {
        for (auto it : std::array{langSelectText, bgSelectText, bgInputText, styleBlurEffectSelectText, styleBlurEffectRadiusText, stylePointSizeEditText, winSizeEditText, winSizeEditTextX, lcMethodSetText, netProxyEnable->text(), netThreadAutoEnable->text(), moreTempText}) {
            it->setFont(h2Font);
        }
        for (auto it : std::array{langSelectBox, bgInputLineEdit, bgInputToolButton, styleBlurEffectRadiusSlider, stylePointSizeEditLine, stylePointSizeEditFontBox, winSizeEditWidth, winSizeEditHeight, lcMethodSetBox, netProxyEdit, netThreadSetNums, moreTempEdit}) {
            it->setFont(text);
        }
        for (auto it : std::array{bgSelectRadioNone, bgSelectRadioImage, styleBlurEffectSelectRadioPerformance, styleBlurEffectSelectRadioQuality,
                                  styleBlurEffectSelectRadioAnimation, winSysFrameCheckBox, winBarKeepRightCheckBox}) {
            it->setFont(text);
        }
        for (auto it : std::array{generalGroup, bgGroup, styleGroup, winGroup, lcGroup, netGroup, moreGroup}) {
            it->setFont(h1Font);
        }
    }

    void SettingPageTwo::setupText() {
        using namespace neko::info;
        langSelectText->setText(QString::fromStdString(lang::tr(Keys::General::language)));
        bgSelectText->setText(QString::fromStdString(lang::tr(Keys::General::background)));
        styleBlurEffectSelectText->setText(QString::fromStdString(lang::tr(Keys::Style::blurEffectSelect)));
        styleBlurEffectRadiusText->setText(QString::fromStdString(lang::tr(Keys::Style::blurEffectRadius)));
        stylePointSizeEditText->setText(QString::fromStdString(lang::tr(Keys::Style::pointSizeEdit)));
        winSizeEditText->setText(QString::fromStdString(lang::tr(Keys::General::windowSizeEdit)));

        generalGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::general)));
        bgGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::background)));
        styleGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::style)));
        winGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::window)));
        lcGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::launcher)));
        netGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::network)));
        moreGroup->setTitle(QString::fromStdString(lang::tr(Keys::General::more)));

        bgSelectRadioNone->setText(QString::fromStdString(lang::tr(Keys::General::none)));
        bgSelectRadioImage->setText(QString::fromStdString(lang::tr(Keys::General::image)));
        bgInputText->setText(QString::fromStdString(lang::tr(Keys::General::setting)));
        bgInputToolButton->setText(QStringLiteral("..."));

        styleBlurEffectSelectRadioAnimation->setText(QString::fromStdString(lang::tr(Keys::General::animation)));
        styleBlurEffectSelectRadioPerformance->setText(QString::fromStdString(lang::tr(Keys::General::performance)));
        styleBlurEffectSelectRadioQuality->setText(QString::fromStdString(lang::tr(Keys::General::quality)));

        lcMethodSetText->setText(QString::fromStdString(lang::tr(Keys::General::launcherMethod)));
        lcMethodSetBox->setItemText(0, QString::fromStdString(lang::tr(Keys::General::keepWindow)));
        lcMethodSetBox->setItemText(1, QString::fromStdString(lang::tr(Keys::General::endProcess)));
        lcMethodSetBox->setItemText(2, QString::fromStdString(lang::tr(Keys::General::hideAndOverReShow)));

        winSizeEditTextX->setText(QStringLiteral("x"));
        winSysFrameCheckBox->setText(QString::fromStdString(lang::tr(Keys::General::useSystemWindowFrame)));
        winBarKeepRightCheckBox->setText(QString::fromStdString(lang::tr(Keys::General::headBarKeepRight)));

        netProxyEnable->setText(QString::fromStdString(lang::tr(Keys::General::proxy)));
        netThreadAutoEnable->setText(QString::fromStdString(lang::tr(Keys::General::notAutoSetThreadNums)));

        moreTempText->setText(QString::fromStdString(lang::tr(Keys::General::customTempDir)));
        moreTempTool->setText(QStringLiteral("..."));

        // placeholder text
        bgInputLineEdit->setPlaceholderText(QStringLiteral("img/bg.png"));
        stylePointSizeEditLine->setPlaceholderText(QString::fromStdString(lang::tr(Keys::General::pointSize)));
        winSizeEditWidth->setPlaceholderText(QString::fromStdString(lang::tr(Keys::General::width)));
        winSizeEditHeight->setPlaceholderText(QString::fromStdString(lang::tr(Keys::General::height)));
        netProxyEdit->setPlaceholderText(QString::fromStdString(lang::tr(Keys::General::proxyPlaceholder)));
        moreTempEdit->setPlaceholderText(QString::fromStdString(lang::tr(Keys::General::tempFolder)));
    }

    void SettingPageTwo::setupSize() {

        for (auto group : std::array{generalGroup, moreGroup, lcGroup}) {
            group->setMinimumHeight(110);
            group->setMaximumHeight(680);
        }
        for (auto group : std::array{bgGroup, winGroup, netGroup}) {
            group->setMinimumHeight(220);
            group->setMaximumHeight(680);
        }
        for (auto group : std::array{styleGroup}) {
            group->setMinimumHeight(330);
            group->setMaximumHeight(900);
        }
        for (auto widget : std::array{langSelectLayoutWidget, bgSelectLayoutWidget, bgInputLayoutWidget, winSelectLayoutWidget, winSizeEditLayoutWidget, styleBlurEffectRadiusLayoutWidget, styleBlurEffectSelectLayoutWidget, stylePointSizeEditLayoutWidget, lcMethodSetLayoutWidget, netProxyLayoutWidget, netThreadLayoutWidget, moreTempLayoutWidget}) {
            widget->setMinimumHeight(30);
        }
    }

    void SettingPageTwo::resizeItems(int windowWidth, int windowHeight) {

        int scrollW = windowWidth;
        int scrollH = windowHeight;

        pageScrollArea->setGeometry(0, 0, scrollW, scrollH);
        scrollContent->setGeometry(0, 0, scrollW, scrollH * 2);

        styleBlurEffectRadiusSlider->setMaximumWidth(windowWidth * 0.5);
        stylePointSizeEditLine->setMaximumWidth(windowWidth * 0.5);
        stylePointSizeEditFontBox->setMaximumWidth(windowWidth * 0.32);

        // group widget height adjustment
        for (auto group : std::array{generalGroup, moreGroup, lcGroup}) {
            group->setMinimumHeight(std::max<double>(110, windowHeight * 0.18));
            group->setMaximumHeight(std::max<double>(680, windowHeight * 0.5));
        }
        for (auto group : std::array{bgGroup, winGroup, netGroup}) {
            group->setMinimumHeight(std::max<double>(220, windowHeight * 0.35));
            group->setMaximumHeight(std::max<double>(680, windowHeight * 0.75));
        }
        for (auto group : std::array{styleGroup}) {
            group->setMinimumHeight(std::max<double>(330, windowHeight * 0.5));
            group->setMaximumHeight(std::max<double>(900, windowHeight * 0.9));
        }
        // layout widget height adjustment
        for (auto widget : std::array{langSelectLayoutWidget, bgSelectLayoutWidget, bgInputLayoutWidget, winSelectLayoutWidget, winSizeEditLayoutWidget,
                                      styleBlurEffectRadiusLayoutWidget, styleBlurEffectSelectLayoutWidget,
                                      stylePointSizeEditLayoutWidget, lcMethodSetLayoutWidget,
                                      netProxyLayoutWidget, netThreadLayoutWidget,
                                      moreTempLayoutWidget}) {
            widget->setBaseSize(windowWidth * 0.7, windowHeight * 0.2);
        }
    }

    // --- SettingPageThree ---
    SettingPageThree::SettingPageThree(neko::ClientConfig cfg, QWidget *parent)
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
          devOptHintDialog(new QCheckBox(devOptCheckLayoutWidget)),
          devServerInputLayoutWidget(new QWidget(devOptGroup)),
          devServerInputLayout(new QHBoxLayout(devServerInputLayoutWidget)),
          devServerAuto(new QCheckBox(devServerInputLayoutWidget)),
          devServerEdit(new QLineEdit(devServerInputLayoutWidget)) {
        devOptCheckLayout->addWidget(devOptEnable);
        devOptCheckLayout->addWidget(devOptDebug);
        devOptCheckLayout->addWidget(devOptTls);
        devOptCheckLayout->addWidget(devOptLoadingPage);
        devOptCheckLayout->addWidget(devOptHintDialog);
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
        devOptHintDialog->hide();

        setupConfig(cfg);
    }

    SettingPageThree::setupConfig(neko::ClientConfig cfg) {
        devOptEnable->setChecked(cfg.dev.enable);
        devOptDebug->setChecked(cfg.dev.debug);
        devOptTls->setChecked(cfg.dev.tls);

        std::string server = cfg.dev.server ? cfg.dev.server : "auto";
        devServerAuto->setChecked(server.empty() || server == "auto");
        if (devServerAuto->isChecked()) {
            devServerEdit->setText(QString::fromStdString("auto"));
        } else {
            devServerEdit->setText(QString::fromStdString(server));
        }
    }

    SettingPageThree::setupConnects(std::function<void(const HintMsg &)> hintDialog, std::function<void(const InputMsg &)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput) {
        connect(devOptEnable, &QCheckBox::toggled, [this](bool checked) {
            core::getThreadPool().enqueue([checked] {
                neko::ClientConfig cfg(core::getConfigObj());
                cfg.dev.enable = checked;
                cfg.save(core::getConfigObj());
            });
        });

        connect(devOptDebug, &QCheckBox::toggled, [this](bool checked) {
            core::getThreadPool().enqueue([checked] {
                neko::ClientConfig cfg(core::getConfigObj());
                cfg.dev.debug = checked;
                cfg.save(core::getConfigObj());
            });
        });

        connect(devOptTls, &QCheckBox::toggled, [this](bool checked) {
            core::getThreadPool().enqueue([checked] {
                neko::ClientConfig cfg(core::getConfigObj());
                cfg.dev.tls = checked;
                cfg.save(core::getConfigObj());
            });
        });

        connect(devServerAuto, &QCheckBox::toggled, [this](bool checked) {
            devServerEdit->setEnabled(!checked);
            if (checked) {
                devServerEdit->setText(QString::fromStdString("auto"));
            }
            emit this->devServerAutoChanged(checked);
        });

        connect(devServerEdit, &QLineEdit::editingFinished, [this]() {
            std::string server = devServerEdit->text().toStdString();
            if (server.empty() || server == "auto") {
                emit this->devServerChanged("auto");
            } else {
                emit this->devServerChanged(server);
            }
        });
    }

    SettingPageThree::setupStyle(const Theme &theme) {
        this->setAttribute(Qt::WA_TranslucentBackground, true);
        this->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        scrollContent->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));
        scrollLayout->setContentsMargins(0, 0, 0, 0);
        scrollLayout->setSpacing(0);
        scrollLayout->setAlignment(Qt::AlignTop);

        devOptGroup->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devOptEnable->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devOptDebug->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devOptTls->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devOptLoadingPage->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devOptHintDialog->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devServerAuto->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        devServerEdit->setStyleSheet(QString("color: %1;").arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));

        devOptGroupLayout->setContentsMargins(10, 10, 10, 10);
    }

    SettingPageThree::setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font) {
        devOptGroup->setFont(h1Font);
        devOptEnable->setFont(text);
        devOptDebug->setFont(text);
        devOptTls->setFont(text);
        devOptLoadingPage->setFont(text);
        devOptHintDialog->setFont(text);
        devServerAuto->setFont(text);
        devServerEdit->setFont(text);
    }

    SettingPageThree::setupSize() {
        devOptGroup->setMinimumHeight(150);
        devOptGroup->setMaximumHeight(300);
        devOptCheckLayoutWidget->setMinimumHeight(30);
        devServerInputLayoutWidget->setMinimumHeight(30);
    }

    SettingPageThree::resizeItems(int windowWidth, int windowHeight) {
        int scrollW = windowWidth;
        int scrollH = windowHeight;

        pageScrollArea->setGeometry(0, 0, scrollW, scrollH);
        scrollContent->setGeometry(0, 0, scrollW, scrollH * 2);

        devOptGroup->setMinimumHeight(std::max<double>(150, windowHeight * 0.2));
        devOptGroup->setMaximumHeight(std::max<double>(300, windowHeight * 0.4));
    }

    // --- SettingPage ---
    SettingPage::SettingPage(neko::ClientConfig cfg, QWidget *parent)
        : QWidget(parent),
          m_tabWidget(new QTabWidget(this)),
          m_page1(new SettingPageOne(cfg, this)),
          m_page2(new SettingPageTwo(cfg, this)),
          m_page3(new SettingPageThree(cfg, this)),
          m_scrollArea(new QScrollArea),
          m_scrollAreaLayout(new QVBoxLayout(this)),
          m_closeButton(new QToolButton(m_tabWidget)) {
        this->setLayout(m_scrollAreaLayout);

        m_scrollArea->setWidget(m_tabWidget);
        m_scrollArea->setWidgetResizable(true);
        m_scrollAreaLayout->addWidget(m_scrollArea);

        m_tabWidget->addTab(m_page1, info::lang::tr(info::lang::lang.general.account).c_str());
        m_tabWidget->addTab(m_page2, info::lang::tr(info::lang::lang.general.general).c_str());
        m_tabWidget->addTab(m_page3, "dev");

        m_tabWidget->setCornerWidget(m_closeButton, Qt::TopRightCorner);
        m_closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_DockWidgetCloseButton));

        m_closeButton->setMinimumSize(25, 25);

        setupConfig(cfg);
    }

    void SettingPage::setupConfig(neko::ClientConfig cfg) {

        for (auto tab : std::array{m_page1, m_page2, m_page3}) {
            if (tab) {
                tab->setupConfig(cfg);
            }
        }
    }
    void SettingPage::setupConnects(std::function<void(const HintMsg &)> hintDialog, std::function<void(const InputMsg &)> inputDialog, std::function<std::vector<std::string>()> getInputLines, std::function<void()> hideInput) {
        for (auto tab : std::array{m_page1, m_page2, m_page3}) {
            if (tab) {
                tab->setupConnects(hintDialog, inputDialog, getInputLines, hideInput);
            }
        }
        connect(m_closeButton, &QToolButton::clicked, [this]() {
            this->hide();
            emit this->closeRequested();
        });

        // SettingPageOne signals forwards
        if (m_page1) {
            connect(m_page1, &SettingPageOne::setAccountLogInD, this, &SettingPage::setAccountLogInD);
        }

        // SettingPageTwo signals forwards
        if (m_page2) {
            connect(m_page2, &SettingPageTwo::languageChanged, this, &SettingPage::languageChanged);
            connect(m_page2, &SettingPageTwo::backgroundSelectChanged, this, &SettingPage::backgroundSelectChanged);
            connect(m_page2, &SettingPageTwo::backgroundImageChanged, this, &SettingPage::backgroundImageChanged);
            connect(m_page2, &SettingPageTwo::styleBlurEffectChanged, this, &SettingPage::styleBlurEffectChanged);
            connect(m_page2, &SettingPageTwo::styleBlurEffectRadiusChanged, this, &SettingPage::styleBlurEffectRadiusChanged);
            connect(m_page2, &SettingPageTwo::styleFontChanged, this, &SettingPage::styleFontChanged);
            connect(m_page2, &SettingPageTwo::winSysFrameChanged, this, &SettingPage::winSysFrameChanged);
            connect(m_page2, &SettingPageTwo::winBarKeepRightChanged, this, &SettingPage::winBarKeepRightChanged);
            connect(m_page2, &SettingPageTwo::winSizeChanged, this, &SettingPage::winSizeChanged);
        }

    }
    void SettingPage::setupStyle(const Theme &theme) {
        m_tabWidget->setStyleSheet(QString("background-color: %1;").arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));

        for (auto tab : std::array{m_page1, m_page2, m_page3}) {
            if (tab) {
                tab->setupStyle(theme);
            }
        }
    }
    void SettingPage::setupFont(const QFont &text, const QFont &h1Font, const QFont &h2Font) {
        for (auto tab : std::array{m_page1, m_page2, m_page3}) {
            if (tab) {
                tab->setupFont(text, h1Font, h2Font);
            }
        }
    }
    void SettingPage::resizeItems(int windowWidth, int windowHeight) {
        if (!isVisible())
            return;

        m_scrollArea->setGeometry(0, 0, windowWidth, windowHeight);
        m_tabWidget->setGeometry(0, 0, windowWidth, windowHeight);

        for (auto tab : std::array{m_page1, m_page2, m_page3}) {
            if (tab) {
                tab->resizeItems(windowWidth, windowHeight);
            }
        }
    }

} // namespace neko::ui
