#include "neko/ui/dialogs/noticeDialog.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"
#include "neko/app/lang.hpp"

#include <algorithm>
#include <memory>
#include <string_view>

#include <QtCore/QTimer>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace neko::ui::dialog {

    NoticeDialog::NoticeDialog(QWidget *parent)
                : QWidget(parent),
                    poster(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
                    centralWidget(new QWidget(this)),
                    centralWidgetLayout(new QVBoxLayout(centralWidget)),
                    title(new QLabel(centralWidget)),
                    line(new QFrame(centralWidget)),
                    msg(new QLabel(centralWidget)),
                    buttonContainer(new QWidget(centralWidget)),
                    buttonLayout(new QHBoxLayout(buttonContainer)) {

        this->setAttribute(Qt::WA_TranslucentBackground, true);

        centralWidget->setObjectName("noticeCentral");
        buttonContainer->setObjectName("noticeButtons");
        centralWidgetLayout->addWidget(title);
        centralWidgetLayout->addWidget(line);
        centralWidgetLayout->addWidget(msg);
        centralWidgetLayout->addWidget(buttonContainer);
        centralWidgetLayout->setStretchFactor(title, 1);
        centralWidgetLayout->setStretchFactor(msg, 2);
        centralWidgetLayout->setStretchFactor(buttonContainer, 1);
        centralWidgetLayout->setContentsMargins(24, 16, 24, 20);
        centralWidgetLayout->setSpacing(12);
        centralWidget->setLayout(centralWidgetLayout);

        title->setAlignment(Qt::AlignLeft);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        msg->setWordWrap(true);
        poster->lower();

        buttonLayout->setContentsMargins(0, 8, 0, 4);
        buttonLayout->setSpacing(12);
        buttonContainer->setLayout(buttonLayout);
        buttonContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        buttonContainer->setFocusPolicy(Qt::StrongFocus);
        buttonContainer->setFocus();
        buttonContainer->setFocusProxy(this);

        connect(this, &NoticeDialog::showNoticeD, this, &NoticeDialog::showNotice);
        connect(this, &NoticeDialog::resetStateD, this, &NoticeDialog::resetState);
        connect(this, &NoticeDialog::resetButtonsD, this, &NoticeDialog::resetButtons);
    }
    void NoticeDialog::setupFont(QFont font, QFont titleFont) {
        msg->setFont(font);
        title->setFont(titleFont);
    }
    void NoticeDialog::setupTheme(const Theme &theme) {

        centralWidget->setAttribute(Qt::WA_StyledBackground, true);
        centralWidget->setStyleSheet(
            QString(
                "#noticeCentral { background-color: %1; border-radius: 22px; }"
                "#noticeButtons { background-color: transparent; }"
                "QLabel { color: %2; }"
                "QPushButton { background-color: %3; color: %2; border-radius: 14px; padding: 14px 18px; }"
                "QPushButton:hover { background-color: %6; }"
                "QPushButton:disabled { background-color: %7; }")
                .arg(theme.colors.background.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.accent.data())
                .arg(theme.colors.hover.data())
                .arg(theme.colors.disabled.data()));

        line->setStyleSheet(
            QString("QFrame { border: 1px solid %1; }")
                .arg(theme.colors.secondary.data()));
        title->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; }")
                .arg(theme.colors.accent.data()));
        msg->setStyleSheet(
            QString("QLabel { color: %1; }")
                .arg(theme.colors.text.data()));
    }

    void NoticeDialog::showNotice(const NoticeMsg &m) {
        this->show();
        this->raise();
        title->setText(QString::fromStdString(m.title));
        msg->setText(QString::fromStdString(m.message));
        if (!m.posterPath.empty()){
            (void)poster->setPixmap(std::string_view(m.posterPath));
        }
            
        const auto configureButton = [](QPushButton *button) {
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        };

        const auto applyUniformButtonSize = [this]() {
            if (buttons.empty())
                return;

            int uniformWidth = 0;
            int uniformHeight = 0;
            for (auto button : buttons) {
                const auto hint = button->sizeHint();
                uniformWidth = std::max(uniformWidth, hint.width());
                uniformHeight = std::max(uniformHeight, hint.height());
            }
            constexpr int kMinButtonWidth = 120;
            constexpr int kMaxButtonWidth = 220;
            constexpr int kButtonHeight = 44;
            uniformWidth = std::clamp(uniformWidth, kMinButtonWidth, kMaxButtonWidth);
            uniformHeight = kButtonHeight;
            for (auto button : buttons) {
                button->setMinimumWidth(uniformWidth);
                button->setMaximumWidth(QWIDGETSIZE_MAX);
                button->setMinimumHeight(uniformHeight);
                button->setMaximumHeight(uniformHeight);
                button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            }

            // Ensure layout applies equal stretch to every button.
            for (int i = 0; i < buttonLayout->count(); ++i) {
                buttonLayout->setStretch(i, 1);
            }
        };

        // If no buttons, add the default button
        if (m.buttonText.empty()) {
            auto btn = new QPushButton(QString::fromStdString(lang::tr(lang::keys::button::category,lang::keys::button::ok)), buttonContainer);
            buttons.push_back(btn);
            buttonLayout->addWidget(btn);
            configureButton(btn);
            btn->setDefault(true);
            btn->setFocus();
            applyUniformButtonSize();
            return;
        }

        // Push the buttons to the custom layout
        for (neko::uint32 i = 0; i < m.buttonText.size(); ++i) {
            auto btn = new QPushButton(QString::fromStdString(m.buttonText[i]), buttonContainer);
            buttons.push_back(btn);
            buttonLayout->addWidget(btn);
            configureButton(btn);
        }

        applyUniformButtonSize();
        // Set the default button if specified
        buttonContainer->setFocus();
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
            for (auto btn : buttons) {
                connect(btn, &QPushButton::clicked, this, [this]() {
                    resetState();
                });
            }
            return;
        }

        for (neko::uint32 i = 0; i < buttons.size(); ++i) {
            connect(buttons[i], &QPushButton::clicked, this, [this, m, did, i]() {
                m.callback(i);
                *did = true;
                resetState();
            });
        }

        connect(this, &QWidget::destroyed, this, [this, m, did](QObject *) {
            if (!*did) {
                m.callback(m.defaultButtonIndex);
                resetState();
            }
        });
    }

    void NoticeDialog::resetState() {
        poster->clearPixmap();
        title->clear();
        msg->clear();
        resetButtons();
        disconnect(this, &QWidget::destroyed, nullptr, nullptr);
        this->hide();
    }
    void NoticeDialog::resetButtons() {
        disconnect(buttonContainer);
        for (auto btn : buttons) {
            disconnect(btn);
            btn->deleteLater();
        }
        buttons.clear();
        while (buttonLayout->count() > 0) {
            auto item = buttonLayout->takeAt(0);
            if (auto widget = item->widget()) {
                widget->setParent(nullptr);
                widget->deleteLater();
            }
            delete item;
        }
    }

    void NoticeDialog::resizeItems(int windowWidth, int windowHeight) {
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
} // namespace neko::ui::dialog
