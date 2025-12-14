#include "neko/ui/pages/settingPage.hpp"

#include "neko/app/lang.hpp"
#include "neko/app/nekoLc.hpp"
#include "neko/bus/configBus.hpp"
#include "neko/ui/dialogs/themeEditorDialog.hpp"
#include "neko/ui/themeIO.hpp"

#include "neko/core/auth.hpp"

#include <neko/log/nlog.hpp>

#include <QtCore/QDir>
#include <QtCore/QSignalBlocker>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
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
                    themeDir(lc::ThemeFolderName.data()),
          authScroll(new QScrollArea(tabWidget)),
          authTab(new QWidget()),
          authStatusLabel(new QLabel(authTab)),
          authButton(new QPushButton(authTab)),
          mainScroll(new QScrollArea(tabWidget)),
          mainTab(new QWidget()),
          mainGroup(new QGroupBox(QStringLiteral("Main"), mainTab)),
          languageCombo(new QComboBox(mainGroup)),
          backgroundTypeCombo(new QComboBox(mainGroup)),
          backgroundPathEdit(new QLineEdit(mainGroup)),
          backgroundBrowseBtn(new QToolButton(mainGroup)),
          windowSizeEdit(new QLineEdit(mainGroup)),
          launcherMethodCombo(new QComboBox(mainGroup)),
          styleGroup(new QGroupBox(QStringLiteral("Style"), mainTab)),
          themeCombo(new QComboBox(styleGroup)),
          editThemeBtn(new QPushButton(QStringLiteral("Edit"), styleGroup)),
          blurEffectCombo(new QComboBox(styleGroup)),
          blurRadiusSlider(new QSlider(Qt::Horizontal, styleGroup)),
          fontPointSizeSpin(new QSpinBox(styleGroup)),
          fontFamiliesCombo(new QFontComboBox(styleGroup)),
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
          javaPathBrowseBtn(new QToolButton(minecraftGroup)),
          downloadSourceCombo(new QComboBox(minecraftGroup)),
          customResolutionEdit(new QLineEdit(minecraftGroup)),
          joinServerAddressEdit(new QLineEdit(minecraftGroup)),
          joinServerPortSpin(new QSpinBox(minecraftGroup)),
          advancedScroll(new QScrollArea(tabWidget)),
          advancedTab(new QWidget()),
          devGroup(new QGroupBox(QStringLiteral("Advanced"), advancedTab)),
          devEnableCheck(new QCheckBox(devGroup)),
          devDebugCheck(new QCheckBox(devGroup)),
          devLogViewerCheck(new QCheckBox(devGroup)),
          devServerCheck(new QCheckBox(devGroup)),
          devServerEdit(new QLineEdit(devGroup)),
          devTlsCheck(new QCheckBox(devGroup)),
          devPreviewLabel(new QLabel(devGroup)),
          devShowNoticeBtn(new QPushButton(devGroup)),
          devShowInputBtn(new QPushButton(devGroup)),
          devShowLoadingBtn(new QPushButton(devGroup)) {
        this->setAttribute(Qt::WA_TranslucentBackground);
        buildUi();
        setupCombos();
        retranslateUi();
        setAuthState(false, {});
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
        connect(authButton, &QPushButton::clicked, this, [this]() {
            if (core::auth::isLoggedIn()) {
                emit logoutRequested();
            } else {
                emit loginRequested();
            }
        });
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
        auto *languageLabel = new QLabel(mainGroup);
        languageLabel->setObjectName(QStringLiteral("languageLabel"));
        mainGroupLayout->addWidget(languageLabel);
        mainGroupLayout->addWidget(languageCombo);
        auto *backgroundTypeLabel = new QLabel(mainGroup);
        backgroundTypeLabel->setObjectName(QStringLiteral("backgroundTypeLabel"));
        mainGroupLayout->addWidget(backgroundTypeLabel);
        mainGroupLayout->addWidget(backgroundTypeCombo);
        auto *backgroundLabel = new QLabel(mainGroup);
        backgroundLabel->setObjectName(QStringLiteral("backgroundLabel"));
        mainGroupLayout->addWidget(backgroundLabel);
        auto *backgroundRow = new QHBoxLayout();
        backgroundRow->setContentsMargins(0, 0, 0, 0);
        backgroundRow->setSpacing(8);
        backgroundBrowseBtn->setText("...");
        backgroundRow->addWidget(backgroundPathEdit, 1);
        backgroundRow->addWidget(backgroundBrowseBtn, 0);
        mainGroupLayout->addLayout(backgroundRow);
        auto *windowSizeLabel = new QLabel(mainGroup);
        windowSizeLabel->setObjectName(QStringLiteral("windowSizeLabel"));
        mainGroupLayout->addWidget(windowSizeLabel);
        mainGroupLayout->addWidget(windowSizeEdit);
        auto *launcherMethodLabel = new QLabel(mainGroup);
        launcherMethodLabel->setObjectName(QStringLiteral("launcherMethodLabel"));
        mainGroupLayout->addWidget(launcherMethodLabel);
        mainGroupLayout->addWidget(launcherMethodCombo);
        mainGroupLayout->addStretch();

        // style group content
        auto *styleLayout = makeVBox(styleGroup, 12, 8);
        auto *themeLabel = new QLabel(styleGroup);
        themeLabel->setObjectName(QStringLiteral("themeLabel"));
        styleLayout->addWidget(themeLabel);
        auto *themeRow = new QHBoxLayout();
        themeRow->setContentsMargins(0, 0, 0, 0);
        themeRow->setSpacing(8);
        themeRow->addWidget(themeCombo, 1);
        editThemeBtn->setObjectName(QStringLiteral("editThemeBtn"));
        editThemeBtn->setFixedWidth(72);
        themeRow->addWidget(editThemeBtn, 0);
        styleLayout->addLayout(themeRow);
        auto *blurEffectLabel = new QLabel(styleGroup);
        blurEffectLabel->setObjectName(QStringLiteral("blurEffectLabel"));
        styleLayout->addWidget(blurEffectLabel);
        styleLayout->addWidget(blurEffectCombo);
        auto *blurRadiusLabel = new QLabel(styleGroup);
        blurRadiusLabel->setObjectName(QStringLiteral("blurRadiusLabel"));
        styleLayout->addWidget(blurRadiusLabel);
        blurRadiusSlider->setRange(0, 255);
        blurRadiusSlider->setSingleStep(1);
        blurRadiusSlider->setPageStep(8);
        styleLayout->addWidget(blurRadiusSlider);
        auto *fontSizeLabel = new QLabel(styleGroup);
        fontSizeLabel->setObjectName(QStringLiteral("fontSizeLabel"));
        styleLayout->addWidget(fontSizeLabel);
        fontPointSizeSpin->setRange(6, 48);
        styleLayout->addWidget(fontPointSizeSpin);
        auto *fontFamiliesLabel = new QLabel(styleGroup);
        fontFamiliesLabel->setObjectName(QStringLiteral("fontFamiliesLabel"));
        styleLayout->addWidget(fontFamiliesLabel);
        fontFamiliesCombo->setEditable(true);
        styleLayout->addWidget(fontFamiliesCombo);
        styleLayout->addStretch();

        // network group content
        auto *netLayout = makeVBox(networkGroup, 12, 8);
        auto *threadsLabel = new QLabel(networkGroup);
        threadsLabel->setObjectName(QStringLiteral("threadsLabel"));
        netLayout->addWidget(threadsLabel);
        threadSpin->setRange(0, 128);
        netLayout->addWidget(threadSpin);
        proxyCheck->setObjectName(QStringLiteral("proxyCheck"));
        netLayout->addWidget(proxyCheck);
        proxyEdit->setObjectName(QStringLiteral("proxyEdit"));
        proxyEdit->setVisible(false);
        netLayout->addWidget(proxyEdit);
        connect(proxyCheck, &QCheckBox::toggled, this, [this](bool checked) {
            proxyEdit->setVisible(!checked);
            if (suppressSignals) {
                return;
            }
            emit proxyModeChanged(checked);
            emit proxyValueChanged(checked, proxyEdit->text());
            emit configChanged();
        });
        netLayout->addStretch();

        // other group
        auto *otherLayout = makeVBox(otherGroup, 12, 8);
        auto *customTempDirLabel = new QLabel(otherGroup);
        customTempDirLabel->setObjectName(QStringLiteral("customTempDirLabel"));
        otherLayout->addWidget(customTempDirLabel);
        auto *otherRow = new QHBoxLayout();
        otherRow->setContentsMargins(0, 0, 0, 0);
        otherRow->setSpacing(8);
        customTempDirBrowseBtn->setText("...");
        otherRow->addWidget(customTempDirEdit, 1);
        otherRow->addWidget(customTempDirBrowseBtn, 0);
        otherLayout->addLayout(otherRow);
        otherLayout->addStretch();

        connect(customTempDirBrowseBtn, &QToolButton::clicked, this, [this]() {
            const QString startDir = customTempDirEdit->text().isEmpty() ? QDir::homePath() : customTempDirEdit->text();
            const QString dir = QFileDialog::getExistingDirectory(
                this,
                QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::selectTempDir, "Select temp folder")),
                startDir);
            if (!dir.isEmpty()) {
                customTempDirEdit->setText(dir);
            }
        });

        connect(backgroundBrowseBtn, &QToolButton::clicked, this, [this]() {
            const QString startDir = backgroundPathEdit->text().isEmpty() ? QDir::homePath() : QFileInfo(backgroundPathEdit->text()).absolutePath();
            const QString file = QFileDialog::getOpenFileName(
                this,
                QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::selectBackground, "Select background image")),
                startDir,
                QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::imageFileFilter, "Images (*.png *.jpg *.jpeg *.bmp *.gif);;All Files (*.*)")));
            if (!file.isEmpty()) {
                backgroundPathEdit->setText(file);
            }
        });

        connect(javaPathBrowseBtn, &QToolButton::clicked, this, [this]() {
            const QString startDir = javaPathEdit->text().isEmpty() ? QDir::homePath() : javaPathEdit->text();
            const QString file = QFileDialog::getOpenFileName(
                this,
                QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::browseJava, "Select Java executable")),
                startDir,
                QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::javaExecutableFilter, "Executables (*.exe);;All Files (*.*)")));
            if (!file.isEmpty()) {
                javaPathEdit->setText(file);
            }
        });

        // minecraft group
        auto *mcLayout = makeVBox(minecraftGroup, 12, 8);
        auto *javaPathLabel = new QLabel(minecraftGroup);
        javaPathLabel->setObjectName(QStringLiteral("javaPathLabel"));
        mcLayout->addWidget(javaPathLabel);
        auto *javaRow = new QHBoxLayout();
        javaRow->setContentsMargins(0, 0, 0, 0);
        javaRow->setSpacing(8);
        javaPathBrowseBtn->setText("...");
        javaRow->addWidget(javaPathEdit, 1);
        javaRow->addWidget(javaPathBrowseBtn, 0);
        mcLayout->addLayout(javaRow);
        auto *downloadSourceLabel = new QLabel(minecraftGroup);
        downloadSourceLabel->setObjectName(QStringLiteral("downloadSourceLabel"));
        mcLayout->addWidget(downloadSourceLabel);
        mcLayout->addWidget(downloadSourceCombo);
        auto *customResolutionLabel = new QLabel(minecraftGroup);
        customResolutionLabel->setObjectName(QStringLiteral("customResolutionLabel"));
        mcLayout->addWidget(customResolutionLabel);
        mcLayout->addWidget(customResolutionEdit);
        auto *joinServerAddressLabel = new QLabel(minecraftGroup);
        joinServerAddressLabel->setObjectName(QStringLiteral("joinServerAddressLabel"));
        mcLayout->addWidget(joinServerAddressLabel);
        mcLayout->addWidget(joinServerAddressEdit);
        auto *joinServerPortLabel = new QLabel(minecraftGroup);
        joinServerPortLabel->setObjectName(QStringLiteral("joinServerPortLabel"));
        mcLayout->addWidget(joinServerPortLabel);
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
        devEnableCheck->setObjectName(QStringLiteral("devEnableCheck"));
        devDebugCheck->setObjectName(QStringLiteral("devDebugCheck"));
        devLogViewerCheck->setObjectName(QStringLiteral("devLogViewerCheck"));
        devTlsCheck->setObjectName(QStringLiteral("devTlsCheck"));
        devLayout->addWidget(devEnableCheck);
        devLayout->addWidget(devDebugCheck);
        devLayout->addWidget(devLogViewerCheck);
        auto *devServerLabel = new QLabel(devGroup);
        devServerLabel->setObjectName(QStringLiteral("devServerLabel"));
        devLayout->addWidget(devServerLabel);
        devServerCheck->setObjectName(QStringLiteral("devServerCheck"));
        devLayout->addWidget(devServerCheck);
        devServerEdit->setObjectName(QStringLiteral("devServerEdit"));
        devServerEdit->setVisible(false);
        devLayout->addWidget(devServerEdit);
        devLayout->addWidget(devTlsCheck);
        devPreviewLabel->setObjectName(QStringLiteral("devPreviewLabel"));
        devLayout->addWidget(devPreviewLabel);
        auto *devPreviewRow = new QHBoxLayout();
        devPreviewRow->setContentsMargins(0, 0, 0, 0);
        devPreviewRow->setSpacing(8);
        devPreviewRow->addWidget(devShowNoticeBtn);
        devPreviewRow->addWidget(devShowInputBtn);
        devPreviewRow->addWidget(devShowLoadingBtn);
        devLayout->addLayout(devPreviewRow);
        devLayout->addStretch();

        connect(devServerCheck, &QCheckBox::toggled, this, [this](bool checked) {
            devServerEdit->setVisible(!checked);
            if (suppressSignals) {
                return;
            }
            emit devServerModeChanged(checked);
            emit configChanged();
        });
        connect(devShowNoticeBtn, &QPushButton::clicked, this, [this]() {
            emit showNoticePreviewRequested();
        });
        connect(devShowInputBtn, &QPushButton::clicked, this, [this]() {
            emit showInputPreviewRequested();
        });
        connect(devShowLoadingBtn, &QPushButton::clicked, this, [this]() {
            emit showLoadingPreviewRequested();
        });

        // Live update signals
        connect(languageCombo, &QComboBox::currentIndexChanged, this, [this](int index) {
            const QString langCode = languageCombo->itemData(index).toString();
            if (langCode.isEmpty()) {
                return;
            }
            lang::language(langCode.toStdString());
            retranslateUi();
            if (suppressSignals) {
                return;
            }
            emit languageChanged(langCode);
            emit configChanged();
        });
        connect(themeCombo, &QComboBox::currentIndexChanged, this, [this](int) {
            if (suppressSignals) {
                return;
            }
            updateEditThemeState();
            emit themeChanged(themeCombo->currentData().toString());
            emit configChanged();
        });
        connect(editThemeBtn, &QPushButton::clicked, this, [this]() {
            const std::string name = themeCombo->currentText().toStdString();
            Theme base = themeio::loadThemeByName(name, themeDir).value_or(lightTheme);
            dialog::ThemeEditorDialog dlg(base, this);
            if (dlg.exec() != QDialog::Accepted) {
                return;
            }
            Theme edited = dlg.getEditedTheme();
            if (edited.info.name.empty()) {
                edited.info.name = name.empty() ? std::string("Custom") : name;
            }
            std::string err;
            if (!themeio::saveTheme(edited, themeDir, err)) {
                log::warn("Failed to save theme: {}", {}, err);
                return;
            }
            refreshThemeList();
            const QString editedName = QString::fromStdString(edited.info.name);
            const int idx = themeCombo->findData(editedName);
            if (idx >= 0) {
                themeCombo->setCurrentIndex(idx);
            }
            emit themeChanged(editedName);
            emit configChanged();
        });
        connect(fontPointSizeSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
            if (suppressSignals) {
                return;
            }
            emit fontPointSizeChanged(value);
            emit configChanged();
        });
        connect(fontFamiliesCombo, &QFontComboBox::currentFontChanged, this, [this](const QFont &f) {
            if (suppressSignals) {
                return;
            }
            emit fontFamiliesChanged(f.family());
            emit configChanged();
        });
        connect(blurEffectCombo, &QComboBox::currentIndexChanged, this, [this](int) {
            if (suppressSignals) {
                return;
            }
            emit blurEffectChanged(blurEffectCombo->currentData().toString());
            emit configChanged();
        });
        connect(blurRadiusSlider, &QSlider::valueChanged, this, [this](int value) {
            if (value == 1) {
                return; // skip problematic radius
            }
            if (suppressSignals) {
                return;
            }
            emit blurRadiusChanged(value);
            emit configChanged();
        });
        connect(backgroundTypeCombo, &QComboBox::currentIndexChanged, this, [this](int) {
            if (suppressSignals) {
                return;
            }
            emit backgroundTypeChanged(backgroundTypeCombo->currentData().toString());
            emit configChanged();
        });
        connect(backgroundPathEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit backgroundPathChanged(text);
            emit configChanged();
        });
        connect(windowSizeEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit windowSizeEdited(text);
        });
        connect(windowSizeEdit, &QLineEdit::returnPressed, this, [this]() {
            if (suppressSignals) {
                return;
            }
            emit windowSizeApplyRequested(windowSizeEdit->text());
        });
        connect(launcherMethodCombo, &QComboBox::currentIndexChanged, this, [this](int) {
            if (suppressSignals) {
                return;
            }
            emit launcherMethodChanged(launcherMethodCombo->currentData().toString());
            emit configChanged();
        });
        connect(javaPathEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit javaPathChanged(text);
            emit configChanged();
        });
        connect(downloadSourceCombo, &QComboBox::currentIndexChanged, this, [this](int) {
            if (suppressSignals) {
                return;
            }
            emit downloadSourceChanged(downloadSourceCombo->currentData().toString());
            emit configChanged();
        });
        connect(customResolutionEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit customResolutionChanged(text);
            emit configChanged();
        });
        connect(joinServerAddressEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit joinServerAddressChanged(text);
            emit configChanged();
        });
        connect(joinServerPortSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
            if (suppressSignals) {
                return;
            }
            emit joinServerPortChanged(value);
            emit configChanged();
        });
        connect(customTempDirEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit customTempDirChanged(text);
            emit configChanged();
        });
        connect(threadSpin, qOverload<int>(&QSpinBox::valueChanged), this, [this](int value) {
            if (suppressSignals) {
                return;
            }
            emit threadCountChanged(value);
            emit configChanged();
        });
        connect(proxyEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
            if (suppressSignals) {
                return;
            }
            emit proxyValueChanged(proxyCheck->isChecked(), text);
            emit configChanged();
        });
        connect(devEnableCheck, &QCheckBox::toggled, this, [this](bool) {
            if (suppressSignals) {
                return;
            }
            emit configChanged();
        });
        connect(devDebugCheck, &QCheckBox::toggled, this, [this](bool) {
            if (suppressSignals) {
                return;
            }
            emit configChanged();
        });
        connect(devLogViewerCheck, &QCheckBox::toggled, this, [this](bool) {
            if (suppressSignals) {
                return;
            }
            emit configChanged();
        });
        connect(devTlsCheck, &QCheckBox::toggled, this, [this](bool) {
            if (suppressSignals) {
                return;
            }
            emit configChanged();
        });
    }

    QString SettingPage::getBackgroundPath() const {
        return backgroundPathEdit->text();
    }

    void SettingPage::setupCombos() {
        const auto tr = [](neko::cstr category, neko::cstr key, const char *fallback) {
            return QString::fromStdString(lang::tr(category, key, fallback));
        };

        backgroundTypeCombo->clear();
        backgroundTypeCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::backgroundTypeImage, "Image background"), QStringLiteral("image"));
        backgroundTypeCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::backgroundTypeNone, "No background"), QStringLiteral("none"));

        blurEffectCombo->clear();
        blurEffectCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::blurEffectPerformance, "Performance"), QStringLiteral("performance"));
        blurEffectCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::blurEffectQuality, "Quality"), QStringLiteral("quality"));
        blurEffectCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::blurEffectAnimation, "Animation"), QStringLiteral("animation"));

        launcherMethodCombo->clear();
        launcherMethodCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::launcherVisible, "Keep launcher visible"), QStringLiteral("launchVisible"));
        launcherMethodCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::launcherExit, "Exit after launch"), QStringLiteral("launchExit"));
        launcherMethodCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::launcherHideRestore, "Hide launcher and restore when done"), QStringLiteral("launchHideRestore"));

        downloadSourceCombo->clear();
        downloadSourceCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::downloadSourceOfficial, "Official"), QStringLiteral("Official"));
        downloadSourceCombo->addItem(tr(lang::keys::setting::category, lang::keys::setting::downloadSourceBmclapi, "BMCLAPI"), QStringLiteral("BMCLAPI"));
        refreshThemeList();

        languageCombo->clear();
        try {
            const auto languages = lang::getLanguages();
            if (!languages.empty()) {
                for (const auto &[code, name] : languages) {
                    languageCombo->addItem(QString::fromStdString(name), QString::fromStdString(code));
                }
            }
        } catch (const std::exception &e) {
            log::warn("Failed to load languages: {}", {}, e.what());
        }
        if (languageCombo->count() == 0) {
            languageCombo->addItem(QStringLiteral("English"), QStringLiteral("en"));
        }
        const QString currentLang = QString::fromStdString(lang::language());
        const int langIndex = languageCombo->findData(currentLang);
        if (langIndex >= 0) {
            languageCombo->setCurrentIndex(langIndex);
        }

        // Theme list uses data roles for stable values; refresh to apply translations.
        refreshThemeList();

        fontFamiliesCombo->setWritingSystem(QFontDatabase::Any);
        fontFamiliesCombo->setMaxVisibleItems(8);
    }

    void SettingPage::retranslateUi() {
        const auto tr = [](neko::cstr category, neko::cstr key, const char *fallback) {
            return QString::fromStdString(lang::tr(category, key, fallback));
        };

        tabWidget->setTabText(tabWidget->indexOf(authScroll), tr(lang::keys::setting::category, lang::keys::setting::tabAccount, "Account"));
        tabWidget->setTabText(tabWidget->indexOf(mainScroll), tr(lang::keys::setting::category, lang::keys::setting::tabMain, "Main"));
        tabWidget->setTabText(tabWidget->indexOf(advancedScroll), tr(lang::keys::setting::category, lang::keys::setting::tabAdvanced, "Advanced"));

        mainGroup->setTitle(tr(lang::keys::setting::category, lang::keys::setting::groupMain, "Main"));
        styleGroup->setTitle(tr(lang::keys::setting::category, lang::keys::setting::groupStyle, "Style"));
        networkGroup->setTitle(tr(lang::keys::setting::category, lang::keys::setting::groupNetwork, "Network"));
        otherGroup->setTitle(tr(lang::keys::setting::category, lang::keys::setting::groupOther, "Other"));
        minecraftGroup->setTitle(tr(lang::keys::setting::category, lang::keys::setting::groupMinecraft, "Minecraft"));
        devGroup->setTitle(tr(lang::keys::setting::category, lang::keys::setting::groupAdvanced, "Advanced"));

        if (auto *label = mainGroup->findChild<QLabel *>(QStringLiteral("languageLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::language, "Language"));
        }
        if (auto *label = mainGroup->findChild<QLabel *>(QStringLiteral("backgroundTypeLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::backgroundType, "Background Type"));
        }
        {
            const int idxImg = backgroundTypeCombo->findData(QStringLiteral("image"));
            if (idxImg >= 0) {
                backgroundTypeCombo->setItemText(idxImg, tr(lang::keys::setting::category, lang::keys::setting::backgroundTypeImage, "Image background"));
            }
            const int idxNone = backgroundTypeCombo->findData(QStringLiteral("none"));
            if (idxNone >= 0) {
                backgroundTypeCombo->setItemText(idxNone, tr(lang::keys::setting::category, lang::keys::setting::backgroundTypeNone, "No background"));
            }
        }
        if (auto *label = mainGroup->findChild<QLabel *>(QStringLiteral("backgroundLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::background, "Background"));
        }
        backgroundBrowseBtn->setToolTip(tr(lang::keys::setting::category, lang::keys::setting::selectBackground, "Select background image"));
        if (auto *label = mainGroup->findChild<QLabel *>(QStringLiteral("windowSizeLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::windowSize, "Window Size"));
        }
        if (auto *label = mainGroup->findChild<QLabel *>(QStringLiteral("launcherMethodLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::launcherMethod, "Launcher Method"));
        }
        {
            const int idxVis = launcherMethodCombo->findData(QStringLiteral("launchVisible"));
            if (idxVis >= 0) {
                launcherMethodCombo->setItemText(idxVis, tr(lang::keys::setting::category, lang::keys::setting::launcherVisible, "Keep launcher visible"));
            }
            const int idxExit = launcherMethodCombo->findData(QStringLiteral("launchExit"));
            if (idxExit >= 0) {
                launcherMethodCombo->setItemText(idxExit, tr(lang::keys::setting::category, lang::keys::setting::launcherExit, "Exit after launch"));
            }
            const int idxHide = launcherMethodCombo->findData(QStringLiteral("launchHideRestore"));
            if (idxHide >= 0) {
                launcherMethodCombo->setItemText(idxHide, tr(lang::keys::setting::category, lang::keys::setting::launcherHideRestore, "Hide launcher and restore when done"));
            }
        }

        if (auto *label = styleGroup->findChild<QLabel *>(QStringLiteral("themeLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::theme, "Theme"));
        }
        editThemeBtn->setText(tr(lang::keys::button::category, lang::keys::button::edit, "Edit"));
        if (auto *label = styleGroup->findChild<QLabel *>(QStringLiteral("blurEffectLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::blurEffect, "Blur effect"));
        }
            {
                const int idxPerf = blurEffectCombo->findData(QStringLiteral("performance"));
                if (idxPerf >= 0) {
                    blurEffectCombo->setItemText(idxPerf, tr(lang::keys::setting::category, lang::keys::setting::blurEffectPerformance, "Performance"));
                }
                const int idxQual = blurEffectCombo->findData(QStringLiteral("quality"));
                if (idxQual >= 0) {
                    blurEffectCombo->setItemText(idxQual, tr(lang::keys::setting::category, lang::keys::setting::blurEffectQuality, "Quality"));
                }
                const int idxAnim = blurEffectCombo->findData(QStringLiteral("animation"));
                if (idxAnim >= 0) {
                    blurEffectCombo->setItemText(idxAnim, tr(lang::keys::setting::category, lang::keys::setting::blurEffectAnimation, "Animation"));
                }
            }
        if (auto *label = styleGroup->findChild<QLabel *>(QStringLiteral("blurRadiusLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::blurRadius, "Blur radius"));
        }
        if (auto *label = styleGroup->findChild<QLabel *>(QStringLiteral("fontSizeLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::fontSize, "Font size"));
        }
        if (auto *label = styleGroup->findChild<QLabel *>(QStringLiteral("fontFamiliesLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::fontFamilies, "Font families"));
        }

        if (auto *label = networkGroup->findChild<QLabel *>(QStringLiteral("threadsLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::threads, "Threads"));
        }
        proxyCheck->setText(tr(lang::keys::setting::category, lang::keys::setting::useSystemProxy, "Use system proxy"));
        proxyEdit->setPlaceholderText(tr(lang::keys::setting::category, lang::keys::setting::proxyPlaceholder, "http://host:port or socks5://..."));

        if (auto *label = otherGroup->findChild<QLabel *>(QStringLiteral("customTempDirLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::customTempDir, "Custom temp dir"));
        }

        if (auto *label = minecraftGroup->findChild<QLabel *>(QStringLiteral("javaPathLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::javaPath, "Java path"));
        }
        javaPathBrowseBtn->setToolTip(tr(lang::keys::setting::category, lang::keys::setting::browseJava, "Browse for Java executable"));
        if (auto *label = minecraftGroup->findChild<QLabel *>(QStringLiteral("downloadSourceLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::downloadSource, "Download source"));
        }
        if (auto *label = minecraftGroup->findChild<QLabel *>(QStringLiteral("customResolutionLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::customResolution, "Custom resolution"));
        }
        if (auto *label = minecraftGroup->findChild<QLabel *>(QStringLiteral("joinServerAddressLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::joinServerAddress, "Join server address"));
        }
        if (auto *label = minecraftGroup->findChild<QLabel *>(QStringLiteral("joinServerPortLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::joinServerPort, "Join server port"));
        }

        devEnableCheck->setText(tr(lang::keys::setting::category, lang::keys::setting::devEnable, "Enable dev"));
        devDebugCheck->setText(tr(lang::keys::setting::category, lang::keys::setting::devDebug, "Debug"));
        devLogViewerCheck->setText(tr(lang::keys::setting::category, lang::keys::setting::devShowLogViewer, "Show log viewer"));
        devTlsCheck->setText(tr(lang::keys::setting::category, lang::keys::setting::devTls, "TLS"));
        if (auto *label = devGroup->findChild<QLabel *>(QStringLiteral("devServerLabel"))) {
            label->setText(tr(lang::keys::setting::category, lang::keys::setting::devServer, "Server"));
        }
        devServerCheck->setText(tr(lang::keys::setting::category, lang::keys::setting::useDefaultServer, "Use default server"));
        devServerEdit->setPlaceholderText(tr(lang::keys::setting::category, lang::keys::setting::devServerPlaceholder, "https://example.com"));
        devPreviewLabel->setText(QStringLiteral("UI Preview"));
        devShowNoticeBtn->setText(QStringLiteral("Show Notice"));
        devShowInputBtn->setText(QStringLiteral("Show Input"));
        devShowLoadingBtn->setText(QStringLiteral("Show Loading"));

        customTempDirBrowseBtn->setToolTip(tr(lang::keys::setting::category, lang::keys::setting::selectTempDir, "Select temp folder"));
        closeTabButton->setToolTip(tr(lang::keys::setting::category, lang::keys::setting::close, "Close"));

        const std::string status = authStatusText.empty()
                                       ? lang::tr(lang::keys::setting::category, lang::keys::setting::notLoggedIn, "__not_logged_in__")
                                       : authStatusText;
        authStatusLabel->setText(QString::fromStdString(status));
        authButton->setText(QString::fromStdString(lang::tr(lang::keys::setting::category, core::auth::isLoggedIn() ? lang::keys::setting::logout : lang::keys::setting::login,
                                                            core::auth::isLoggedIn() ? "__logout__" : "__login__")));
    }

    bool SettingPage::isBuiltinTheme(const QString &name) {
        const QString lower = name.trimmed().toLower();
        return lower == QString::fromLatin1(themeio::kSystemName).toLower() ||
               lower == QString::fromLatin1(themeio::kLightName).toLower() ||
               lower == QString::fromLatin1(themeio::kDarkName).toLower();
    }

    void SettingPage::updateEditThemeState() {
        editThemeBtn->setEnabled(!isBuiltinTheme(themeCombo->currentData().toString()));
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
                                      "QLineEdit, QComboBox, QSpinBox, QFontComboBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 6px; }"
                                      "QLineEdit:focus, QComboBox:focus, QSpinBox:focus, QFontComboBox:focus { border: 1px solid %4; }")
                                      .arg(theme.colors.surface.data())
                                      .arg(theme.colors.text.data())
                                      .arg(theme.colors.disabled.data())
                                      .arg(theme.colors.focus.data());
        for (auto *w : {static_cast<QWidget *>(backgroundPathEdit), static_cast<QWidget *>(windowSizeEdit),
                        static_cast<QWidget *>(themeCombo), static_cast<QWidget *>(fontFamiliesCombo), static_cast<QWidget *>(languageCombo),
                        static_cast<QWidget *>(devServerEdit), static_cast<QWidget *>(proxyEdit), static_cast<QWidget *>(customTempDirEdit),
                        static_cast<QWidget *>(javaPathEdit), static_cast<QWidget *>(customResolutionEdit), static_cast<QWidget *>(joinServerAddressEdit)}) {
            w->setStyleSheet(editStyle);
        }
        for (auto *w : {static_cast<QWidget *>(backgroundTypeCombo), static_cast<QWidget *>(blurEffectCombo),
                        static_cast<QWidget *>(launcherMethodCombo), static_cast<QWidget *>(blurRadiusSlider),
                        static_cast<QWidget *>(fontPointSizeSpin), static_cast<QWidget *>(threadSpin),
                        static_cast<QWidget *>(downloadSourceCombo), static_cast<QWidget *>(joinServerPortSpin),
                        static_cast<QWidget *>(customTempDirBrowseBtn), static_cast<QWidget *>(fontFamiliesCombo), static_cast<QWidget *>(backgroundBrowseBtn)}) {
            w->setStyleSheet(editStyle);
        }

        const QString sliderStyle = QString(
                                        "QSlider::groove:horizontal { height: 8px; background: %1; border: 1px solid %2; border-radius: 4px; }"
                                        "QSlider::handle:horizontal { width: 14px; background: %3; border: 1px solid %2; margin: -4px 0; border-radius: 7px; }"
                                        "QSlider::handle:horizontal:hover { background: %4; }")
                                        .arg(theme.colors.surface.data())
                                        .arg(theme.colors.focus.data())
                                        .arg(theme.colors.accent.data())
                                        .arg(theme.colors.hover.data());
        blurRadiusSlider->setStyleSheet(sliderStyle);

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
        for (auto *c : {proxyCheck, devEnableCheck, devDebugCheck, devLogViewerCheck, devServerCheck, devTlsCheck}) {
            c->setStyleSheet(checkStyle);
        }

        const QString toolBtnStyle = QString(
                                         "QToolButton, QPushButton#editThemeBtn { background-color: %1; color: %2; border: 1px solid %3; border-radius: 8px; padding: 6px 10px; }"
                                         "QToolButton:hover, QPushButton#editThemeBtn:hover { background-color: %4; border-color: %3; }"
                                         "QToolButton:disabled, QPushButton#editThemeBtn:disabled { background-color: %5; color: %2; border-color: %5; }")
                                         .arg(theme.colors.surface.data())
                                         .arg(theme.colors.text.data())
                                         .arg(theme.colors.accent.data())
                                         .arg(theme.colors.hover.data())
                                         .arg(theme.colors.disabled.data());
        for (auto *tb : std::initializer_list<QWidget *>{customTempDirBrowseBtn, closeTabButton, javaPathBrowseBtn, backgroundBrowseBtn, editThemeBtn}) {
            tb->setStyleSheet(toolBtnStyle);
        }

        const QString btnStyle = QString(
                                     "QPushButton { background-color: %1; color: %2; border: none; border-radius: 10px; padding: 10px 14px; }"
                                     "QPushButton:hover { background-color: %3; }")
                                     .arg(theme.colors.primary.data())
                                     .arg(theme.colors.text.data())
                                     .arg(theme.colors.hover.data());
        for (auto *btn : {authButton, devShowNoticeBtn, devShowInputBtn, devShowLoadingBtn}) {
            btn->setStyleSheet(btnStyle);
        }

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

        for (auto w : std::initializer_list<QWidget *>{authStatusLabel, authButton, backgroundPathEdit, windowSizeEdit, themeCombo, fontFamiliesCombo, devServerEdit, languageCombo, backgroundBrowseBtn, devPreviewLabel, devShowNoticeBtn, devShowInputBtn, devShowLoadingBtn}) {
            w->setFont(text);
        }
        for (auto w : std::initializer_list<QWidget *>{backgroundTypeCombo, blurEffectCombo, launcherMethodCombo, blurRadiusSlider, fontPointSizeSpin, threadSpin}) {
            w->setFont(text);
        }
        for (auto c : std::initializer_list<QWidget *>{proxyCheck, devEnableCheck, devDebugCheck, devLogViewerCheck, devServerCheck, devTlsCheck}) {
            c->setFont(text);
        }
        for (auto w : std::initializer_list<QWidget *>{customTempDirEdit, customTempDirBrowseBtn, closeTabButton, proxyEdit, javaPathEdit, downloadSourceCombo, customResolutionEdit, joinServerAddressEdit, joinServerPortSpin, javaPathBrowseBtn, themeCombo}) {
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

    void SettingPage::setupText() {
        retranslateUi();
    }

    void SettingPage::refreshThemeList() {
        const auto names = themeio::listThemeNames(themeDir);
        const QString currentData = themeCombo->currentData().toString();
        themeCombo->blockSignals(true);
        themeCombo->clear();

        for (const auto &name : names) {
            const QString raw = QString::fromStdString(name);
            QString display = raw;
            const QString lower = raw.trimmed().toLower();
            if (lower == QString::fromLatin1(themeio::kSystemName).toLower()) {
                display = QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::themeSystem, raw.toUtf8().constData()));
            } else if (lower == QString::fromLatin1(themeio::kLightName).toLower()) {
                display = QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::themeLight, raw.toUtf8().constData()));
            } else if (lower == QString::fromLatin1(themeio::kDarkName).toLower()) {
                display = QString::fromStdString(lang::tr(lang::keys::setting::category, lang::keys::setting::themeDark, raw.toUtf8().constData()));
            }
            themeCombo->addItem(display, raw);
        }

        int idx = themeCombo->findData(currentData, Qt::MatchExactly);
        if (idx < 0) {
            idx = themeCombo->findText(currentData, Qt::MatchFixedString);
        }
        if (idx >= 0) {
            themeCombo->setCurrentIndex(idx);
        }

        themeCombo->blockSignals(false);
        updateEditThemeState();
    }

    void SettingPage::resizeItems(int windowWidth, int windowHeight) {
        setGeometry(0, 0, windowWidth, windowHeight);
        tabWidget->setGeometry(0, 0, windowWidth, windowHeight);
    }

    void SettingPage::setWindowSizeDisplay(const QString &size) {
        QSignalBlocker blocker(windowSizeEdit);
        windowSizeEdit->setText(size);
    }

    void SettingPage::settingFromConfig(const neko::ClientConfig &cfg) {

        suppressSignals = true;

        {
            const int idx = backgroundTypeCombo->findData(QString::fromStdString(cfg.main.backgroundType));
            if (idx >= 0) {
                backgroundTypeCombo->setCurrentIndex(idx);
            }
        }
        backgroundPathEdit->setText(QString::fromStdString(cfg.main.background));
        windowSizeEdit->setText(QString::fromStdString(cfg.main.windowSize));
        {
            const int idx = launcherMethodCombo->findData(QString::fromStdString(cfg.main.launcherMethod));
            if (idx >= 0) {
                launcherMethodCombo->setCurrentIndex(idx);
            }
        }

        {
            QSignalBlocker blocker(languageCombo);
            const QString langCode = QString::fromStdString(cfg.main.lang);
            const int langIndex = languageCombo->findData(langCode);
            if (langIndex >= 0) {
                languageCombo->setCurrentIndex(langIndex);
            }
        }
        if (!cfg.main.lang.empty()) {
            lang::language(cfg.main.lang);
        }
        retranslateUi();

        {
            const int idx = themeCombo->findData(QString::fromStdString(cfg.style.theme));
            if (idx >= 0) {
                themeCombo->setCurrentIndex(idx);
            }
        }
        updateEditThemeState();
        {
            const int idx = blurEffectCombo->findData(QString::fromStdString(cfg.style.blurEffect));
            if (idx >= 0) {
                blurEffectCombo->setCurrentIndex(idx);
            }
        }
        const int blurRadiusVal = static_cast<int>(cfg.style.blurRadius);
        blurRadiusSlider->setValue(blurRadiusVal == 1 ? 0 : blurRadiusVal);
        fontPointSizeSpin->setValue(static_cast<int>(cfg.style.fontPointSize));
        fontFamiliesCombo->setCurrentText(QString::fromStdString(cfg.style.fontFamilies));

        threadSpin->setValue(static_cast<int>(cfg.net.thread));
        const bool useSystemProxy = (cfg.net.proxy == neko::strview("true"));
        proxyCheck->setChecked(useSystemProxy);
        proxyEdit->setText(useSystemProxy ? QString() : QString::fromStdString(cfg.net.proxy));
        proxyEdit->setVisible(!useSystemProxy);

        customTempDirEdit->setText(QString::fromStdString(cfg.other.tempFolder));

        javaPathEdit->setText(QString::fromStdString(cfg.minecraft.javaPath));
        {
            const int idx = downloadSourceCombo->findData(QString::fromStdString(cfg.minecraft.downloadSource));
            if (idx >= 0) {
                downloadSourceCombo->setCurrentIndex(idx);
            }
        }
        customResolutionEdit->setText(QString::fromStdString(cfg.minecraft.customResolution));
        joinServerAddressEdit->setText(QString::fromStdString(cfg.minecraft.joinServerAddress));
        bool okPort = false;
        int portVal = QString::fromStdString(cfg.minecraft.joinServerPort).toInt(&okPort);
        if (okPort) {
            joinServerPortSpin->setValue(portVal);
        }

        devEnableCheck->setChecked(cfg.dev.enable);
        devDebugCheck->setChecked(cfg.dev.debug);
        devLogViewerCheck->setChecked(cfg.dev.showLogViewer);
        const bool useDefaultDevServer = (cfg.dev.server == neko::strview("auto"));
        devServerCheck->setChecked(useDefaultDevServer);
        devServerEdit->setText(useDefaultDevServer ? QString() : QString::fromStdString(cfg.dev.server));
        devServerEdit->setVisible(!useDefaultDevServer);
        devTlsCheck->setChecked(cfg.dev.tls);

        suppressSignals = false;
    }

    void SettingPage::setAuthState(bool loggedIn, const std::string &statusText) {
        authStatusText = statusText;
        const std::string status = statusText.empty()
                                       ? lang::tr(lang::keys::setting::category, lang::keys::setting::notLoggedIn, "__not_logged_in__")
                                       : statusText;
        authStatusLabel->setText(QString::fromStdString(status));
        authButton->setText(QString::fromStdString(lang::tr(lang::keys::setting::category, core::auth::isLoggedIn() ? lang::keys::setting::logout : lang::keys::setting::login,
                                                            core::auth::isLoggedIn() ? "__logout__" : "__login__")));
    }

    void SettingPage::writeToConfig(ClientConfig &cfg) const {

        const QString langCode = languageCombo->currentData().toString();
        cfg.main.lang = langCode.isEmpty() ? languageCombo->currentText().toStdString() : langCode.toStdString();
        const QString bgType = backgroundTypeCombo->currentData().toString();
        auto bgTypeStd = bgType.toStdString();
        cfg.main.backgroundType = std::move(bgTypeStd);
        const QString bgPath = backgroundPathEdit->text();
        auto bgStd = bgPath.toStdString();
        cfg.main.background = std::move(bgStd);
        const QString ws = windowSizeEdit->text();
        auto wsStd = ws.toStdString();
        cfg.main.windowSize = std::move(wsStd);
        const QString lm = launcherMethodCombo->currentData().toString();
        auto lmStd = lm.toStdString();
        cfg.main.launcherMethod = std::move(lmStd);
        const QString th = themeCombo->currentData().toString();
        auto thStd = th.toStdString();
        cfg.style.theme = std::move(thStd);
        const QString be = blurEffectCombo->currentData().toString();
        auto beStd = be.toStdString();
        cfg.style.blurEffect = std::move(beStd);
        const int radiusVal = blurRadiusSlider->value();
        cfg.style.blurRadius = (radiusVal == 1) ? 0 : radiusVal;
        cfg.style.fontPointSize = fontPointSizeSpin->value();
        const QString ff = fontFamiliesCombo->currentText();
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

        const QString ds = downloadSourceCombo->currentData().toString();
        auto dsStd = ds.toStdString();
        cfg.minecraft.downloadSource = std::move(dsStd);

        const QString cr = customResolutionEdit->text();
        auto crStd = cr.toStdString();
        cfg.minecraft.customResolution = std::move(crStd);
        const QString js = joinServerAddressEdit->text();

        auto jsStd = js.toStdString();

        cfg.minecraft.joinServerAddress = std::move(jsStd);
        cfg.minecraft.joinServerPort = std::to_string(joinServerPortSpin->value());

        cfg.dev.enable = devEnableCheck->isChecked();

        cfg.dev.debug = devDebugCheck->isChecked();
        cfg.dev.showLogViewer = devLogViewerCheck->isChecked();
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
