#include "neko/ui/dialogs/inputDialog.hpp"

#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/app/lang.hpp"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>

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
                poster->setAttribute(Qt::WA_TransparentForMouseEvents, true);
                centralWidget->setAttribute(Qt::WA_StyledBackground, true);
                poster->lower();

                msg->setWordWrap(true);
                msg->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
                msg->setMaximumHeight(msg->fontMetrics().lineSpacing() * 3 + 12); // keep the message compact

                centralWidget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
                centralWidgetLayout->setContentsMargins(24, 16, 24, 20);
                centralWidgetLayout->setSpacing(14);
                title->setAlignment(Qt::AlignCenter);
                centralWidgetLayout->addWidget(title);
                centralWidgetLayout->addWidget(msg);
                centralWidget->setLayout(centralWidgetLayout);

                dialogButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                dialogButton->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
                dialogButton->setCenterButtons(true);
                dialogButton->setMinimumHeight(52);

                connect(this, &InputDialog::hideInputD, this, &InputDialog::hideInput);
                connect(this, &InputDialog::showInputD, this, &InputDialog::showInput);
                connect(this, &InputDialog::getLinesD, this, &InputDialog::getLines);
        }

    void InputDialog::hideInput() {
        this->hide();
        poster->clearPixmap();
        for (auto it : lines) {
            centralWidgetLayout->removeWidget(it);
            it->deleteLater();
        }
        lines.clear();
        centralWidgetLayout->removeWidget(dialogButton);
        disconnect(dialogButton);
    }

    void InputDialog::showInput(const InputMsg &m) {
        this->show();
        this->raise();
        title->setText(QString::fromStdString(m.title));
        msg->setText(QString::fromStdString(m.message));
        if (!m.posterPath.empty())
            (void)poster->setPixmap(std::string_view(m.posterPath));

        if (m.lineText.empty()) {
            setLines({lang::tr(lang::keys::input::category, lang::keys::input::placeholder)});
        } else {
            setLines(m.lineText);
        }

        auto did = std::make_shared<bool>(false);
        if (!m.callback)
            return;

        connect(
            dialogButton, &QDialogButtonBox::accepted, [did, m] {
                m.callback(true);
                *did = true;
            });
        connect(
            dialogButton, &QDialogButtonBox::rejected, [did, m] {
                m.callback(false);
                *did = true;
            });

        // Do not connect to destroyed(): during app shutdown the callback target may already be dead.
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
        // raise();
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
        centralWidget->setStyleSheet(
            QString("QWidget { background-color: %1; border-radius: 22px; }"
                    "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; padding: 8px 10px; border-radius: 12px; }")
                .arg(theme.colors.background.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.accent.data()));

        title->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; }")
                .arg(theme.colors.accent.data()));
        msg->setStyleSheet(
            QString("QLabel { color: %1; border-radius: 30%; }")
                .arg(theme.colors.text.data()));
        dialogButton->setStyleSheet(
            QString("QDialogButtonBox { background-color: %1; border-radius: 14px; padding: 6px 10px; }"
                    "QPushButton { color: %2; background-color: %3; border-radius: 14px; padding: 10px 18px; }"
                    "QPushButton:hover { background-color: %4; }"
                    "QPushButton:disabled { background-color: %5; }")
                .arg(theme.colors.primary.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.secondary.data())
                .arg(theme.colors.hover.data())
                .arg(theme.colors.disabled.data()));
    }

} // namespace neko::ui::dialog
