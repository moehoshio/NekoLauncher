#pragma once
#include <string>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

#include <QtWidgets/QToolBar>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QGroupBox>

#include <QtWidgets/QGraphicsBlurEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QStyle>

#include <QtWidgets/QScrollArea>

#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QToolButton>

#include <QtWidgets/QSlider>

#include <QtWidgets/QFileDialog>

#include <QtGui/QWindow>
#include <QtGui/QtEvents>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <QtGui/QIntValidator>
#include <QtGui/QValidator>

#include <QtGui/QScreen>

#include "cconfig.h"

namespace ui {

    class ToolBar : public QToolBar {
        Q_OBJECT
    public:
        ToolBar(QWidget *parent = nullptr) : QToolBar(parent) {
            setMovable(false);
            setFloatable(false);

            setAcceptDrops(true);

            setContextMenuPolicy(Qt::PreventContextMenu);
        }
        bool event(QEvent *event) {
            if (event->type() == QEvent::MouseButtonPress)
                emit request_move_window();

            return QToolBar::event(event);
        }
        void dragEnterEvent(QDragEnterEvent *event) {
            event->acceptProposedAction();
        }

        void dragMoveEvent(QDragMoveEvent *p_event) {
            p_event->acceptProposedAction();
        }
    signals:
        void request_move_window();
    };

    class pixmapWidget : public QWidget {
        QPixmap bg;

    public:
        pixmapWidget(QWidget *parent = nullptr) : QWidget(parent){};
        pixmapWidget(const QPixmap &pixmap, QWidget *parent = nullptr) : QWidget(parent) {
            setPixmap(pixmap);
        }

        void setPixmap(const QPixmap &pix) {
            if (pix.isNull())
                return;
            bg = pix;
        }
        void setPixmap(const char *fileName) {
            if (std::string(fileName) == "")
                return;

            bg.load(fileName);
        }
        void paintEvent(QPaintEvent *event) {
            QPainter painter(this);

            painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

            if (!bg.isNull())
                painter.drawPixmap(rect(), bg);

            QWidget::paintEvent(event);
        }
    };

    class MainWindow : public QMainWindow {
        Q_OBJECT
        enum class pageState {
            index,
            setting
        };
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

            QWidget *devServerInputLayoutWidget;
            QHBoxLayout *devServerInputLayout;
            QCheckBox *devServerAuto;
            QLineEdit *devServerEdit;
            SettingPageThree(QWidget *parent = nullptr);
        };

        struct Index : public QWidget {
            Index(QWidget *parent = nullptr) : QWidget(parent) {
                startButton = new QPushButton(this);
                menuButton = new QPushButton(this);
                versionText = new QLabel(this);
            };

            QPushButton *startButton;
            QPushButton *menuButton;
            QLabel *versionText;
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
        struct HeadBar : public QWidget {
            ToolBar *toolbar;
            QAction *close_;
            QAction *minimize;
            QAction *maximize;
            HeadBar(QWidget *parent = nullptr);
        };

    private:
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        pixmapWidget *bgWidget;
        QGraphicsBlurEffect *m_pBlurEffect;

        QWidget *widget;
        Index *index;
        Setting *setting;

        HeadBar *headbar;

        QFont f;
        int blurVal;

        pageState state;
        pageState oldState;

    public:
        void resizeEvent(QResizeEvent *event);

        void resizeItem();

        void setupSize();

        void setupStyle();
        void setupTranslucentBackground();

        void setupText();
        void setupFont(neko::Config config);

        void setupBase(neko::Config config);

        void autoSetText(QFont text);
        void setTextFont(QFont text, QFont h2, QFont h1);

        void setupConnect();

        MainWindow(neko::Config config);

        void updatePage(pageState state, pageState oldState);

        void closeEvent(QCloseEvent *event);

        void dragEnterEvent(QDragEnterEvent *p_event) {

            p_event->acceptProposedAction();
        }

        void dragMoveEvent(QDragMoveEvent *p_event) {

            p_event->acceptProposedAction();
        }

        void dropEvent(QDropEvent *p_event) {
            p_event->acceptProposedAction();
        }
        bool event(QEvent *event);
    };

} // namespace ui