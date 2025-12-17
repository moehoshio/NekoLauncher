#pragma once

#include "neko/ui/theme.hpp"

#include <QtGui/QFont>
#include <QtWidgets/QWidget>

class QLabel;
class QPushButton;
class QWidget;

namespace neko::ui::page {

    class AboutPage : public QWidget {
        Q_OBJECT
    private:
        QWidget *card;
        QLabel *titleLabel;
        QLabel *subtitleLabel;
        QLabel *versionLabel;
        QLabel *resourceLabel;
        QLabel *buildLabel;
        QLabel *descriptionLabel;
        QPushButton *repoButton;
        QPushButton *feedbackButton;
        QPushButton *backButton;

    public:
        explicit AboutPage(QWidget *parent = nullptr);

        void setupTheme(const Theme &theme);
        void setupFont(QFont text, QFont h1Font, QFont h2Font);
        void setupText();
        void resizeItems(int windowWidth, int windowHeight);

    signals:
        void backRequested();
    };

} // namespace neko::ui::page
