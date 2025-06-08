#pragma once

#include "neko/ui/widgets/pixmapWidget.hpp"
#include "neko/ui/uiMsg.hpp"

#include <QtGui/QMovie>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace neko::ui {

    struct LoadingPage : public QWidget {
        LoadingPage(QWidget *parent = nullptr);
        QProgressBar *progressBar;
        PixmapWidget *poster;
        QWidget *textLayoutWidget;
        QVBoxLayout *textLayout;
        QLabel *titleH1;
        QLabel *titleH2;
        QLabel *text;
        QLabel *loadingLabel;
        QMovie *loadingMv;
        QLabel *process;

        inline void showLoad(const loadMsg &m) {
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
} // namespace neko::ui
