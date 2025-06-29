#include "neko/ui/dialogs/inputDialog.hpp"

#include "neko/ui/theme.hpp"
#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/function/info.hpp"

namespace neko::ui {
    InputDialog::InputDialog(QWidget *parent)
        : QWidget(parent),
          gridLayout(new QGridLayout()),
          poster(new PixmapWidget(this)),
          centralWidget(new QWidget()),
          centralWidgetLayout(new QVBoxLayout(centralWidget)),
          dialogButton(new QDialogButtonBox(centralWidget)),
          title(new QLabel(centralWidget)),
          msg(new QLabel(centralWidget)) {
        gridLayout->setRowStretch(0, 3);
        gridLayout->setColumnStretch(0, 3);
        gridLayout->setRowStretch(1, 3);
        gridLayout->setColumnStretch(1, 3);
        gridLayout->setRowStretch(2, 3);
        gridLayout->setColumnStretch(2, 3);
        gridLayout->setColumnStretch(3, 3);
        gridLayout->setColumnStretch(4, 3);

        this->setLayout(gridLayout);
        gridLayout->addWidget(centralWidget, 1, 2);

        poster->lower();

        centralWidget->setSizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        title->setAlignment(Qt::AlignCenter);
        centralWidgetLayout->addWidget(title);
        centralWidgetLayout->addWidget(msg);
        centralWidgetLayout->setStretchFactor(msg, 0.5);
        centralWidgetLayout->addSpacing(12);
        centralWidget->setLayout(centralWidgetLayout);

        dialogButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        dialogButton->setStandardButtons(QDialogButtonBox::Cancel | QDialogButtonBox::Ok);
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
        disconnect(this, &QWidget::destroyed, nullptr, nullptr);
    }

    void InputDialog::showInput(const InputMsg &m) {
        this->show();
        title->setText(QString::fromStdString(m.title));
        msg->setText(QString::fromStdString(m.msg));
        if (!m.poster.empty())
            (void)poster->setPixmap(std::string_view(m.poster));

        if (m.lineText.empty()) {
            setLines({neko::info::translations(neko::info::lang.general.input)});
        } else {
            setLines(m.lineText);
        }
        auto did = std::make_shared<bool>(false);
        if (!m.callback)
            return;

        connect(
            dialogButton, &QDialogButtonBox::accepted, [=, this, did] {
                m.callback(true);
                *did = true;
            });
        connect(
            dialogButton, &QDialogButtonBox::rejected, [=, this, did] {
                m.callback(false);
                *did = true;
            });

        connect(this, &QWidget::destroyed, [=, did](QObject *) {
            if (!(*did))
                m.callback(false);
        });
    }

    void InputDialog::setLines(const std::vector<std::string> &vec) {
        for (auto &it : vec) {
            QLineEdit *line = new QLineEdit(centralWidget);
            line->setMinimumHeight(50);
            line->setPlaceholderText(QString::fromStdString(it));
            line->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            if (it == neko::info::translations(neko::info::lang.general.password)) {
                line->setEchoMode(QLineEdit::PasswordEchoOnEdit);
            }
            centralWidgetLayout->addWidget(line);
            lines.push_back(line);
        }
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
        if (!isVisible())
            return;
        resize(windowWidth, windowHeight);
        if (poster) {
            poster->setGeometry(0, 0, windowWidth, windowHeight);
        }

        if (centralWidget) {
            int cw = windowWidth * 0.5;
            int ch = windowHeight * 0.5;
            int cx = (windowWidth - cw) / 2;
            int cy = (windowHeight - ch) / 2;
            centralWidget->setGeometry(cx, cy, cw, ch);
        }

        gridLayout->setRowStretch(0, 3);
        gridLayout->setColumnStretch(0, 3);
        gridLayout->setRowStretch(1, 3);
        gridLayout->setColumnStretch(1, 3);
        gridLayout->setRowStretch(2, 3);
        gridLayout->setColumnStretch(2, 3);
        gridLayout->setColumnStretch(3, 3);
        gridLayout->setColumnStretch(4, 3);
    }

    void InputDialog::setupFont(QFont font, QFont titleFont) {
        this->setFont(font);
        centralWidget->setFont(font);
        msg->setFont(font);
        dialogButton->setFont(font);
        title->setFont(titleFont);
    }
    void InputDialog::setupStyle(const Theme &theme) {
        centralWidget->setStyleSheet(
            QString("QWidget { background-color: %1; }"
                    "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; padding: %4px; border-radius: %5; }")
                .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size()))
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size()))
                .arg(QString::fromUtf8(theme.borderColor.data(), theme.borderColor.size()))
                .arg(theme.padding)
                .arg(QString::fromUtf8(theme.borderRadius.data(), theme.borderRadius.size())));

        title->setStyleSheet(
            QString("QLabel { color: %1; font-weight: bold; }")
                .arg(QString::fromUtf8(theme.accentColor.data(), theme.accentColor.size())));
        msg->setStyleSheet(
            QString("QLabel { color: %1; }")
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        dialogButton->setStyleSheet(
            QString("QDialogButtonBox { background-color: %1; }"
                    "QPushButton { color: %2; background-color: %3; border-radius: %4; padding: %5; }"
                    "QPushButton:hover { background-color: %6; }"
                    "QPushButton:disabled { background-color: %7; }")
                .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size()))
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size()))
                .arg(QString::fromUtf8(theme.buttonColor.data(), theme.buttonColor.size()))
                .arg(theme.borderRadius)
                .arg(theme.padding)
                .arg(QString::fromUtf8(theme.buttonHoverColor.data(), theme.buttonHoverColor.size()))
                .arg(QString::fromUtf8(theme.buttonDisabledColor.data(), theme.buttonDisabledColor.size())));
    }
} // namespace neko::ui
