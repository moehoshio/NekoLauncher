#pragma once
// Neko Modules

// Neko Ui Modules
#include "neko/ui/state.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/headBarWidget.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/ui/dialogs/choiceDialog.hpp"
#include "neko/ui/dialogs/hintDialog.hpp"
#include "neko/ui/dialogs/inputDialog.hpp"

#include "neko/ui/pages/homePage.hpp"
#include "neko/ui/pages/loadingPage.hpp"
#include "neko/ui/pages/settingPages.hpp"

// Neko Other Modules
#include "neko/schema/clientconfig.hpp"

// C++ Standard Library
#include <string>
#include <vector>

// Qt Modules
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

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

#include <QtGui/QIntValidator>
#include <QtGui/QValidator>

#include <QtGui/QScreen>

namespace neko::ui {

    class MainWindow : public QMainWindow {
        Q_OBJECT

    private:
        const QSize scrSize = QGuiApplication::primaryScreen()->size();

        PixmapWidget *bgWidget;
        HeadBarWidget *headBarWidget;
        QWidget *widget;

        HomePage *homePage;
        SettingPage *settingPage;
        LoadingPage *loadingPage;

        HintDialog *hintDialog;
        InputDialog *inputDialog;
        // ChoiceDialog *choice; Not implemented yet

        QGraphicsBlurEffect *m_pBlurEffect;

        QFont font;
        int blurRadius;

        pageState state;
        pageState oldState;

    protected:
        void closeEvent(QCloseEvent *event) override;

        void dragEnterEvent(QDragEnterEvent *p_event) override {

            p_event->acceptProposedAction();
        }

        void dragMoveEvent(QDragMoveEvent *p_event) override {

            p_event->acceptProposedAction();
        }

        void dropEvent(QDropEvent *p_event) override {
            p_event->acceptProposedAction();
        }

        void keyPressEvent(QKeyEvent *event) override {
            QWidget *currentFocus = focusWidget();

            switch (event->key()) {
                case Qt::Key_Return:
                    if (auto checkBox = dynamic_cast<QCheckBox *>(currentFocus)) {
                        emit checkBox->clicked(!checkBox->isChecked());
                    } else if (auto button = dynamic_cast<QPushButton *>(currentFocus)) {
                        emit button->clicked(true);
                    }

                    break;
                default:
                    QWidget::keyPressEvent(event);
            }
        }

        bool event(QEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    public:
        void resizeItems();

        void setupSize();

        void setupStyle();
        void setupTranslucentBackground();

        void setupText();
        void setupFont(neko::ClientConfig config);

        void setupBase(neko::ClientConfig config);

        void autoSetText(QFont text);
        void setTextFont(QFont text, QFont h2, QFont h1);

        void setupConnect();

        MainWindow(neko::ClientConfig config);

        void updatePage(pageState state, pageState oldState);

    public:
        inline void showPage(pageState page) {
            oldState = state;
            state = page;
            updatePage(state, oldState);
        }

        inline void showLoad(const loadMsg &m) {
            loading->showLoad(m);
            oldState = state;
            state = pageState::loading;
            updatePage(state, oldState);
        }
        inline void setLoadingVal(unsigned int val) {
            loading->progressBar->setValue(val);
        };
        inline void setLoadingNow(const char *msg) {
            loading->process->setText(msg);
        };
        // button type 1 : use one button
        inline void showHint(const hintMsg &m) {
            hintWidget->showHint(m);
            resizeItem();
        };

        inline void showInput(const InputMsg &m) {
            input->showInput(m);
            resizeItem();
        }
        inline auto getInput() {
            return input->getLines();
        }
        inline void hideInput() {
            input->InputPage::hideInput();
        }
        inline void winShowHide(bool check) {
            if (check)
                this->show();
            else
                this->hide();
        }
    signals:
        void showPageD(pageState page);
        void showLoadD(const loadMsg &m);
        void setLoadingValD(unsigned int val);
        void setLoadingNowD(const char *msg);
        void showHintD(const hintMsg &m);
        void showInputD(const InputMsg &m);
        void hideInputD();
        void loginStatusChangeD(const std::string &name);
        void winShowHideD(bool check);
    };

} // namespace neko::ui