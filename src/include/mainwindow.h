#pragma once
#include <string>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

#include <QtWidgets/QToolBar>

#include <QtWidgets/QProgressBar>

#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDialogButtonBox>
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

#include <QtGui/QMovie>
#include <QtGui/QWindow>
#include <QtGui/QtEvents>

#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <QtGui/QIntValidator>
#include <QtGui/QValidator>

#include <QtGui/QScreen>

#include "cconfig.h"
#include "msgtypes.h"

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
    public:
        enum class pageState {
            index,
            setting,
            loading
        };

    private:
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

            //launcher group
            QGroupBox *lcGroup;
            QVBoxLayout *lcGroupLayout;

            QWidget *lcWindowSetLayoutWidget;
            QHBoxLayout *lcWindowSetLayout;
            QLabel * lcWindowSetText;
            QComboBox * lcWindowSetBox;

            //launcher group

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
            QCheckBox *devOptUpdatePage;
            QCheckBox *devOptHintPage;

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
                versionText->setWordWrap(true);
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

        struct LoadingPage : public QWidget {
            LoadingPage(QWidget *parent = nullptr);
            QProgressBar *progressBar;
            pixmapWidget *poster;
            QWidget *textLayoutWidget;
            QVBoxLayout *textLayout;
            QLabel *titleH1;
            QLabel *titleH2;
            QLabel *text;
            QLabel *loadingLabel;
            QMovie *loadingMv;
            QLabel *process;

            void showLoad(const loadMsg &m) {
                process->setText(m.process.c_str());

                if (m.type == loadMsg::Type::Text || m.type == loadMsg::Type::All) {
                    titleH1->setText(m.h1.c_str());
                    titleH2->setText(m.h2.c_str());
                    text->setText(m.msg.c_str());
                }

                if (m.type == loadMsg::Type::Progress || m.type == loadMsg::Type::All) {
                    progressBar->setMaximum(m.progressMax);
                    progressBar->setValue(m.progressVal);
                }

                if (!m.poster.empty()) {
                    poster->setPixmap(m.poster.c_str());
                    poster->show();
                } else {
                    poster->hide();
                }

                if (loadingMv->speed() != m.speed)
                    loadingMv->setSpeed(m.speed);

                switch (m.type) {
                    case loadMsg::Type::Text:
                        progressBar->hide();
                        textLayoutWidget->show();
                        break;
                    case loadMsg::Type::Progress:
                        progressBar->show();
                        textLayoutWidget->hide();
                        break;
                    case loadMsg::Type::OnlyRaw:
                        progressBar->hide();
                        textLayoutWidget->hide();
                        break;
                    case loadMsg::Type::All:
                        progressBar->show();
                        textLayoutWidget->show();
                        break;
                    default:
                        break;
                }
            }
        };

        struct HeadBar : public QWidget {
            ToolBar *toolbar;
            QAction *close_;
            QAction *minimize;
            QAction *maximize;
            QAction *sp1;
            QAction *sp2;
            HeadBar(QWidget *parent = nullptr);
        };

        struct HintWindow : public QWidget {
            QWidget *centralWidget;
            pixmapWidget *poster;
            QVBoxLayout *centralWidgetLayout;
            QFrame *line;
            QPushButton *button;
            QDialogButtonBox *dialogButton;
            QLabel *title;
            QLabel *msg;
            HintWindow(QWidget *parent = nullptr);
            void setupButton(QPushButton *btn, const std::function<void(bool)> &callback,bool &did) {
                btn->show();
                connect(
                    btn, &QPushButton::clicked, [=, this,&did] {
                        callback(true);
                        disconnect(btn);
                        did = true;
                    });
            }

            void setupButton(QDialogButtonBox *btnBox, const std::function<void(bool)> &callback,bool & did) {
                btnBox->show();
                connect(
                    btnBox, &QDialogButtonBox::accepted, [=, this,&did] {
                        callback(true);
                        disconnect(btnBox);
                        did = true;
                    });
                connect(
                    btnBox, &QDialogButtonBox::rejected, [=, this,&did] {
                        callback(false);
                        disconnect(btnBox);
                        did = true;
                    });
            }
            void showHint(const hintMsg &m) {
                this->show();
                this->title->setText(m.title.c_str());
                this->msg->setText(m.msg.c_str());
                if (!m.poster.empty())
                    this->poster->setPixmap(m.poster.c_str());
                bool * did = new bool(false);
                if (m.buttonType == 1) {
                    setupButton(button, m.callback,*did);
                    dialogButton->hide();
                } else {
                    button->hide();
                    setupButton(dialogButton, m.callback,*did);
                }
                connect(this, &QWidget::destroyed, [=](QObject *) {
                    if (!(*did))
                        m.callback(false);

                });
            };
        };

    private:
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        pixmapWidget *bgWidget;
        QGraphicsBlurEffect *m_pBlurEffect;
        HintWindow *hintWidget;
        QWidget *widget;
        Index *index;
        Setting *setting;
        LoadingPage *loading;

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

        void showPage(pageState page) {
            oldState = state;
            state = page;
            updatePage(state, oldState);
        }

        void showLoad(const loadMsg &m) {
            loading->showLoad(m);
            oldState = state;
            state = pageState::loading;
            updatePage(state, oldState);
        }
        void setLoadingVal(unsigned int val) {
            loading->progressBar->setValue(val);
        };
        void setLoadingNow(const char *msg) {
            loading->process->setText(msg);
        };
        // button type 1 : use one button
        void showHint(const hintMsg &m) {
            hintWidget->HintWindow::showHint(m);
            resizeItem();
        };
    signals:
        void showPageD(pageState page);
        void showLoadD(const loadMsg &m);
        void setLoadingValD(unsigned int val);
        void setLoadingNowD(const char *msg);
        void showHintD(const hintMsg &m);
    };

} // namespace ui