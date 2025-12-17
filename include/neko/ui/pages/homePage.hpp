#pragma once

#include "neko/ui/theme.hpp"

#include <QtWidgets/QWidget>
#include <QtGui/QFont>

class QPushButton;

namespace neko::ui::page {
    
    class HomePage : public QWidget {
        Q_OBJECT
    private:
        QPushButton *startButton;
        QPushButton *menuButton;
        QPushButton *versionButton;
    public:
        HomePage(QWidget *parent = nullptr);

        void setupTheme(const Theme &theme);
        void setupText();
        void setupFont(QFont text, QFont h1Font, QFont h2Font);

        void resizeItems(int windowWidth, int windowHeight);

    signals:
        void menuButtonClicked();
        void startButtonClicked();
        void versionButtonClicked();

    };
} // namespace neko::ui::page
