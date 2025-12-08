#include "neko/ui/pages/settingPage.hpp"

#include "neko/bus/configBus.hpp"

#include <neko/log/nlog.hpp>

#include <QtCore/QDir>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

namespace neko::ui::page {

    namespace {
        constexpr int kGroupSpacing = 10;
        constexpr int kTabPadding = 16;

        QVBoxLayout *makeVBox(QWidget *parent, int margin = kTabPadding, int spacing = kGroupSpacing) {
            auto *layout = new QVBoxLayout(parent);
            layout->setContentsMargins(margin, margin, margin, margin);
            layout->setSpacing(spacing);
            return layout;
        }
    } // namespace

    SettingPage::SettingPage(QWidget *parent)
        : QWidget(parent),
          tabWidget(new QTabWidget(this)),
          authScroll(new QScrollArea(tabWidget)),
          authTab(new QWidget()),
          authStatusLabel(new QLabel(authTab)),
          authButton(new QPushButton(authTab)),
          mainScroll(new QScrollArea(tabWidget)),
          mainTab(new QWidget()),
          mainGroup(new QGroupBox(QStringLiteral("Main"), mainTab)),
          backgroundTypeCombo(new QComboBox(mainGroup)),
          backgroundPathEdit(new QLineEdit(mainGroup)),
          windowSizeEdit(new QLineEdit(mainGroup)),
          launcherMethodCombo(new QComboBox(mainGroup)),
          useSysWindowFrameCheck(new QCheckBox(mainGroup)),
          headBarKeepRightCheck(new QCheckBox(mainGroup)),
          styleGroup(new QGroupBox(QStringLiteral("Style"), mainTab)),
          themeEdit(new QLineEdit(styleGroup)),
          blurEffectCombo(new QComboBox(styleGroup)),
          blurRadiusSpin(new QSpinBox(styleGroup)),
          fontPointSizeSpin(new QSpinBox(styleGroup)),
          fontFamiliesEdit(new QLineEdit(styleGroup)),
          networkGroup(new QGroupBox(QStringLiteral("Network"), mainTab)),
          threadSpin(new QSpinBox(networkGroup)),
          proxyCheck(new QCheckBox(networkGroup)),
          proxyEdit(new QLineEdit(networkGroup)),
          otherGroup(new QGroupBox(QStringLiteral("Other"), mainTab)),
          customTempDirEdit(new QLineEdit(otherGroup)),
          customTempDirBrowseBtn(new QToolButton(otherGroup)),
          closeTabButton(new QToolButton(tabWidget)),
          minecraftGroup(new QGroupBox(QStringLiteral("Minecraft"), mainTab)),
          javaPathEdit(new QLineEdit(minecraftGroup)),
          downloadSourceCombo(new QComboBox(minecraftGroup)),
          playerNameEdit(new QLineEdit(minecraftGroup)),
          customResolutionEdit(new QLineEdit(minecraftGroup)),
          joinServerAddressEdit(new QLineEdit(minecraftGroup)),
          joinServerPortSpin(new QSpinBox(minecraftGroup)),
          advancedScroll(new QScrollArea(tabWidget)),
          advancedTab(new QWidget()),
          devGroup(new QGroupBox(QStringLiteral("Advanced"), advancedTab)),
          devEnableCheck(new QCheckBox(devGroup)),
          devDebugCheck(new QCheckBox(devGroup)),
          devServerCheck(new QCheckBox(devGroup)),
          devServerEdit(new QLineEdit(devGroup)),
          devTlsCheck(new QCheckBox(devGroup)) {
        this->setAttribute(Qt::WA_TranslucentBackground);
        buildUi();
        setupCombos();
        setAuthState(false, "Not logged in");
    }

    void SettingPage::buildUi() {
        auto *rootLayout = makeVBox(this, 0, 0);
        rootLayout->addWidget(tabWidget);
        tabWidget->setTabPosition(QTabWidget::North);
        tabWidget->setUsesScrollButtons(true);

        closeTabButton->setText(QStringLiteral("X"));
        closeTabButton->setToolTip(QStringLiteral("Close"));
        closeTabButton->setAutoRaise(true);
        tabWidget->setCornerWidget(closeTabButton, Qt::TopRightCorner);
        connect(closeTabButton, &QToolButton::clicked, this, [this]() {
            emit closeRequested();
        });

        // Auth tab
        authScroll->setWidget(authTab);
        authScroll->setWidgetResizable(true);
        authScroll->setFrameShape(QFrame::NoFrame);
        authScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        authScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        auto *authLayout = makeVBox(authTab);
        authStatusLabel->setWordWrap(true);
        authLayout->addWidget(authStatusLabel);
        authButton->setText(QStringLiteral("Login"));
        authLayout->addWidget(authButton);
        authLayout->addStretch();
        tabWidget->addTab(authScroll, QStringLiteral("Account"));

        // Main tab layout
        mainScroll->setWidget(mainTab);
        mainScroll->setWidgetResizable(true);
        mainScroll->setFrameShape(QFrame::NoFrame);
        mainScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        mainScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        auto *mainLayout = makeVBox(mainTab);
        mainLayout->addWidget(mainGroup);
        mainLayout->addWidget(styleGroup);
        mainLayout->addWidget(networkGroup);
        mainLayout->addWidget(otherGroup);
        mainLayout->addWidget(minecraftGroup);
        mainLayout->addStretch();
        tabWidget->addTab(mainScroll, QStringLiteral("Main"));

        // main group content
        auto *mainGroupLayout = makeVBox(mainGroup, 12, 8);
        mainGroupLayout->addWidget(new QLabel(QStringLiteral("Background Type"), mainGroup));
        mainGroupLayout->addWidget(backgroundTypeCombo);
        mainGroupLayout->addWidget(new QLabel(QStringLiteral("Background"), mainGroup));
        mainGroupLayout->addWidget(backgroundPathEdit);
        mainGroupLayout->addWidget(new QLabel(QStringLiteral("Window Size"), mainGroup));
        mainGroupLayout->addWidget(windowSizeEdit);
        mainGroupLayout->addWidget(new QLabel(QStringLiteral("Launcher Method"), mainGroup));
        mainGroupLayout->addWidget(launcherMethodCombo);
        useSysWindowFrameCheck->setText(QStringLiteral("Use system window frame"));
        headBarKeepRightCheck->setText(QStringLiteral("Head bar keep right"));
        mainGroupLayout->addWidget(useSysWindowFrameCheck);
        mainGroupLayout->addWidget(headBarKeepRightCheck);
        mainGroupLayout->addStretch();

        // style group content
        auto *styleLayout = makeVBox(styleGroup, 12, 8);
        styleLayout->addWidget(new QLabel(QStringLiteral("Theme"), styleGroup));
        styleLayout->addWidget(themeEdit);
        styleLayout->addWidget(new QLabel(QStringLiteral("Blur effect"), styleGroup));
        styleLayout->addWidget(blurEffectCombo);
        styleLayout->addWidget(new QLabel(QStringLiteral("Blur radius"), styleGroup));
        blurRadiusSpin->setRange(0, 128);
        styleLayout->addWidget(blurRadiusSpin);
        styleLayout->addWidget(new QLabel(QStringLiteral("Font size"), styleGroup));
        fontPointSizeSpin->setRange(6, 48);
        styleLayout->addWidget(fontPointSizeSpin);
        styleLayout->addWidget(new QLabel(QStringLiteral("Font families"), styleGroup));
        styleLayout->addWidget(fontFamiliesEdit);
        styleLayout->addStretch();

        // network group content
        auto *netLayout = makeVBox(networkGroup, 12, 8);
        netLayout->addWidget(new QLabel(QStringLiteral("Threads"), networkGroup));
        threadSpin->setRange(0, 128);
        netLayout->addWidget(threadSpin);
        proxyCheck->setText(QStringLiteral("Use system proxy"));
        netLayout->addWidget(proxyCheck);
        proxyEdit->setPlaceholderText(QStringLiteral("http://host:port or socks5://..."));
        proxyEdit->setVisible(false);
        netLayout->addWidget(proxyEdit);
        connect(proxyCheck, &QCheckBox::toggled, this, [this](bool checked) {
            proxyEdit->setVisible(!checked);
            emit proxyModeChanged(checked);
        });
        netLayout->addStretch();

        // other group
        auto *otherLayout = makeVBox(otherGroup, 12, 8);
        otherLayout->addWidget(new QLabel(QStringLiteral("Custom temp dir"), otherGroup));
        auto *otherRow = new QHBoxLayout();
        otherRow->setContentsMargins(0, 0, 0, 0);
        otherRow->setSpacing(8);
        customTempDirBrowseBtn->setText("...");
        customTempDirBrowseBtn->setToolTip(QStringLiteral("Browse"));
        otherRow->addWidget(customTempDirEdit, 1);
        otherRow->addWidget(customTempDirBrowseBtn, 0);
        otherLayout->addLayout(otherRow);
        otherLayout->addStretch();

        connect(customTempDirBrowseBtn, &QToolButton::clicked, this, [this]() {
            const QString startDir = customTempDirEdit->text().isEmpty() ? QDir::homePath() : customTempDirEdit->text();
            const QString dir = QFileDialog::getExistingDirectory(this, QStringLiteral("Select temp folder"), startDir);
            if (!dir.isEmpty()) {
                customTempDirEdit->setText(dir);
            }
        });

        // minecraft group
        auto *mcLayout = makeVBox(minecraftGroup, 12, 8);
        mcLayout->addWidget(new QLabel(QStringLiteral("Java path"), minecraftGroup));
        mcLayout->addWidget(javaPathEdit);
        mcLayout->addWidget(new QLabel(QStringLiteral("Download source"), minecraftGroup));
        mcLayout->addWidget(downloadSourceCombo);
        mcLayout->addWidget(new QLabel(QStringLiteral("Player name"), minecraftGroup));
        mcLayout->addWidget(playerNameEdit);
        mcLayout->addWidget(new QLabel(QStringLiteral("Custom resolution"), minecraftGroup));
        mcLayout->addWidget(customResolutionEdit);
        mcLayout->addWidget(new QLabel(QStringLiteral("Join server address"), minecraftGroup));
        mcLayout->addWidget(joinServerAddressEdit);
        mcLayout->addWidget(new QLabel(QStringLiteral("Join server port"), minecraftGroup));
        joinServerPortSpin->setRange(1, 65535);
        mcLayout->addWidget(joinServerPortSpin);
        mcLayout->addStretch();

        // advanced tab
        advancedScroll->setWidget(advancedTab);
        advancedScroll->setWidgetResizable(true);
        advancedScroll->setFrameShape(QFrame::NoFrame);
        advancedScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        advancedScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        auto *advLayout = makeVBox(advancedTab);
        advLayout->addWidget(devGroup);
        advLayout->addStretch();
        tabWidget->addTab(advancedScroll, QStringLiteral("Advanced"));

        auto *devLayout = makeVBox(devGroup, 12, 8);
        devEnableCheck->setText(QStringLiteral("Enable dev"));
        devDebugCheck->setText(QStringLiteral("Debug"));
        devTlsCheck->setText(QStringLiteral("TLS"));
        devLayout->addWidget(devEnableCheck);
        devLayout->addWidget(devDebugCheck);
        devLayout->addWidget(new QLabel(QStringLiteral("Server"), devGroup));
        devServerCheck->setText(QStringLiteral("Use default server"));
        devLayout->addWidget(devServerCheck);
        devServerEdit->setPlaceholderText(QStringLiteral("https://example.com"));
        devServerEdit->setVisible(false);
        devLayout->addWidget(devServerEdit);
        devLayout->addWidget(devTlsCheck);
        devLayout->addStretch();

        connect(devServerCheck, &QCheckBox::toggled, this, [this](bool checked) {
            devServerEdit->setVisible(!checked);
            emit devServerModeChanged(checked);
        });
    }

    void SettingPage::setupCombos() {
        backgroundTypeCombo->addItems({"image", "none"});
        blurEffectCombo->addItems({"performance", "quality", "animation"});
        launcherMethodCombo->addItems({"launchVisible", "launchHidden"});
        downloadSourceCombo->addItems({"Official", "BMCLAPI", "Mojang"});
    }

    void SettingPage::setupTheme(const Theme &theme) {
        applyGroupStyle(theme);
        const QString tabPaneBg = theme.colors.background.data();
        tabWidget->setStyleSheet(
            QString("QTabWidget::pane { border: 1px solid %1; background: %2; }"
                    "QTabWidget::tab-bar { left: 8px; }"
                    "QTabBar::tab { background: %3; color: %4; padding: 8px 14px; border-top-left-radius: 10px; border-top-right-radius: 10px; margin-right: 4px; }"
                    "QTabBar::tab:selected { background: %5; color: %4; }"
                    "QTabBar::tab:hover { background: %6; }"
                    "QScrollArea { background: transparent; border: none; }"
                    "QScrollArea QWidget { background: transparent; }"
                    "QLabel { color: %4; background: transparent; }")
                .arg(theme.colors.accent.data())
                .arg(tabPaneBg.data())
                .arg(theme.colors.surface.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.primary.data())
                .arg(theme.colors.hover.data()));

        const QString scrollStyle = QString(
                                         "QScrollBar:vertical { width: 10px; background: transparent; margin: 4px 0 4px 0; }"
                                         "QScrollBar::handle:vertical { background: %1; min-height: 30px; border-radius: 5px; }"
                                         "QScrollBar::handle:vertical:hover { background: %2; }"
                                         "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
                                         "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: transparent; }"
                                         "QScrollBar:horizontal { height: 10px; background: transparent; margin: 0 4px 0 4px; }"
                                         "QScrollBar::handle:horizontal { background: %1; min-width: 30px; border-radius: 5px; }"
                                         "QScrollBar::handle:horizontal:hover { background: %2; }"
                                         "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
                                         "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: transparent; }")
                                         .arg(theme.colors.disabled.data())
                                         .arg(theme.colors.focus.data());
        tabWidget->setStyleSheet(tabWidget->styleSheet() + scrollStyle);

        const QString editStyle = QString(
                                      "QLineEdit, QComboBox, QSpinBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 6px; }"
                                      "QLineEdit:focus, QComboBox:focus, QSpinBox:focus { border: 1px solid %4; }")
                                      .arg(theme.colors.surface.data())
                                      .arg(theme.colors.text.data())
                                      .arg(theme.colors.disabled.data())
                                      .arg(theme.colors.focus.data());
        for (auto *w : {static_cast<QWidget *>(backgroundPathEdit), static_cast<QWidget *>(windowSizeEdit),
                        static_cast<QWidget *>(themeEdit), static_cast<QWidget *>(fontFamiliesEdit),
                        static_cast<QWidget *>(devServerEdit), static_cast<QWidget *>(proxyEdit), static_cast<QWidget *>(customTempDirEdit),
                        static_cast<QWidget *>(javaPathEdit), static_cast<QWidget *>(playerNameEdit),
                        static_cast<QWidget *>(customResolutionEdit), static_cast<QWidget *>(joinServerAddressEdit)}) {
            w->setStyleSheet(editStyle);
        }
        for (auto *w : {static_cast<QWidget *>(backgroundTypeCombo), static_cast<QWidget *>(blurEffectCombo),
                        static_cast<QWidget *>(launcherMethodCombo), static_cast<QWidget *>(blurRadiusSpin),
                        static_cast<QWidget *>(fontPointSizeSpin), static_cast<QWidget *>(threadSpin),
                        static_cast<QWidget *>(downloadSourceCombo), static_cast<QWidget *>(joinServerPortSpin),
                        static_cast<QWidget *>(customTempDirBrowseBtn)}) {
            w->setStyleSheet(editStyle);
        }

        const QString checkStyle = QString(
                                       "QCheckBox { color: %1; background: transparent; }"
                                       "QCheckBox::indicator { width: 16px; height: 16px; border: 1px solid %2; border-radius: 4px; background: %3; }"
                                       "QCheckBox::indicator:checked { background: %4; border-color: %4; }"
                                       "QCheckBox::indicator:hover { border-color: %5; }")
                                       .arg(theme.colors.text.data())
                                       .arg(theme.colors.disabled.data())
                                       .arg(theme.colors.surface.data())
                                       .arg(theme.colors.accent.data())
                                       .arg(theme.colors.focus.data());
        for (auto *c : {useSysWindowFrameCheck, headBarKeepRightCheck, proxyCheck, devEnableCheck, devDebugCheck, devServerCheck, devTlsCheck}) {
            c->setStyleSheet(checkStyle);
        }

        const QString toolBtnStyle = QString(
                                         "QToolButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 6px 10px; }"
                                         "QToolButton:hover { background-color: %4; border-color: %3; }")
                                         .arg(theme.colors.surface.data())
                                         .arg(theme.colors.text.data())
                                         .arg(theme.colors.accent.data())
                                         .arg(theme.colors.hover.data());
        for (auto *tb : {customTempDirBrowseBtn, closeTabButton}) {
            tb->setStyleSheet(toolBtnStyle);
        }

        const QString btnStyle = QString(
                                     "QPushButton { background-color: %1; color: %2; border: none; border-radius: 10px; padding: 10px 14px; }"
                                     "QPushButton:hover { background-color: %3; }")
                                     .arg(theme.colors.primary.data())
                                     .arg(theme.colors.text.data())
                                     .arg(theme.colors.hover.data());
        authButton->setStyleSheet(btnStyle);
    }

    void SettingPage::applyGroupStyle(const Theme &theme) {
        const QString groupStyle = QString(
                                       "QGroupBox { background-color: %1; border: 1px solid %2; border-radius: 12px; margin-top: 12px; color: %3; padding: 8px; }"
                                       "QGroupBox:title { subcontrol-origin: margin; left: 12px; padding: 0 4px; }")
                                       .arg(theme.colors.surface.data())
                                       .arg(theme.colors.accent.data())
                                       .arg(theme.colors.text.data());
        for (auto *g : {mainGroup, styleGroup, networkGroup, otherGroup, minecraftGroup, devGroup}) {
            g->setStyleSheet(groupStyle);
        }
    }

    void SettingPage::setupFont(QFont text, QFont h1Font, QFont h2Font) {
        Q_UNUSED(h1Font);
        Q_UNUSED(h2Font);

        for (auto w : std::initializer_list<QWidget *>{authStatusLabel, authButton, backgroundPathEdit, windowSizeEdit, themeEdit, fontFamiliesEdit, devServerEdit}) {
            w->setFont(text);
        }
        for (auto w : std::initializer_list<QWidget *>{backgroundTypeCombo, blurEffectCombo, launcherMethodCombo, blurRadiusSpin, fontPointSizeSpin, threadSpin}) {
            w->setFont(text);
        }
        for (auto c : std::initializer_list<QWidget *>{useSysWindowFrameCheck, headBarKeepRightCheck, proxyCheck, devEnableCheck, devDebugCheck, devServerCheck, devTlsCheck}) {
            c->setFont(text);
        }
        for (auto w : std::initializer_list<QWidget *>{customTempDirEdit, customTempDirBrowseBtn, closeTabButton, proxyEdit, javaPathEdit, downloadSourceCombo, playerNameEdit, customResolutionEdit, joinServerAddressEdit, joinServerPortSpin}) {
            w->setFont(text);
        }
        mainGroup->setFont(text);
        styleGroup->setFont(text);
        networkGroup->setFont(text);
        otherGroup->setFont(text);
        minecraftGroup->setFont(text);
        devGroup->setFont(text);
        tabWidget->setFont(text);
    }

    void SettingPage::resizeItems(int windowWidth, int windowHeight) {
        setGeometry(0, 0, windowWidth, windowHeight);
        tabWidget->setGeometry(0, 0, windowWidth, windowHeight);
    }

    void SettingPage::settingFromConfig(const neko::ClientConfig &cfg) {

        backgroundTypeCombo->setCurrentText(QString::fromStdString(cfg.main.backgroundType));
        backgroundPathEdit->setText(QString::fromStdString(cfg.main.background));
        windowSizeEdit->setText(QString::fromStdString(cfg.main.windowSize));
        launcherMethodCombo->setCurrentText(QString::fromStdString(cfg.main.launcherMethod));
        useSysWindowFrameCheck->setChecked(cfg.main.useSysWindowFrame);
        headBarKeepRightCheck->setChecked(cfg.main.headBarKeepRight);

        themeEdit->setText(QString::fromStdString(cfg.style.theme));
        blurEffectCombo->setCurrentText(QString::fromStdString(cfg.style.blurEffect));
        blurRadiusSpin->setValue(static_cast<int>(cfg.style.blurRadius));
        fontPointSizeSpin->setValue(static_cast<int>(cfg.style.fontPointSize));
        fontFamiliesEdit->setText(QString::fromStdString(cfg.style.fontFamilies));

        threadSpin->setValue(static_cast<int>(cfg.net.thread));
        const bool useSystemProxy = (cfg.net.proxy == neko::strview("true"));
        proxyCheck->setChecked(useSystemProxy);
        proxyEdit->setText(useSystemProxy ? QString() : QString::fromStdString(cfg.net.proxy));
        proxyEdit->setVisible(!useSystemProxy);

        customTempDirEdit->setText(QString::fromStdString(cfg.other.tempFolder));

        javaPathEdit->setText(QString::fromStdString(cfg.minecraft.javaPath));
        downloadSourceCombo->setCurrentText(QString::fromStdString(cfg.minecraft.downloadSource));
        playerNameEdit->setText(QString::fromStdString(cfg.minecraft.playerName));
        customResolutionEdit->setText(QString::fromStdString(cfg.minecraft.customResolution));
        joinServerAddressEdit->setText(QString::fromStdString(cfg.minecraft.joinServerAddress));
        bool okPort = false;
        int portVal = QString::fromStdString(cfg.minecraft.joinServerPort).toInt(&okPort);
        if (okPort) {
            joinServerPortSpin->setValue(portVal);
        }

        devEnableCheck->setChecked(cfg.dev.enable);
        devDebugCheck->setChecked(cfg.dev.debug);
        const bool useDefaultDevServer = (cfg.dev.server == neko::strview("auto"));
        devServerCheck->setChecked(useDefaultDevServer);
        devServerEdit->setText(useDefaultDevServer ? QString() : QString::fromStdString(cfg.dev.server));
        devServerEdit->setVisible(!useDefaultDevServer);
        devTlsCheck->setChecked(cfg.dev.tls);
    }

    void SettingPage::setAuthState(bool loggedIn, const std::string &statusText) {
        authStatusLabel->setText(QString::fromStdString(statusText));
        if (loggedIn) {
            authButton->setText(QStringLiteral("Logout"));
        } else {
            authButton->setText(QStringLiteral("Login"));
        }
    }

    void SettingPage::writeToConfig(ClientConfig &cfg) const {

            const QString bgType = backgroundTypeCombo->currentText();
            auto bgTypeStd = bgType.toStdString();
            cfg.main.backgroundType = std::move(bgTypeStd);
            const QString bgPath = backgroundPathEdit->text();
            auto bgStd = bgPath.toStdString();
            cfg.main.background = std::move(bgStd);
            const QString ws = windowSizeEdit->text();
            auto wsStd = ws.toStdString();
            cfg.main.windowSize = std::move(wsStd);
            const QString lm = launcherMethodCombo->currentText();
            auto lmStd = lm.toStdString();
            cfg.main.launcherMethod = std::move(lmStd);
            cfg.main.useSysWindowFrame = useSysWindowFrameCheck->isChecked();
            cfg.main.headBarKeepRight = headBarKeepRightCheck->isChecked();
            const QString th = themeEdit->text();
            auto thStd = th.toStdString();
            cfg.style.theme = std::move(thStd);
            const QString be = blurEffectCombo->currentText();
            auto beStd = be.toStdString();
            cfg.style.blurEffect = std::move(beStd);
            cfg.style.blurRadius = blurRadiusSpin->value();
            cfg.style.fontPointSize = fontPointSizeSpin->value();
            const QString ff = fontFamiliesEdit->text();
            auto ffStd = ff.toStdString();
            cfg.style.fontFamilies = std::move(ffStd);

            std::string().swap(cfg.net.proxy); // free buffer completely

            cfg.net.thread = threadSpin->value();

            if (proxyCheck->isChecked()) {
                cfg.net.proxy = "true";
            } else {
                const QString pr = proxyEdit->text();
                auto prStd = pr.toStdString();
                cfg.net.proxy = std::move(prStd);
            }

            std::string().swap(cfg.other.tempFolder); // free buffer completely

            const QString tp = customTempDirEdit->text();

            auto tpStd = tp.toStdString();
            cfg.other.tempFolder = std::move(tpStd);

            const QString jp = javaPathEdit->text();

            auto jpStd = jp.toStdString();

            cfg.minecraft.javaPath = std::move(jpStd);

            const QString ds = downloadSourceCombo->currentText();
            auto dsStd = ds.toStdString();
            cfg.minecraft.downloadSource = std::move(dsStd);

            const QString pn = playerNameEdit->text();

            auto pnStd = pn.toStdString();
            cfg.minecraft.playerName = std::move(pnStd);
            const QString cr = customResolutionEdit->text();
            auto crStd = cr.toStdString();
            cfg.minecraft.customResolution = std::move(crStd);
            const QString js = joinServerAddressEdit->text();

            auto jsStd = js.toStdString();

            cfg.minecraft.joinServerAddress = std::move(jsStd);
            cfg.minecraft.joinServerPort = std::to_string(joinServerPortSpin->value());

            cfg.dev.enable = devEnableCheck->isChecked();

            cfg.dev.debug = devDebugCheck->isChecked();
            std::string serverVal;
            if (devServerCheck->isChecked()) {
                serverVal = "auto";
            } else {
                const QString ds = devServerEdit->text();
                serverVal = ds.toStdString();
            }
            cfg.dev.server = std::move(serverVal);

            cfg.dev.tls = devTlsCheck->isChecked();
    }

} // namespace neko::ui::page
