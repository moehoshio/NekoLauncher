#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

namespace neko::ui {
    struct Index : public QWidget {
        Index(QWidget *parent = nullptr) : QWidget(parent) {
            startButton = new QPushButton(this);
            menuButton = new QPushButton(this);
            versionText = new QLabel(this);
            versionText->setWordWrap(true);
            startButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
            menuButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
        };

        QPushButton *startButton;
        QPushButton *menuButton;
        QLabel *versionText;
    };
} // namespace neko::ui
