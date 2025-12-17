#include "neko/ui/dialogs/inputDialog.hpp"

#include "neko/ui/animation.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/app/lang.hpp"

#include <QTimer>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QGraphicsDropShadowEffect>

namespace neko::ui::dialog {

        InputDialog::InputDialog(QWidget *parent)
                : QWidget(parent),
                    poster(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
                    centralWidget(new QWidget(this)),
                    centralWidgetLayout(new QVBoxLayout(centralWidget)),
                    title(new QLabel(centralWidget)),
                    msg(new QLabel(centralWidget)),
                    dialogButton(new QDialogButtonBox(centralWidget)) {
                this->setAttribute(Qt::WA_TranslucentBackground, true);
                // poster->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                centralWidget->setAttribute(Qt::WA_StyledBackground, true);
                poster->lower();

                msg->setWordWrap(true);
                msg->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
                msg->setMaximumHeight(msg->fontMetrics().lineSpacing() * 3 + 12); // keep the message compact

                centralWidget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                centralWidgetLayout->setContentsMargins(24, 16, 24, 20);
                centralWidgetLayout->setSpacing(18);
                title->setAlignment(Qt::AlignCenter);
                centralWidgetLayout->addWidget(title);
                centralWidgetLayout->addWidget(msg);
                centralWidget->setLayout(centralWidgetLayout);

                auto *cardShadow = new QGraphicsDropShadowEffect(this);
                cardShadow->setBlurRadius(36);
                cardShadow->setOffset(0, 10);
                centralWidget->setGraphicsEffect(cardShadow);

                dialogButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                dialogButton->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
                dialogButton->setCenterButtons(true);
                dialogButton->setMinimumHeight(52);

                connect(this, &InputDialog::hideInputD, this, &InputDialog::hideInput);
                connect(this, &InputDialog::showInputD, this, &InputDialog::showInput);
                connect(this, &InputDialog::getLinesD, this, &InputDialog::getLines);
        }

    void InputDialog::hideInput() {
        // Use pop-out animation before cleaning up
        anim::popOut(this, anim::Duration::Fast);
        
        // Use a delayed cleanup (after animation completes)
        QTimer::singleShot(anim::Duration::Fast + 50, this, [this]() {
            poster->clearPixmap();
            for (auto it : lines) {
                centralWidgetLayout->removeWidget(it);
                it->deleteLater();
            }
            lines.clear();
            centralWidgetLayout->removeWidget(dialogButton);
            disconnect(dialogButton);

            // Remove any spacers inserted by setLines to avoid layout stacking.
            for (int i = centralWidgetLayout->count() - 1; i >= 0; --i) {
                QLayoutItem *item = centralWidgetLayout->itemAt(i);
                if (item && item->spacerItem()) {
                    auto taken = centralWidgetLayout->takeAt(i);
                    delete taken;
                }
            }
        });
    }

    void InputDialog::showInput(const InputMsg &m) {
        title->setText(QString::fromStdString(m.title));
        msg->setText(QString::fromStdString(m.message));
        if (!m.posterPath.empty())
            (void)poster->setPixmap(std::string_view(m.posterPath));

        if (m.lineText.empty()) {
            setLines({lang::tr(lang::keys::input::category, lang::keys::input::placeholder)});
        } else {
            setLines(m.lineText);
        }

        // Use pop-in animation for showing
        anim::popIn(this, anim::Duration::Normal);

        auto did = std::make_shared<bool>(false);
        if (!m.callback)
            return;

        // Guard against double emission (Qt can emit both clicked/accepted or repeated signals)
        connect(
            dialogButton, &QDialogButtonBox::accepted, [did, m] {
                if (*did)
                    return;
                *did = true;
                m.callback(true);
            });
        connect(
            dialogButton, &QDialogButtonBox::rejected, [did, m] {
                if (*did)
                    return;
                *did = true;
                m.callback(false);
            });

    }

    void InputDialog::setLines(const std::vector<std::string> &vec) {
        for (auto &it : vec) {
            QLineEdit *line = new QLineEdit(centralWidget);
            line->setMinimumHeight(50);
            line->setPlaceholderText(QString::fromStdString(it));
            line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            if (it == lang::tr(lang::keys::input::category, lang::keys::input::password)) {
                line->setEchoMode(QLineEdit::PasswordEchoOnEdit);
            }
            centralWidgetLayout->addWidget(line);
            lines.push_back(line);
        }
        centralWidgetLayout->addSpacing(8);
        centralWidgetLayout->addWidget(dialogButton);
    }
    std::vector<std::string> InputDialog::getLines() {
        std::vector<std::string> vec;
        for (auto it : lines) {
            vec.push_back(it->text().toStdString());
        }
        return vec;
    }
    void InputDialog::resizeItems(int windowWidth, int windowHeight) {
        setGeometry(0, 0, windowWidth, windowHeight);
        poster->setGeometry(0, 0, windowWidth, windowHeight);

        const int cw = static_cast<int>(windowWidth * 0.32);
        const int ch = static_cast<int>(windowHeight * 0.68);
        const int cx = (windowWidth - cw) / 2;
        const int cy = (windowHeight - ch) / 2;
        centralWidget->setGeometry(cx, cy, cw, ch);
    }

    void InputDialog::setupFont(QFont font, QFont titleFont) {
        this->setFont(font);
        centralWidget->setFont(font);
        msg->setFont(font);
        dialogButton->setFont(font);
        title->setFont(titleFont);
    }
    void InputDialog::setupTheme(const Theme &theme) {
        const auto cardBg = theme.colors.panel.empty() ? theme.colors.surface : theme.colors.panel;
        centralWidget->setStyleSheet(
            QString("QWidget { background: %1; border: 1.5px solid %2; border-radius: 22px; }"
                    "QLineEdit { background-color: %3; color: %4; border: 1px solid %5; padding: 10px 12px; border-radius: 12px; selection-background-color: %6; }"
                    "QLineEdit:focus { border: 1.5px solid %6; }"
                    "QLineEdit::placeholder { color: %7; }")
                .arg(cardBg.data())
                .arg(theme.colors.accent.data())
                .arg(theme.colors.surface.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.focus.data())
                .arg(theme.colors.disabled.data()));

        title->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; background-color: transparent; border: none; }")
                .arg(theme.colors.accent.data()));
        msg->setStyleSheet(
            QString("QLabel { color: %1; background-color: transparent; border: none; }")
                .arg(theme.colors.text.data()));
        dialogButton->setStyleSheet(
            QString("QDialogButtonBox { background-color: transparent; border: none; }"
                    "QPushButton { color: %1; background-color: %2; border: none; border-radius: 16px; padding: 12px 20px; }"
                    "QPushButton:hover { background-color: %3; }"
                    "QPushButton:pressed { background-color: %5; }"
                    "QPushButton:disabled { background-color: %4; }")
                .arg(theme.colors.text.data())
                .arg(theme.colors.primary.data())
                .arg(theme.colors.hover.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.focus.data()));
    }

} // namespace neko::ui::dialog
