#include "neko/ui/pages/aboutPage.hpp"

#include "neko/app/appinfo.hpp"
#include "neko/app/lang.hpp"
#include "neko/core/feedback.hpp"
#include "neko/bus/threadBus.hpp"
#include <neko/log/nlog.hpp>

#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>
#include <QtCore/QPointer>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMessageBox>


#include <algorithm>
#include <vector>

namespace neko::ui::page {

    AboutPage::AboutPage(QWidget *parent)
        : QWidget(parent),
          card(new QWidget(this)),
          titleLabel(new QLabel(card)),
          subtitleLabel(new QLabel(card)),
          versionLabel(new QLabel(card)),
          resourceLabel(new QLabel(card)),
          buildLabel(new QLabel(card)),
          descriptionLabel(new QLabel(card)),
          repoButton(new QPushButton(card)),
          feedbackButton(new QPushButton(card)),
          backButton(new QPushButton(card)) {

        this->setAttribute(Qt::WA_TranslucentBackground);
                card->setObjectName(QStringLiteral("aboutCard"));
        card->setAttribute(Qt::WA_StyledBackground);

        auto *layout = new QVBoxLayout(card);
        layout->setContentsMargins(28, 28, 28, 24);
        layout->setSpacing(10);

        titleLabel->setAlignment(Qt::AlignCenter);
        subtitleLabel->setAlignment(Qt::AlignCenter);
        versionLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        resourceLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        buildLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setOpenExternalLinks(true);

        layout->addWidget(titleLabel);
        layout->addWidget(subtitleLabel);
        layout->addSpacing(4);
        layout->addWidget(versionLabel);
        layout->addWidget(resourceLabel);
        layout->addWidget(buildLabel);
        layout->addSpacing(8);
        layout->addWidget(descriptionLabel);
        layout->addSpacing(6);
        layout->addStretch();

        auto *btnRow = new QHBoxLayout();
        btnRow->setContentsMargins(0, 0, 0, 0);
        btnRow->setSpacing(8);
        btnRow->addStretch();
        btnRow->addWidget(repoButton);
        btnRow->addWidget(feedbackButton);
        btnRow->addWidget(backButton);
        layout->addLayout(btnRow);

        backButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
        repoButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);
        feedbackButton->setFocusPolicy(Qt::FocusPolicy::TabFocus);

        setupText();

        connect(backButton, &QPushButton::clicked, this, &AboutPage::backRequested);
        connect(repoButton, &QPushButton::clicked, this, []() {
            QDesktopServices::openUrl(QUrl("https://github.com/moehoshio/NekoLauncher"));
        });
        connect(feedbackButton, &QPushButton::clicked, this, [this]() {
            feedbackButton->setEnabled(false);
            const std::string payload =
                std::string{"User triggered feedback log from About page.\n"}
                + "core: " + app::getVersion() + "\n"
                + "resource: " + app::getResourceVersion() + "\n"
                + "build: " + app::getBuildId() + "\n";

            QPointer<AboutPage> self(this);
            bus::thread::submit([self, payload]() {
                bool ok = false;
                std::string errMsg;
                try {
                    core::feedbackLog(payload);
                    ok = true;
                } catch (const ex::Exception &e) {
                    errMsg = e.what();
                }

                if (!self) {
                    return;
                }
                QMetaObject::invokeMethod(self, [self, ok, errMsg]() {
                    if (!self) {
                        return;
                    }
                    self->feedbackButton->setEnabled(true);
                    if (ok) {
                        QMessageBox::information(self, self->tr("Feedback"), self->tr("Logs sent successfully."));
                    } else {
                        log::error("feedbackLog failed: {}", {}, errMsg);
                        QMessageBox::warning(self, self->tr("Feedback"), QString::fromStdString(std::string("Failed to send logs: ") + errMsg));
                    }
                }, Qt::QueuedConnection);
            });
        });
    }

    void AboutPage::setupTheme(const Theme &theme) {
        const auto cardBg = theme.colors.panel.empty() ? theme.colors.surface : theme.colors.panel;
        card->setStyleSheet(
            QString(
                "QWidget#aboutCard {"
                "background: %1;"
                "border: 1.5px solid %2;"
                "border-radius: 18px;"
                "}")
                .arg(cardBg.data())
                .arg(theme.colors.accent.data()));

        const QString labelStyle =
            QString(
                "QLabel {"
                "color: %1;"
                "background-color: transparent;"
                "border: none;"
                "}")
                .arg(theme.colors.text.data());

        for (auto *label : std::vector<QLabel *>{titleLabel, subtitleLabel, versionLabel, resourceLabel, buildLabel, descriptionLabel}) {
            label->setStyleSheet(labelStyle);
        }

        const QString buttonStyle =
            QString(
                "QPushButton {"
                "background-color: %1;"
                "color: %2;"
                "border: none;"
                "border-radius: 12px;"
                "padding: 8px 16px;"
                "font-weight: 600;"
                "}"
                "QPushButton:hover {"
                "background-color: %3;"
                "}")
                .arg(theme.colors.secondary.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.hover.data());
        for (auto *btn : std::vector<QPushButton *>{repoButton, feedbackButton, backButton}) {
            btn->setStyleSheet(buttonStyle);
        }
    }

    void AboutPage::setupFont(QFont text, QFont h1Font, QFont h2Font) {
        titleLabel->setFont(h1Font);
        subtitleLabel->setFont(h2Font);
        versionLabel->setFont(text);
        resourceLabel->setFont(text);
        buildLabel->setFont(text);
        descriptionLabel->setFont(text);
        repoButton->setFont(h2Font);
        feedbackButton->setFont(h2Font);
        backButton->setFont(h2Font);
    }

    void AboutPage::setupText() {
        titleLabel->setText(QString::fromStdString(lang::tr(lang::keys::about::category, lang::keys::about::title, app::getAppName())));
        subtitleLabel->setText(QString::fromStdString(lang::tr(lang::keys::about::category, lang::keys::about::tagline, "A calm Minecraft launcher")));
        versionLabel->setText(QString("core: %1").arg(app::getVersion()));
        resourceLabel->setText(QString("resource: %1").arg(QString::fromStdString(app::getResourceVersion())));
        buildLabel->setText(QString("build: %1").arg(app::getBuildId()));
        descriptionLabel->setText(QString::fromStdString(lang::tr(lang::keys::about::category, lang::keys::about::description, "Open source launcher built with Qt 6.")));
        repoButton->setText(QString::fromStdString(lang::tr(lang::keys::about::category, lang::keys::about::openRepo, "Repository")));
        feedbackButton->setText(QString::fromStdString(lang::tr(lang::keys::about::category, lang::keys::about::feedbackLogs, "Send Logs")));
        backButton->setText(QString::fromStdString(lang::tr(lang::keys::button::category, lang::keys::button::close, "Close")));
    }

    void AboutPage::resizeItems(int windowWidth, int windowHeight) {
        resize(windowWidth, windowHeight);

        const int cardWidth = std::max<int>(static_cast<int>(windowWidth * 0.6), 420);
        const int cardHeight = std::max<int>(static_cast<int>(windowHeight * 0.55), 360);
        const int cardX = (windowWidth - cardWidth) / 2;
        const int cardY = (windowHeight - cardHeight) / 2;
        card->setGeometry(cardX, cardY, cardWidth, cardHeight);
    }

} // namespace neko::ui::page
