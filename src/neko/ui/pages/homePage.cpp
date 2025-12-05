#include "neko/ui/pages/homePage.hpp"
#include "neko/app/appinfo.hpp"
#include "neko/app/lang.hpp"

#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>

namespace neko::ui::page {
    HomePage::HomePage(QWidget *parent)
        : QWidget(parent),
          startButton(new QPushButton(this)),
          menuButton(new QPushButton(this)),
          versionButton(new QPushButton(this)) {
            
        this->setAttribute(Qt::WA_TranslucentBackground);
        startButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
        menuButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
        versionButton->setFocusPolicy(Qt::FocusPolicy::NoFocus);
        setupText();
    }
    void HomePage::setupTheme(const Theme &theme) {
        QString startButtonStyle =
            QString(
                "QPushButton {"
                "background-color: %1;"
                "color: %2;"
                "border: none;"
                "border-radius: 30%;"
                "}"
                "QPushButton:hover {"
                "background-color: %3;"
                "}")
                .arg(theme.colors.primary.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.hover.data());
        QString menuButtonStyle =
            QString(
                "QPushButton {"
                "background-color: %1;"
                "color: %2;"
                "border: none;"
                "border-radius: 20%;"
                "}"
                "QPushButton:hover {"
                "background-color: %3;"
                "}")
                .arg(theme.colors.secondary.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.hover.data());

        startButton->setStyleSheet(startButtonStyle);
        menuButton->setStyleSheet(menuButtonStyle);

        versionButton->setStyleSheet(
            QString(
                "QPushButton {"
                "background-color: transparent;"
                "color: %1;"
                "border: none;"
                "text-align: left;"
                "}"
                "QPushButton:hover {"
                "background-color: transparent;"
                "}")
                .arg(theme.colors.text.data()));
    }
    void HomePage::setupFont(QFont text, QFont h1Font, QFont h2Font) {
        startButton->setFont(h1Font);
        menuButton->setFont(h2Font);
        versionButton->setFont(text);
    }
    void HomePage::setupText() {
        startButton->setText(QString::fromStdString(lang::tr(lang::keys::button::category, lang::keys::button::start, "Start")));
        menuButton->setText(QString::fromStdString(lang::tr(lang::keys::button::category, lang::keys::button::menu, "Menu")));

        versionButton->setText(
            QString("core: %1\nresource: %2")
                .arg(app::getVersion())
                .arg(QString::fromStdString(app::getResourceVersion())));
    }
    void HomePage::resizeItems(int windowWidth, int windowHeight) {
        resize(windowWidth, windowHeight);

        startButton->setGeometry(windowWidth * 0.3, windowHeight * 0.75, windowWidth * 0.4, windowHeight * 0.18);
        menuButton->setGeometry(windowWidth * 0.82, windowHeight * 0.82, windowWidth * 0.15, windowHeight * 0.1);
        versionButton->setGeometry(10, windowHeight - std::max<double>(windowHeight * 0.01, 60), std::max<double>(width() * 0.12, 180), std::max<double>(height() * 0.1, 65));
    }
} // namespace neko::ui::page
