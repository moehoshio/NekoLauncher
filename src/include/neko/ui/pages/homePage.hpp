#pragma once

#include <QtWidgets/QWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>

namespace neko::ui {
    
    class HomePage : public QWidget {
    private:
        QPushButton *startButton;
        QPushButton *menuButton;
        QLabel *versionText;
    public:
        HomePage(QWidget *parent = nullptr);

        void setupStyle(const Theme &theme);
        void setupText();

        void setupFont(QFont text, QFont h1Font, QFont h2Font);

        void resizeItems(int windowWidth, int windowHeight);

    };
} // namespace neko::ui
