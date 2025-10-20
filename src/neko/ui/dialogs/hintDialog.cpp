#include "neko/ui/dialogs/hintDialog.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"
#include "neko/app/lang.hpp"

#include <QtCore/QTimer>

namespace neko::ui {
    HintDialog::HintDialog(QWidget *parent)
        : QWidget(parent),
          poster(new PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
          centralWidget(new QWidget(this)),
          centralWidgetLayout(new QVBoxLayout(centralWidget)),
          title(new QLabel(centralWidget)),
          line(new QFrame(centralWidget)),
          dialogButton(new QDialogButtonBox(centralWidget)),
          msg(new QLabel(centralWidget)) {

        this->setAttribute(Qt::WA_TranslucentBackground, true);

        centralWidgetLayout->addWidget(title);
        centralWidgetLayout->addWidget(line);
        centralWidgetLayout->addWidget(msg);
        centralWidgetLayout->addWidget(dialogButton);
        centralWidgetLayout->setStretchFactor(msg, 2);
        centralWidget->setLayout(centralWidgetLayout);

        title->setAlignment(Qt::AlignLeft);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        msg->setWordWrap(true);
        poster->lower();

        dialogButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        dialogButton->setMinimumHeight(50);
        dialogButton->setStandardButtons(QDialogButtonBox::NoButton);
        dialogButton->setCenterButtons(true);
        dialogButton->setFocusPolicy(Qt::StrongFocus);
        dialogButton->setFocus();
        dialogButton->setFocusProxy(this);
    }
    void HintDialog::setupFont(QFont font, QFont titleFont) {
        this->setFont(font);
        msg->setFont(font);
        title->setFont(titleFont);
    }
    void HintDialog::setupStyle(const Theme &theme) {

        centralWidget->setStyleSheet(
            QString(
                "QWidget { background-color: %1; }"
                "QLabel { color: %2; }"
                "QPushButton { background-color: %3; color: %2; border-radius: %4; padding: %5; }"
                "QPushButton:hover { background-color: %6; }"
                "QPushButton:disabled { background-color: %7; }"
                "QDialogButtonBox { border: none; }")
                .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size()))
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size()))
                .arg(QString::fromUtf8(theme.accentColor.data(), theme.accentColor.size()))
                .arg(QString::fromUtf8(theme.borderRadius.data(), theme.borderRadius.size()))
                .arg(theme.padding)
                .arg(QString::fromUtf8(theme.hoverColor.data(), theme.hoverColor.size()))
                .arg(QString::fromUtf8(theme.disabledColor.data(), theme.disabledColor.size())));

        line->setStyleSheet(
            QString("QFrame { border: 1px solid %1; }")
                .arg(QString::fromUtf8(theme.separatorColor.data(), theme.separatorColor.size())));
        title->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; }")
                .arg(QString::fromUtf8(theme.accentColor.data(), theme.accentColor.size())));
        msg->setStyleSheet(
            QString("QLabel { color: %1; }")
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        dialogButton->setStyleSheet(
            QString(
                "QDialogButtonBox { background-color: %1; }"
                "QPushButton { color: %2; }")
                .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size()))
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
    }

    void HintDialog::showHint(const HintMsg &m) {
        this->show();
        title->setText(QString::fromStdString(m.title));
        msg->setText(QString::fromStdString(m.message));
        if (!m.poster.empty())
            (void)poster->setPixmap(std::string_view(m.poster));

        // If no buttons, add the default button
        if (m.buttonText.empty()) {
            auto btn = new QPushButton(QString::fromStdString(neko::lang::tr(std::string(neko::lang::keys::button::ok))), this);
            buttons.push_back(btn);
            dialogButton->addButton(btn, QDialogButtonBox::ActionRole);
            btn->setDefault(true);
            btn->setFocus();
            return;
        }

        // Push the buttons to the dialogButtonBox
        for (neko::uint32 i = 0; i < m.buttonText.size(); ++i) {
            auto btn = new QPushButton(QString::fromStdString(m.buttonText[i]), this);
            buttons.push_back(btn);
            dialogButton->addButton(btn, QDialogButtonBox::ActionRole);
        }

        // Set the default button if specified
        dialogButton->setFocus();
        if (m.defaultButtonIndex >= 0 && m.defaultButtonIndex < buttons.size()) {
            buttons[m.defaultButtonIndex]->setDefault(true);
            buttons[m.defaultButtonIndex]->setFocus();
        }

        // If did ( user clicked ) , window is closed , the do not call the callback
        auto did = std::make_shared<bool>(false);

        // If autoClose is set, start a timer to close the dialog after the specified time
        if (m.autoClose > 0) {
            QTimer::singleShot(m.autoClose, this, [=]() {
                if (m.callback && !*did) {
                    m.callback(m.defaultButtonIndex);
                    *did = true;
                    resetState();
                }
            });
        }

        // If the callback is not set, only reset the state
        if (!m.callback) {
            connect(dialogButton, &QDialogButtonBox::clicked, this, [this](QAbstractButton *button) {
                resetState();
            });
            return;
        }

        connect(dialogButton, &QDialogButtonBox::clicked, this, [this, m, did](QAbstractButton *button) {
            for (neko::uint32 i = 0; i < buttons.size(); ++i) {
                if (buttons[i] == button) {
                    m.callback(i);
                    *did = true;
                    resetState();
                    break;
                }
            }
        });

        connect(this, &QWidget::destroyed, this, [this, m, did](QObject *) {
            if (!*did) {
                m.callback(m.defaultButtonIndex); // If the dialog is closed without clicking a button, call the callback with 0
                resetState();
            }
        });
    }

    void HintDialog::resetState() {
        poster->clearPixmap();
        title->clear();
        msg->clear();
        resetButtons();
        disconnect(this, &QWidget::destroyed, nullptr, nullptr);
        this->hide();
    }
    void HintDialog::resetButtons() {
        dialogButton->clear();
        disconnect(dialogButton);
        for (auto btn : buttons) {
            btn->deleteLater();
        }
        buttons.clear();
    }

    void HintDialog::resizeItems(int windowWidth, int windowHeight) {
        if (!this->isVisible())
            return;

        resize(windowWidth, windowHeight);

        if (poster) {
            poster->setGeometry(0, 0, windowWidth, windowHeight);
        }
        if (centralWidget) {
            int cw = windowWidth * 0.4;
            int ch = windowHeight * 0.4;
            int cx = (windowWidth - cw) / 2;
            int cy = (windowHeight - ch) / 2;
            centralWidget->setGeometry(cx, cy, cw, ch);
        }
    }
} // namespace neko::ui
