#pragma once

#include "neko/ui/theme.hpp"
#include "neko/app/clientConfig.hpp"

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
class QVBoxLayout;
class QToolButton;
class QScrollArea;

namespace neko::ui::page {

	class SettingPage : public QWidget {
		Q_OBJECT
	private:
		QTabWidget *tabWidget;

		QScrollArea *authScroll;

		// Auth tab
		QWidget *authTab;
		QLabel *authStatusLabel;
		QPushButton *authButton;

		// Main settings tab
		QScrollArea *mainScroll;
		QWidget *mainTab;
		QGroupBox *mainGroup;
		QComboBox *backgroundTypeCombo;
		QLineEdit *backgroundPathEdit;
		QLineEdit *windowSizeEdit;
		QComboBox *launcherMethodCombo;
		QCheckBox *useSysWindowFrameCheck;
		QCheckBox *headBarKeepRightCheck;

		QGroupBox *styleGroup;
		QLineEdit *themeEdit;
		QComboBox *blurEffectCombo;
		QSpinBox *blurRadiusSpin;
		QSpinBox *fontPointSizeSpin;
		QLineEdit *fontFamiliesEdit;

		QGroupBox *networkGroup;
		QSpinBox *threadSpin;
		QCheckBox *proxyCheck;
			QLineEdit *proxyEdit;

		QGroupBox *otherGroup;
		QLineEdit *customTempDirEdit;
		QToolButton *customTempDirBrowseBtn;
			QToolButton *closeTabButton;

		QGroupBox *minecraftGroup;
		QLineEdit *javaPathEdit;
		QComboBox *downloadSourceCombo;
		QLineEdit *playerNameEdit;
		QLineEdit *customResolutionEdit;
		QLineEdit *joinServerAddressEdit;
		QSpinBox *joinServerPortSpin;

		// Advanced tab
		QScrollArea *advancedScroll;
		QWidget *advancedTab;
		QGroupBox *devGroup;
		QCheckBox *devEnableCheck;
		QCheckBox *devDebugCheck;
		QCheckBox *devServerCheck;
		QLineEdit *devServerEdit;
		QCheckBox *devTlsCheck;

		void buildUi();
		void setupCombos();
		void applyGroupStyle(const Theme &theme);

	public:
		SettingPage(QWidget *parent = nullptr);

		void setupTheme(const Theme &theme);
		void setupFont(QFont text, QFont h1Font, QFont h2Font);
		void resizeItems(int windowWidth, int windowHeight);

		void settingFromConfig(const neko::ClientConfig &cfg);
		void writeToConfig(neko::ClientConfig &cfg) const;
		void setAuthState(bool loggedIn, const std::string &statusText);

	signals:
		void closeRequested();
		void proxyModeChanged(bool useSystemProxy);
		void devServerModeChanged(bool useDefaultServer);
	};

} // namespace neko::ui::page
