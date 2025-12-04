#pragma once

#include <neko/schema/types.hpp>

#include "neko/ui/uiMsg.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/widgets/pixmapWidget.hpp"


#include <QtWidgets/QWidget>

#include <string>


class QProgressBar;
class QMovie;
class QLabel;
class QVBoxLayout;

namespace neko::ui::page {

    class LoadingPage : public QWidget {
        Q_OBJECT
    private:
        QProgressBar *progressBar;
        widget::PixmapWidget *poster;
        QWidget *textLayoutWidget;
        QVBoxLayout *textLayout;
        QLabel *h1Title;
        QLabel *h2Title;
        QLabel *text;
        QLabel *loadingLabel;
        QMovie *loadingMv;
        QLabel *process;

    public:
        LoadingPage(QWidget *parent = nullptr);

        void showLoading(const LoadingMsg &m);
        void hideLoading();
        
        void setLoadingValue(neko::uint32 val);
        void setLoadingStatus(const std::string& msg);

        void setupTheme(const Theme &theme);
        void setupFont(QFont text, QFont h1Font, QFont h2Font);
        void resizeItems(int windowWidth, int windowHeight);

    signals:
        void setLoadingValueD(neko::uint32 val);
        void setLoadingStatusD(const std::string& msg);
    };
} // namespace neko::ui::page
