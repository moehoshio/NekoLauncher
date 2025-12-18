#include "neko/ui/pages/newsPage.hpp"
#include "neko/app/lang.hpp"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>

namespace neko::ui::page {

    // NewsPageItem implementation
    NewsPageItem::NewsPageItem(const api::NewsItem &item, QWidget *parent)
        : QFrame(parent), newsItem(item) {
        
        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Raised);
        setCursor(Qt::PointingHandCursor);
        setObjectName("newsPageItem");

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(20, 16, 20, 16);
        layout->setSpacing(8);

        // Header row with title and category
        auto *headerLayout = new QHBoxLayout();
        headerLayout->setContentsMargins(0, 0, 0, 0);
        headerLayout->setSpacing(12);

        titleLabel = new QLabel(QString::fromStdString(item.title), this);
        titleLabel->setWordWrap(true);
        titleLabel->setObjectName("newsPageTitle");
        headerLayout->addWidget(titleLabel, 1);

        if (!item.category.empty()) {
            categoryLabel = new QLabel(QString::fromStdString(item.category), this);
            categoryLabel->setObjectName("newsPageCategory");
            categoryLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
            headerLayout->addWidget(categoryLabel);
        } else {
            categoryLabel = nullptr;
        }

        layout->addLayout(headerLayout);

        // Summary
        if (!item.summary.empty()) {
            summaryLabel = new QLabel(QString::fromStdString(item.summary), this);
            summaryLabel->setWordWrap(true);
            summaryLabel->setObjectName("newsPageSummary");
            layout->addWidget(summaryLabel);
        } else {
            summaryLabel = nullptr;
        }

        // Content (if available, show more detail)
        if (!item.content.empty()) {
            contentLabel = new QLabel(QString::fromStdString(item.content), this);
            contentLabel->setWordWrap(true);
            contentLabel->setObjectName("newsPageContent");
            contentLabel->setTextFormat(Qt::RichText);
            contentLabel->setOpenExternalLinks(true);
            layout->addWidget(contentLabel);
        } else {
            contentLabel = nullptr;
        }

        // Date
        if (!item.publishTime.empty()) {
            dateLabel = new QLabel(QString::fromStdString(item.publishTime), this);
            dateLabel->setObjectName("newsPageDate");
            dateLabel->setAlignment(Qt::AlignRight);
            layout->addWidget(dateLabel);
        } else {
            dateLabel = nullptr;
        }

        setLayout(layout);
    }

    void NewsPageItem::setupTheme(const Theme &theme) {
        setStyleSheet(
            QString(
                "NewsPageItem, #newsPageItem {"
                "   background-color: %1;"
                "   border: 1px solid %2;"
                "   border-radius: 12px;"
                "}"
                "NewsPageItem:hover, #newsPageItem:hover {"
                "   background-color: %3;"
                "   border-color: %8;"
                "}"
                "#newsPageTitle {"
                "   color: %4;"
                "   font-weight: bold;"
                "   font-size: 16px;"
                "}"
                "#newsPageSummary {"
                "   color: %5;"
                "   font-size: 14px;"
                "}"
                "#newsPageContent {"
                "   color: %5;"
                "   font-size: 13px;"
                "}"
                "#newsPageDate {"
                "   color: %6;"
                "   font-size: 12px;"
                "}"
                "#newsPageCategory {"
                "   color: %7;"
                "   background-color: %8;"
                "   border-radius: 6px;"
                "   padding: 4px 10px;"
                "   font-size: 11px;"
                "   font-weight: bold;"
                "}")
                .arg(theme.colors.surface.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.hover.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.primary.data()));
    }

    void NewsPageItem::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            if (!newsItem.link.empty()) {
                emit linkClicked(QString::fromStdString(newsItem.link));
            }
            emit clicked(newsItem);
        }
        QFrame::mousePressEvent(event);
    }

    // NewsPage implementation
    NewsPage::NewsPage(QWidget *parent)
        : QWidget(parent) {
        this->setAttribute(Qt::WA_TranslucentBackground);
        setupUi();
    }

    void NewsPage::setupUi() {
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // Header section
        headerWidget = new QWidget(this);
        headerWidget->setObjectName("newsPageHeader");
        auto *headerLayout = new QVBoxLayout(headerWidget);
        headerLayout->setContentsMargins(0, 0, 0, 0);
        headerLayout->setSpacing(4);

        titleLabel = new QLabel(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::title, "News")), this);
        titleLabel->setObjectName("newsPageMainTitle");
        titleLabel->setAlignment(Qt::AlignCenter);
        headerLayout->addWidget(titleLabel);

        mainLayout->addWidget(headerWidget);

        // Scroll area for news content
        scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setObjectName("newsPageScrollArea");
        scrollArea->setFrameShape(QFrame::NoFrame);

        contentWidget = new QWidget();
        contentWidget->setObjectName("newsPageContent");
        contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(12);
        contentLayout->addStretch();

        scrollArea->setWidget(contentWidget);
        mainLayout->addWidget(scrollArea, 1);

        // Footer section with dismiss options and continue button
        footerWidget = new QWidget(this);
        footerWidget->setObjectName("newsPageFooter");
        auto *footerLayout = new QHBoxLayout(footerWidget);
        footerLayout->setContentsMargins(0, 0, 0, 0);
        footerLayout->setSpacing(16);

        // Dismiss option combo box
        dismissComboBox = new QComboBox(this);
        dismissComboBox->setObjectName("newsPageDismissCombo");
        dismissComboBox->setMinimumWidth(160);
        connect(dismissComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), 
                this, &NewsPage::onDismissOptionChanged);
        footerLayout->addWidget(dismissComboBox);

        footerLayout->addStretch();

        // Continue button
        continueButton = new QPushButton(QString::fromStdString(
            lang::tr(lang::keys::button::category, "continue", "Continue")), this);
        continueButton->setObjectName("newsPageContinueBtn");
        continueButton->setMinimumSize(100, 36);
        continueButton->setCursor(Qt::PointingHandCursor);
        connect(continueButton, &QPushButton::clicked, this, &NewsPage::onContinueClicked);
        footerLayout->addWidget(continueButton);

        mainLayout->addWidget(footerWidget);

        // Add shadow effect to footer
        auto *footerShadow = new QGraphicsDropShadowEffect(this);
        footerShadow->setBlurRadius(20);
        footerShadow->setOffset(0, -4);
        footerShadow->setColor(QColor(0, 0, 0, 30));
        footerWidget->setGraphicsEffect(footerShadow);
    }

    void NewsPage::setupText() {
        titleLabel->setText(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::title, "News")));
        
        continueButton->setText(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::continueBtn, "Continue")));

        // Update dismiss combo box items
        dismissComboBox->clear();
        dismissComboBox->addItem(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::dismissNone, "Show every time")));
        dismissComboBox->addItem(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::dismiss3Days, "Don't show for 3 days")));
        dismissComboBox->addItem(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::dismiss7Days, "Don't show for 7 days")));
        dismissComboBox->addItem(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::dismissUntilUpdate, "Don't show until update")));
    }

    void NewsPage::setNews(const std::vector<api::NewsItem> &items) {
        newsItems = items;
        updateNewsItems();
    }

    void NewsPage::clearNews() {
        newsItems.clear();
        for (auto *widget : newsWidgets) {
            contentLayout->removeWidget(widget);
            widget->deleteLater();
        }
        newsWidgets.clear();
    }

    void NewsPage::updateNewsItems() {
        // Clear existing widgets
        for (auto *widget : newsWidgets) {
            contentLayout->removeWidget(widget);
            widget->deleteLater();
        }
        newsWidgets.clear();

        // Add new items
        for (const auto &item : newsItems) {
            auto *widget = new NewsPageItem(item, contentWidget);
            widget->setupTheme(currentTheme);
            
            connect(widget, &NewsPageItem::clicked, this, &NewsPage::newsItemClicked);
            connect(widget, &NewsPageItem::linkClicked, [](const QString &link) {
                QDesktopServices::openUrl(QUrl(link));
            });

            // Insert before the stretch
            contentLayout->insertWidget(contentLayout->count() - 1, widget);
            newsWidgets.push_back(widget);
        }

        // Show "no news" message if empty
        if (newsItems.empty()) {
            auto *noNewsLabel = new QLabel(QString::fromStdString(
                lang::tr(lang::keys::news::category, lang::keys::news::noNews, "No news available")), contentWidget);
            noNewsLabel->setObjectName("newsPageNoNews");
            noNewsLabel->setAlignment(Qt::AlignCenter);
            contentLayout->insertWidget(0, noNewsLabel);
        }
    }

    void NewsPage::setupTheme(const Theme &theme) {
        currentTheme = theme;

        const auto headerBg = theme.colors.surface.empty() ? theme.colors.background : theme.colors.surface;
        const auto footerBg = theme.colors.panel.empty() ? theme.colors.surface : theme.colors.panel;

        setStyleSheet(
            QString(
                "#newsPageHeader {"
                "   background-color: transparent;"
                "}"
                "#newsPageMainTitle {"
                "   color: %1;"
                "   font-size: 28px;"
                "   font-weight: bold;"
                "}"
                "#newsPageScrollArea {"
                "   background-color: transparent;"
                "   border: none;"
                "}"
                "#newsPageContent {"
                "   background-color: transparent;"
                "}"
                "#newsPageFooter {"
                "   background-color: %2;"
                "   border-top: 1px solid %3;"
                "}"
                "#newsPageContinueBtn {"
                "   background-color: %4;"
                "   color: %5;"
                "   border: none;"
                "   border-radius: 8px;"
                "   font-size: 14px;"
                "   font-weight: bold;"
                "   padding: 10px 24px;"
                "}"
                "#newsPageContinueBtn:hover {"
                "   background-color: %6;"
                "}"
                "#newsPageContinueBtn:pressed {"
                "   background-color: %7;"
                "}"
                "#newsPageDismissCombo {"
                "   background-color: %8;"
                "   color: %1;"
                "   border: 1px solid %3;"
                "   border-radius: 6px;"
                "   padding: 8px 12px;"
                "   font-size: 13px;"
                "}"
                "#newsPageDismissCombo::drop-down {"
                "   border: none;"
                "   width: 24px;"
                "}"
                "#newsPageDismissCombo QAbstractItemView {"
                "   background-color: %8;"
                "   color: %1;"
                "   selection-background-color: %9;"
                "   border: 1px solid %3;"
                "   border-radius: 4px;"
                "}"
                "#newsPageNoNews {"
                "   color: %10;"
                "   font-size: 16px;"
                "   padding: 40px;"
                "}")
                .arg(theme.colors.text.data())      // %1
                .arg(footerBg.data())               // %2
                .arg(theme.colors.disabled.data())  // %3
                .arg(theme.colors.primary.data())   // %4
                .arg(theme.colors.text.data())      // %5
                .arg(theme.colors.hover.data())     // %6
                .arg(theme.colors.accent.data())    // %7
                .arg(theme.colors.surface.data())   // %8
                .arg(theme.colors.hover.data())     // %9
                .arg(theme.colors.disabled.data())); // %10

        // Update existing news items
        for (auto *widget : newsWidgets) {
            widget->setupTheme(theme);
        }
    }

    void NewsPage::setupFont(const QFont &textFont, const QFont &h1Font, const QFont &h2Font) {
        titleLabel->setFont(h1Font);
        continueButton->setFont(textFont);
        dismissComboBox->setFont(textFont);
    }

    void NewsPage::resizeItems(int windowWidth, int windowHeight) {
        resize(windowWidth, windowHeight);

        // Calculate proportional margins based on window size
        int horizontalMargin = std::max(20, windowWidth / 10);
        int maxContentWidth = std::min(windowWidth - horizontalMargin * 2, 900);
        
        // Center the content
        int contentMargin = (windowWidth - maxContentWidth) / 2;
        
        // Vertical margins proportional to window height
        int headerTopMargin = std::max(10, windowHeight / 25);
        int headerBottomMargin = std::max(8, windowHeight / 40);
        int footerTopMargin = std::max(10, windowHeight / 40);
        int footerBottomMargin = std::max(12, windowHeight / 30);
        int contentVerticalMargin = std::max(8, windowHeight / 50);

        contentLayout->setContentsMargins(contentMargin, contentVerticalMargin, contentMargin, contentVerticalMargin);

        // Adjust footer margins - proportional to window height
        auto *footerLayout = qobject_cast<QHBoxLayout*>(footerWidget->layout());
        if (footerLayout) {
            footerLayout->setContentsMargins(contentMargin, footerTopMargin, contentMargin, footerBottomMargin);
        }

        // Adjust header margins - proportional to window height
        auto *headerLayout = qobject_cast<QVBoxLayout*>(headerWidget->layout());
        if (headerLayout) {
            headerLayout->setContentsMargins(contentMargin, headerTopMargin, contentMargin, headerBottomMargin);
        }
    }

    NewsDismissOption NewsPage::getDismissOption() const {
        return currentDismissOption;
    }

    void NewsPage::onContinueClicked() {
        emit continueClicked();
    }

    void NewsPage::onDismissOptionChanged(int index) {
        switch (index) {
            case 0:
                currentDismissOption = NewsDismissOption::None;
                break;
            case 1:
                currentDismissOption = NewsDismissOption::Days3;
                break;
            case 2:
                currentDismissOption = NewsDismissOption::Days7;
                break;
            case 3:
                currentDismissOption = NewsDismissOption::UntilUpdate;
                break;
            default:
                currentDismissOption = NewsDismissOption::None;
                break;
        }
        emit dismissOptionChanged(currentDismissOption);
    }

} // namespace neko::ui::page
