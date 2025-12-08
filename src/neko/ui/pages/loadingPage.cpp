
#include "neko/ui/pages/loadingPage.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtGui/QMovie>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace neko::ui::page {

    LoadingPage::LoadingPage(QWidget *parent)
        : QWidget(parent),
          progressBar(new QProgressBar(this)),
          poster(new widget::PixmapWidget(Qt::KeepAspectRatioByExpanding, this)),
          textLayoutWidget(new QWidget(this)),
          textLayout(new QVBoxLayout(textLayoutWidget)),
          h1Title(new QLabel(this)),
          h2Title(new QLabel(this)),
          text(new QLabel(this)),
          loadingLabel(new QLabel(this)),
          loadingMv(new QMovie("img/loading.gif", QByteArray(), this)),
          process(new QLabel(this)) {

        this->setAttribute(Qt::WA_TranslucentBackground);

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

        connect(this, &LoadingPage::setLoadingValueD, this, &LoadingPage::setLoadingValue);
        connect(this, &LoadingPage::setLoadingStatusD, this, &LoadingPage::setLoadingStatus);
    }

    void LoadingPage::showLoading(const LoadingMsg &m) {
        this->show();
        process->setText(QString::fromStdString(m.process));
        loadingMv->setFileName(QString::fromStdString(m.loadingIconPath));
        loadingMv->start();

        if (m.type == LoadingMsg::Type::Text || m.type == LoadingMsg::Type::All) {
            h1Title->setText(QString::fromStdString(m.h1));
            h2Title->setText(QString::fromStdString(m.h2));
            text->setText(QString::fromStdString(m.message));
        }

        if (m.type == LoadingMsg::Type::Progress || m.type == LoadingMsg::Type::All) {
            progressBar->setMaximum(m.progressMax);
            progressBar->setValue(m.progressVal);
        }

        if (!m.posterPath.empty()) {
            (void)poster->setPixmap(QString::fromStdString(m.posterPath));
            poster->show();
        } else {
            poster->hide();
        }

        if (loadingMv->speed() != m.speed)
            loadingMv->setSpeed(m.speed);

        switch (m.type) {
            case LoadingMsg::Type::Text:
                progressBar->hide();
                textLayoutWidget->show();
                break;
            case LoadingMsg::Type::Progress:
                progressBar->show();
                textLayoutWidget->hide();
                break;
            case LoadingMsg::Type::OnlyRaw:
                progressBar->hide();
                textLayoutWidget->hide();
                break;
            case LoadingMsg::Type::All:
                progressBar->show();
                textLayoutWidget->show();
                break;
            default:
                break;
        }
    }

    void LoadingPage::hideLoading() {
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

    void LoadingPage::setLoadingValue(neko::uint32 val) {
        progressBar->setValue(val);
    };
    void LoadingPage::setLoadingStatus(const std::string &msg) {
        process->setText(QString::fromStdString(msg));
    };

    void LoadingPage::setupTheme(const Theme &theme) {

        QString textLayoutStyle =
            QString(
                "background-color: %1;"
                "border: 1px solid %2;"
                "border-radius: 18px;")
                .arg(theme.colors.surface.data())
                .arg(theme.colors.accent.data());
        textLayoutWidget->setStyleSheet(textLayoutStyle);
        QString progressBarStyle =
            QString(
                "QProgressBar {"
                "background-color: %1;"
                "color: %2;"
                "border: 1px solid %3;"
                "border-radius: 10px;"
                "padding: 2px 6px;"
                "}"
                "QProgressBar::chunk {"
                "background-color: %4;"
                "border-radius: 8px;"
                "}")
                .arg(theme.colors.surface.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.accent.data())
                .arg(theme.colors.primary.data());

        progressBar->setStyleSheet(progressBarStyle);

        QString labelStyle =
            QString(
                "QLabel {"
                "color: %1;"
                "background-color: transparent;"
                "}")
                .arg(theme.colors.text.data());

        for (auto it : std::vector<QLabel *>{h1Title, h2Title, text, loadingLabel, process}) {
            it->setStyleSheet(labelStyle);
        }


    }

    void LoadingPage::setupFont(QFont text, QFont h1Font, QFont h2Font) {
        this->text->setFont(text);
        this->h1Title->setFont(h1Font);
        this->h2Title->setFont(h2Font);
        this->process->setFont(text);
    }

    void LoadingPage::resizeItems(int windowWidth, int windowHeight) {

        resize(windowWidth, windowHeight);

        poster->setGeometry(0, 0, windowWidth, windowHeight);
        int cw = windowWidth * 0.4;
        int ch = windowHeight * 0.4;
        int cx = (windowWidth - cw) / 2;
        int cy = (windowHeight - ch) / 2;
        textLayoutWidget->setGeometry(cx, cy, cw, ch);

        progressBar->setGeometry(windowWidth * 0.3, windowHeight * 0.82, windowWidth * 0.4, windowHeight * 0.1);
        loadingLabel->setGeometry(3, windowHeight * 0.8, windowWidth * 0.08, windowHeight * 0.12);

        process->setGeometry(5, windowHeight * 0.9, windowWidth * 0.3, windowHeight * 0.05);
    }

} // namespace neko::ui::page
