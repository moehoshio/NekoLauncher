#include "neko/ui/pages/loadingPage.hpp"

#include <QtGui/QMovie>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace neko::ui {
    LoadingPage::LoadingPage(QWidget *parent)
        : QWidget(parent),
          progressBar(new QProgressBar(this)),
          poster(new PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
          textLayoutWidget(new QWidget(this)),
          textLayout(new QVBoxLayout(textLayoutWidget)),
          h1Title(new QLabel(this)),
          h2Title(new QLabel(this)),
          text(new QLabel(this)),
          loadingLabel(new QLabel(this)),
          loadingMv(new QMovie("img/loading.gif", QByteArray(), this)),
          process(new QLabel(this)) {

        textLayout->addWidget(h1Title);
        textLayout->addWidget(h2Title);
        textLayout->addWidget(text);
        textLayoutWidget->setLayout(textLayout);

        progressBar->setTextVisible(true);
        progressBar->setAlignment(Qt::AlignCenter);
        progressBar->setOrientation(Qt::Horizontal);
        progressBar->setInvertedAppearance(false);
        progressBar->setFormat("%v/%m");

        text->setWordWrap(true);
        process->setWordWrap(true);
        text->setOpenExternalLinks(true);
        
        loadingLabel->setAttribute(Qt::WA_NoSystemBackground);
        loadingLabel->setMovie(loadingMv);
        loadingLabel->setScaledContents(true);
        poster->lower();

        for (auto it : std::vector<QLabel *>{h1Title, h2Title, text}) {
            it->setAlignment(Qt::AlignCenter);
        }
        
        connect(this, &LoadingPage::setLoadingValD, this, &LoadingPage::setLoadingVal);
        connect(this, &LoadingPage::setLoadingNowD, this, &LoadingPage::setLoadingNow);
    }

    void LoadingPage::showLoad(const LoadMsg &m) {
        this->show();
        process->setText(QString::fromStdString(m.process));
        loadingMv->setFileName(QString::fromStdString(m.icon));
        loadingMv->start();

        if (m.type == LoadMsg::Type::Text || m.type == LoadMsg::Type::All) {
            h1Title->setText(QString::fromStdString(m.h1));
            h2Title->setText(QString::fromStdString(m.h2));
            text->setText(QString::fromStdString(m.message));
        }

        if (m.type == LoadMsg::Type::Progress || m.type == LoadMsg::Type::All) {
            progressBar->setMaximum(m.progressMax);
            progressBar->setValue(m.progressVal);
        }

        if (!m.poster.empty()) {
            (void)poster->setPixmap(QString::fromStdString(m.poster));
            poster->show();
        } else {
            poster->hide();
        }

        if (loadingMv->speed() != m.speed)
            loadingMv->setSpeed(m.speed);

        switch (m.type) {
            case LoadMsg::Type::Text:
                progressBar->hide();
                textLayoutWidget->show();
                break;
            case LoadMsg::Type::Progress:
                progressBar->show();
                textLayoutWidget->hide();
                break;
            case LoadMsg::Type::OnlyRaw:
                progressBar->hide();
                textLayoutWidget->hide();
                break;
            case LoadMsg::Type::All:
                progressBar->show();
                textLayoutWidget->show();
                break;
            default:
                break;
        }
    }

    void LoadingPage::hideLoad() {
        this->hide();
        loadingMv->stop();

        h1Title->clear();
        h2Title->clear();
        text->clear();
        process->clear();

        poster->clearPixmap();

        progressBar->hide();
        textLayoutWidget->hide();
    }
    void LoadingPage::setupStyle(const Theme &theme) {
        // Set background color for the main widget
        this->setStyleSheet(
            QString("background-color: %1;")
            .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));

        // Poster uses the same background color
        this->poster->setStyleSheet(
            QString("background-color: %1;")
            .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));

        // ProgressBar uses accent color for chunk, background for base, border color for border
        this->progressBar->setStyleSheet(
            QString(
            "QProgressBar {"
            " background-color: %1;"
            " border: 1px solid %2;"
            " border-radius: %3;"
            " color: %4;"
            " }"
            "QProgressBar::chunk {"
            " background-color: %5;"
            " }"
            )
            .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size()))
            .arg(QString::fromUtf8(theme.borderColor.data(), theme.borderColor.size()))
            .arg(theme.borderRadius)
            .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size()))
            .arg(QString::fromUtf8(theme.accentColor.data(), theme.accentColor.size()))
        );

        // Text layout widget uses background color
        this->textLayoutWidget->setStyleSheet(
            QString("background-color: %1;")
            .arg(QString::fromUtf8(theme.backgroundColor.data(), theme.backgroundColor.size())));

        // Loading label uses transparent background
        this->loadingLabel->setStyleSheet(
            QString("background-color: transparent;"));


        // Set the style for the text labels
        for (auto label : std::vector<QLabel *>{this->h1Title, this->h2Title, this->text, this->process}) {
            label->setStyleSheet(
                QString("color: %1;")
                .arg(QString::fromUtf8(theme.textColor.data(), theme.textColor.size())));
        }
    }

    void LoadingPage::setupFont(QFont text, QFont h1Font, QFont h2Font) {
        this->text->setFont(text);
        this->h1Title->setFont(h1Font);
        this->h2Title->setFont(h2Font);
        this->process->setFont(text);
    }

    void LoadingPage::resizeItems(int windowWidth, int windowHeight) {
        if (!isVisible())
            return;
        
        resize(windowWidth, windowHeight);

        // Resize poster
        if (poster) {
            poster->setGeometry(0, 0, windowWidth, windowHeight);
        }

        // Resize text layout widget
        if (textLayoutWidget) {
            int cw = windowWidth * 0.4;
            int ch = windowHeight * 0.4;
            int cx = (windowWidth - cw) / 2;
            int cy = (windowHeight - ch) / 2;
            textLayoutWidget->setGeometry(cx, cy, cw, ch);
        }

        // Resize progress bar
        if (progressBar) {
            progressBar->setGeometry(windowWidth * 0.3, windowHeight * 0.85, windowWidth *0.08, windowHeight *0.08);
        }

        // Resize loading label
        if (loadingLabel) {
            loadingLabel->setGeometry(3, windowHeight * 0.8, windowWidth *0.08, windowHeight * 0.08);
        }

        if (process) {
            process->setGeometry(5, windowHeight * 0.9, windowWidth * 0.3, windowHeight * 0.05);
        }
    }

} // namespace neko::ui

#include "loadingPage.moc"
