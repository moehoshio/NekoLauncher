#pragma once

#include "neko/app/api.hpp"
#include "neko/ui/theme.hpp"

#include <QtWidgets/QWidget>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFrame>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>

#include <vector>
#include <string>
#include <cstdint>

namespace neko::ui::page {

    /**
     * @brief Dismiss option for news page
     */
    enum class NewsDismissOption {
        None,           // Don't dismiss, show every time
        Days3,          // Don't show for 3 days
        Days7,          // Don't show for 7 days
        UntilUpdate     // Don't show until next version update
    };

    /**
     * @brief A single news item widget for the news page
     */
    class NewsPageItem : public QFrame {
        Q_OBJECT

    public:
        explicit NewsPageItem(const api::NewsItem &item, QWidget *parent = nullptr);
        void setupTheme(const Theme &theme);

    signals:
        void clicked(const api::NewsItem &item);
        void linkClicked(const QString &link);

    protected:
        void mousePressEvent(QMouseEvent *event) override;

    private:
        api::NewsItem newsItem;
        QLabel *titleLabel;
        QLabel *summaryLabel;
        QLabel *contentLabel;
        QLabel *dateLabel;
        QLabel *categoryLabel;
    };

    /**
     * @brief Full-screen news page shown after loading completes
     */
    class NewsPage : public QWidget {
        Q_OBJECT

    public:
        explicit NewsPage(QWidget *parent = nullptr);

        void setNews(const std::vector<api::NewsItem> &items);
        void clearNews();

        void setupTheme(const Theme &theme);
        void setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font);
        void setupText();
        void resizeItems(int windowWidth, int windowHeight);

        // Get dismiss preferences
        NewsDismissOption getDismissOption() const;

    signals:
        void continueClicked();
        void dismissOptionChanged(NewsDismissOption option);
        void newsItemClicked(const api::NewsItem &item);

    private slots:
        void onContinueClicked();
        void onDismissOptionChanged(int index);

    private:
        void setupUi();
        void updateNewsItems();

        Theme currentTheme;
        std::vector<api::NewsItem> newsItems;
        std::vector<NewsPageItem*> newsWidgets;
        NewsDismissOption currentDismissOption = NewsDismissOption::None;

        // Header
        QWidget *headerWidget;
        QLabel *titleLabel;
        
        // Content area
        QScrollArea *scrollArea;
        QWidget *contentWidget;
        QVBoxLayout *contentLayout;

        // Footer with controls
        QWidget *footerWidget;
        QComboBox *dismissComboBox;
        QPushButton *continueButton;
    };

} // namespace neko::ui::page
