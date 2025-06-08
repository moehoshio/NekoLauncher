#pragma once

#include "neko/ui/uiMsg.hpp"

#include "neko/ui/widgets/pixmapWidget.hpp"

#include "neko/function/info.hpp"

#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace neko::ui {
    
    struct InputDialog : public QWidget {
        QGridLayout *gridLayout;
        QWidget *centralWidget;
        PixmapWidget *poster;
        QVBoxLayout *centralWidgetLayout;
        std::vector<QWidget *> lineWidgets;
        std::vector<QMetaObject::Connection> conns;
        QDialogButtonBox *dialogButton;
        QLabel *title;
        QLabel *msg;
        InputDialog(QWidget *parent = nullptr);

        inline void hideInput() {
            this->hide();
            for (const auto &it : conns) {
                disconnect(it);
            }
            conns.clear();
        }

        inline void showInput(const InputMsg &m) {
            this->show();
            title->setText(m.title.c_str());
            msg->setText(m.msg.c_str());
            if (!m.poster.empty()) {
                poster->setPixmap(m.poster.c_str());
                poster->show();
            } else {
                poster->hide();
            }

            setLines(m.lines);
            bool *did = new bool(false);
            if (!m.callback)
                return;

            conns.push_back(connect(
                dialogButton, &QDialogButtonBox::accepted, [=, this] {
                    m.callback(true);
                    disconnect(dialogButton);
                    *did = true;
                }));
            conns.push_back(connect(
                dialogButton, &QDialogButtonBox::rejected, [=, this] {
                    m.callback(false);
                    disconnect(dialogButton);
                    *did = true;
                }));

            conns.push_back(connect(this, &QWidget::destroyed, [=](QObject *) {
                if (!(*did))
                    m.callback(false);
            }));
        }

        inline void setLines(const std::vector<std::string> vec) {
            std::vector<QWidget *> lines;
            for (auto &it : vec) {
                QLineEdit *line = new QLineEdit(centralWidget);
                line->setMinimumHeight(50);
                line->setPlaceholderText(it.c_str());
                if (it == neko::info::translations(neko::info::lang.general.password)) {
                    line->setEchoMode(QLineEdit::PasswordEchoOnEdit);
                }
                lines.push_back(line);
            }
            setLines(lines);
        }

        inline void setLines(const std::vector<QWidget *> &placeholder) {
            for (auto it : lineWidgets) {
                centralWidgetLayout->removeWidget(it);
                it->hide();
                delete it;
            }
            lineWidgets.clear();

            for (size_t i = 0; i < placeholder.size(); ++i) {
                lineWidgets.push_back(placeholder[i]);
                centralWidgetLayout->addWidget(lineWidgets[i]);
            }

            centralWidgetLayout->addWidget(dialogButton);
        }

        inline std::vector<std::string> getLines() {
            std::vector<std::string> vec;
            for (auto it : lineWidgets) {

                if (auto line = dynamic_cast<QLineEdit *>(it)) {
                    if (!line->text().isEmpty()) {
                        vec.push_back(line->text().toStdString());
                    }
                }
            }
            return vec;
        }
    };
} // namespace neko::ui
