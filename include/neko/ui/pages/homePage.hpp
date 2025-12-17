#pragma once

#include "neko/app/api.hpp"
#include "neko/ui/theme.hpp"
#include "neko/ui/widgets/newsWidget.hpp"

#include <QtWidgets/QWidget>
#include <QtGui/QFont>

#include <vector>

class QPushButton;

namespace neko::ui::page {
    
    class HomePage : public QWidget {
        Q_OBJECT
    private:
        QPushButton *startButton;
        QPushButton *menuButton;
        QPushButton *versionButton;
        widget::NewsPanel *newsPanel;
        bool newsPanelVisible = true;

    public:
        HomePage(QWidget *parent = nullptr);

        void setupTheme(const Theme &theme);
        void setupText();
        void setupFont(QFont text, QFont h1Font, QFont h2Font);

        void resizeItems(int windowWidth, int windowHeight);

        // News panel methods
        void setNews(const std::vector<api::NewsItem> &items);
        void clearNews();
        void setNewsPanelVisible(bool visible);
        bool isNewsPanelVisible() const;

    signals:
        void menuButtonClicked();
        void startButtonClicked();
        void versionButtonClicked();
        void newsItemClicked(const api::NewsItem &item);
        void newsPanelClosed();

    };
} // namespace neko::ui::page
