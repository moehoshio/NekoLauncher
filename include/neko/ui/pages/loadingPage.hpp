#pragma once

#include <neko/schema/types.hpp>

#include "neko/ui/uiMsg.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"

#include <QtGui/QMovie>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include <string>

namespace neko::ui {

    class LoadingPage : public QWidget {
        Q_OBJECT
    private:
        QProgressBar *progressBar;
        PixmapWidget *poster;
        QWidget *textLayoutWidget;
        QVBoxLayout *textLayout;
        QLabel *h1Title;
        QLabel *h2TitleH2;
        QLabel *text;
        QLabel *loadingLabel;
        QMovie *loadingMv;
        QLabel *process;

    public:
        LoadingPage(QWidget *parent = nullptr);

        void showLoad(const LoadMsg &m);

        void setLoadingVal(neko::uint32 val) {
            progressBar->setValue(val);
        };
        void setLoadingNow(const std::string& msg) {
            process->setText(QString::fromStdString(msg));
        };

        void hideLoad();

        void setupStyle(const Theme &theme);
        void setupFont(QFont text, QFont h1Font, QFont h2Font);
        void resizeItems(int windowWidth, int windowHeight);

    signals:
        void setLoadingValD(neko::uint32 val);
        void setLoadingNowD(const std::string& msg);
    };
} // namespace neko::ui
