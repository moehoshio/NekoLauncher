#include "neko/ui/pages/homePage.hpp"
#include "neko/function/info.hpp"

namespace neko::ui {
    HomePage::HomePage(QWidget *parent) : QWidget(parent),
                                          startButton(new QPushButton(this)),
                                          menuButton(new QPushButton(this)),
                                          versionText(new QLabel(this)) {
        versionText->setWordWrap(true);
        startButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
        menuButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
    }
    void HomePage::setupStyle(const Theme &theme) {
        this->setStyleSheet(QString("background-color: %1;")
                                .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));

        startButton->setStyleSheet(QString("color: %1;")
                                       .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        menuButton->setStyleSheet(QString("color: %1;")
                                      .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        versionText->setStyleSheet(QString("color: %1;")
                                       .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
    }
    void HomePage::setupFont(QFont text, QFont h1Font, QFont h2Font) {
        startButton->setFont(h1Font);
        menuButton->setFont(h2Font);
        versionText->setFont(text);
    }
    void HomePage::setupText() {
        startButton->setText(QString::fromStdString(neko::info::tr(neko::info::lang.general.start)));
        menuButton->setText(QString::fromStdString(neko::info::tr(neko::info::lang.general.menu)));

        versionText->setText(
            QString("core: %1\nres: %2")
            .arg(QString::fromStdString(neko::info::getVersion()))
            .arg(QString::fromStdString(neko::info::getResVersion()))
        );
    }
    void HomePage::resizeItems(int windowWidth, int windowHeight) {
        if (!isVisible())
            return;

        resize(windowWidth, windowHeight);

        // Resize start button
        startButton->setGeometry(windowWidth * 0.3, windowHeight * 0.75, windowWidth * 0.4, windowHeight * 0.18);

        // Resize menu button
        menuButton->setGeometry(windowWidth * 0.82, windowHeight * 0.82, windowWidth * 0.15, windowHeight * 0.1);

        // Resize version text
        versionText->setGeometry(10, windowHeight - std::max<double>(windowHeight * 0.01, 60), std::max<double>(width() * 0.12, 180), std::max<double>(height() * 0.1, 65));
    }
} // namespace neko::ui
