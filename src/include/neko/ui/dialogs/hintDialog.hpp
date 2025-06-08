#pragma once

#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <memory>
#include <string_view>

namespace neko::ui {

    struct HintDialog : public QWidget {

        QWidget *centralWidget;
        PixmapWidget *poster;
        QVBoxLayout *centralWidgetLayout;
        QFrame *line;
        QPushButton *button;
        QDialogButtonBox *dialogButton;
        QLabel *title;
        QLabel *msg;

        HintDialog(QWidget *parent = nullptr);

        void setupButton(QPushButton *btn, const std::function<void(bool)> &callback, std::shared_ptr<bool> did) {
            btn->show();
            if (!callback)
                return;
            connect(btn, &QPushButton::clicked, this, [=, this] {
                if (!*did) {
                    callback(true);
                    *did = true;
                }
            });
        }

        void setupButton(QDialogButtonBox *btnBox, const std::function<void(bool)> &callback, std::shared_ptr<bool> did) {
            btnBox->show();
            if (!callback)
                return;
            connect(btnBox, &QDialogButtonBox::accepted, this, [=, this] {
                if (!*did) {
                    callback(true);
                    *did = true;
                }
            });
            connect(btnBox, &QDialogButtonBox::rejected, this, [=, this] {
                if (!*did) {
                    callback(false);
                    *did = true;
                }
            });
        }

        void showHint(const hintMsg &m) {
            this->show();
            this->title->setText(QString::fromStdString(m.title));
            this->msg->setText(QString::fromStdString(m.msg));
            if (poster && !m.poster.empty())
                this->poster->setPixmap(std::string_view(m.poster));
            auto did = std::make_shared<bool>(false);
            if (m.buttonNum == 1) {
                setupButton(button, m.callback, did);
                dialogButton->hide();
            } else {
                button->hide();
                setupButton(dialogButton, m.callback, did);
            }
            if (!m.callback)
                return;
            connect(this, &QWidget::destroyed, this, [m, did](QObject *) {
                if (!*did && m.callback)
                    m.callback(false);
            });
        }
    };
} // namespace neko::ui
