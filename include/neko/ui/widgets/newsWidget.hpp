#pragma once

#include "neko/app/api.hpp"
#include "neko/ui/theme.hpp"

#include <QtWidgets/QWidget>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>

#include <vector>
#include <functional>

namespace neko::ui::widget {

    /**
     * @brief A single news item widget that displays title, summary, and date.
     */
    class NewsItemWidget : public QFrame {
        Q_OBJECT

    public:
        explicit NewsItemWidget(const api::NewsItem &item, QWidget *parent = nullptr);

        void setupTheme(const Theme &theme);

    signals:
        void clicked(const api::NewsItem &item);
        void linkClicked(const QString &link);

    private:
        api::NewsItem newsItem;
        QLabel *titleLabel;
        QLabel *summaryLabel;
        QLabel *dateLabel;
        QLabel *categoryLabel;

    protected:
        void mousePressEvent(QMouseEvent *event) override;
    };

    /**
     * @brief A collapsible news panel widget that shows recent news items.
     */
    class NewsPanel : public QWidget {
        Q_OBJECT

    public:
        explicit NewsPanel(QWidget *parent = nullptr);

        void setNews(const std::vector<api::NewsItem> &items);
        void clearNews();
        void setExpanded(bool expanded);
        bool isExpanded() const;
        
        void setupTheme(const Theme &theme);
        void setupFont(const QFont &textFont, const QFont &titleFont);

    signals:
        void newsItemClicked(const api::NewsItem &item);
        void toggleExpanded(bool expanded);
        void loadMoreRequested();
        void closeRequested();

    private slots:
        void onToggleClicked();
        void onCloseClicked();

    private:
        void setupUi();
        void updateToggleButton();

        bool expanded = true;
        Theme currentTheme;

        QWidget *headerWidget;
        QPushButton *toggleButton;
        QPushButton *closeButton;
        QLabel *headerLabel;

        QScrollArea *scrollArea;
        QWidget *contentWidget;
        QVBoxLayout *contentLayout;

        std::vector<NewsItemWidget*> newsWidgets;
    };

} // namespace neko::ui::widget
