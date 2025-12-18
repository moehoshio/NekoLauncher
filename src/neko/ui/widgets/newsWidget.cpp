#include "neko/ui/widgets/newsWidget.hpp"
#include "neko/app/lang.hpp"

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtGui/QMouseEvent>
#include <QtGui/QDesktopServices>
#include <QtCore/QUrl>

namespace neko::ui::widget {

    // NewsItemWidget implementation
    NewsItemWidget::NewsItemWidget(const api::NewsItem &item, QWidget *parent)
        : QFrame(parent), newsItem(item) {
        
        setFrameShape(QFrame::StyledPanel);
        setFrameShadow(QFrame::Raised);
        setCursor(Qt::PointingHandCursor);

        auto *layout = new QVBoxLayout(this);
        layout->setContentsMargins(12, 10, 12, 10);
        layout->setSpacing(4);

        // Header row with title and category
        auto *headerLayout = new QHBoxLayout();
        headerLayout->setContentsMargins(0, 0, 0, 0);

        titleLabel = new QLabel(QString::fromStdString(item.title), this);
        titleLabel->setWordWrap(true);
        titleLabel->setObjectName("newsTitle");
        headerLayout->addWidget(titleLabel, 1);

        if (!item.category.empty()) {
            categoryLabel = new QLabel(QString::fromStdString(item.category), this);
            categoryLabel->setObjectName("newsCategory");
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
            summaryLabel->setObjectName("newsSummary");
            layout->addWidget(summaryLabel);
        } else {
            summaryLabel = nullptr;
        }

        // Date
        if (!item.publishTime.empty()) {
            dateLabel = new QLabel(QString::fromStdString(item.publishTime), this);
            dateLabel->setObjectName("newsDate");
            dateLabel->setAlignment(Qt::AlignRight);
            layout->addWidget(dateLabel);
        } else {
            dateLabel = nullptr;
        }

        setLayout(layout);
    }

    void NewsItemWidget::setupTheme(const Theme &theme) {
        const bool dark = theme.info.type == ThemeType::Dark;
        const QString cardBg = dark ? "rgba(255,255,255,0.06)" : "rgba(255,255,255,0.78)";
        const QString hoverBg = dark ? "rgba(255,255,255,0.12)" : "rgba(79,147,255,0.12)";
        const QString border = dark ? "rgba(255,255,255,0.12)" : "rgba(15,23,42,0.12)";
        setStyleSheet(
            QString(
                "NewsItemWidget {"
                "   background-color: %1;"
                "   border: 1px solid %2;"
                "   border-radius: 8px;"
                "}"
                "NewsItemWidget:hover {"
                "   background-color: %3;"
                "}"
                "#newsTitle {"
                "   color: %4;"
                "   font-weight: 600;"
                "}"
                "#newsSummary {"
                "   color: %5;"
                "}"
                "#newsDate {"
                "   color: %6;"
                "   font-size: 11px;"
                "}"
                "#newsCategory {"
                "   color: %7;"
                "   background-color: %8;"
                "   border-radius: 4px;"
                "   padding: 2px 6px;"
                "   font-size: 10px;"
                "}")
                .arg(cardBg)
                .arg(border)
                .arg(hoverBg)
                .arg(theme.colors.text.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.text.data())
                .arg(dark ? "rgba(255,255,255,0.08)" : "rgba(79,147,255,0.12)"));
    }

    void NewsItemWidget::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            if (!newsItem.link.empty()) {
                emit linkClicked(QString::fromStdString(newsItem.link));
            }
            emit clicked(newsItem);
        }
        QFrame::mousePressEvent(event);
    }

    // NewsPanel implementation
    NewsPanel::NewsPanel(QWidget *parent)
        : QWidget(parent), expanded(false) { // Start collapsed as banner
        setAttribute(Qt::WA_StyledBackground, true);
        setAutoFillBackground(false);
        setupUi();
    }

    void NewsPanel::setupUi() {
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // ===== Banner Widget (collapsed mode) =====
        bannerWidget = new QWidget(this);
        bannerWidget->setObjectName("newsBanner");
        bannerWidget->setCursor(Qt::PointingHandCursor);
        auto *bannerLayout = new QHBoxLayout(bannerWidget);
        bannerLayout->setContentsMargins(12, 8, 12, 8);
        bannerLayout->setSpacing(8);

        bannerIcon = new QLabel("📰", this);
        bannerIcon->setObjectName("newsBannerIcon");
        bannerLayout->addWidget(bannerIcon);

        bannerText = new QLabel(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::title, "News")), this);
        bannerText->setObjectName("newsBannerText");
        bannerText->setWordWrap(false);
        bannerLayout->addWidget(bannerText, 1);

        // Make banner clickable
        bannerWidget->installEventFilter(this);
        
        mainLayout->addWidget(bannerWidget);

        // ===== Expanded Widget =====
        expandedWidget = new QWidget(this);
        expandedWidget->setObjectName("newsExpandedWidget");
        auto *expandedLayout = new QVBoxLayout(expandedWidget);
        expandedLayout->setContentsMargins(0, 0, 0, 0);
        expandedLayout->setSpacing(0);

        // Header
        headerWidget = new QWidget(expandedWidget);
        auto *headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setContentsMargins(12, 8, 8, 8);

        headerLabel = new QLabel(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::title, "News")), this);
        headerLabel->setObjectName("newsPanelHeader");
        headerLayout->addWidget(headerLabel);

        headerLayout->addStretch();

        toggleButton = new QPushButton("▼", this);
        toggleButton->setFixedSize(24, 24);
        toggleButton->setObjectName("newsToggleBtn");
        toggleButton->setFocusPolicy(Qt::NoFocus);
        toggleButton->setToolTip("Collapse");
        connect(toggleButton, &QPushButton::clicked, this, &NewsPanel::onToggleClicked);
        headerLayout->addWidget(toggleButton);

        closeButton = new QPushButton("×", this);
        closeButton->setFixedSize(24, 24);
        closeButton->setObjectName("newsCloseBtn");
        closeButton->setFocusPolicy(Qt::NoFocus);
        closeButton->setToolTip("Close");
        connect(closeButton, &QPushButton::clicked, this, &NewsPanel::onCloseClicked);
        headerLayout->addWidget(closeButton);

        expandedLayout->addWidget(headerWidget);

        // Scroll area for content
        scrollArea = new QScrollArea(expandedWidget);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setObjectName("newsScrollArea");
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->viewport()->setAutoFillBackground(false);

        contentWidget = new QWidget();
        contentWidget->setObjectName("newsContent");
        contentWidget->setAttribute(Qt::WA_TranslucentBackground, true);
        contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(8, 4, 8, 8);
        contentLayout->setSpacing(8);
        contentLayout->addStretch();

        scrollArea->setWidget(contentWidget);
        expandedLayout->addWidget(scrollArea);

        mainLayout->addWidget(expandedWidget);

        // Initial state: show banner, hide expanded
        bannerWidget->setVisible(true);
        expandedWidget->setVisible(false);
    }

    bool NewsPanel::eventFilter(QObject *obj, QEvent *event) {
        if (obj == bannerWidget && event->type() == QEvent::MouseButtonRelease) {
            onBannerClicked();
            return true;
        }
        return QWidget::eventFilter(obj, event);
    }

    void NewsPanel::onBannerClicked() {
        setExpanded(true);
    }

    void NewsPanel::setNews(const std::vector<api::NewsItem> &items) {
        clearNews();
        newsItems = items; // Store for banner text
        updateBannerText();

        for (const auto &item : items) {
            auto *widget = new NewsItemWidget(item, contentWidget);
            widget->setupTheme(currentTheme);
            
            connect(widget, &NewsItemWidget::clicked, this, &NewsPanel::newsItemClicked);
            connect(widget, &NewsItemWidget::linkClicked, [](const QString &link) {
                QDesktopServices::openUrl(QUrl(link));
            });

            // Insert before the stretch
            contentLayout->insertWidget(contentLayout->count() - 1, widget);
            newsWidgets.push_back(widget);
        }
    }

    void NewsPanel::updateBannerText() {
        if (!newsItems.empty()) {
            // Show latest news title in banner
            QString latestTitle = QString::fromStdString(newsItems[0].title);
            // Truncate if too long
            if (latestTitle.length() > 40) {
                latestTitle = latestTitle.left(37) + "...";
            }
            bannerText->setText(latestTitle);
        } else {
            bannerText->setText(QString::fromStdString(
                lang::tr(lang::keys::news::category, lang::keys::news::title, "News")));
        }
    }

    void NewsPanel::clearNews() {
        for (auto *widget : newsWidgets) {
            contentLayout->removeWidget(widget);
            widget->deleteLater();
        }
        newsWidgets.clear();
        newsItems.clear();
    }

    void NewsPanel::setExpanded(bool expand) {
        if (expanded != expand) {
            expanded = expand;
            bannerWidget->setVisible(!expanded);
            expandedWidget->setVisible(expanded);
            updateToggleButton();
            emit toggleExpanded(expanded);
            emit expandedChanged(expanded);
        }
    }

    bool NewsPanel::isExpanded() const {
        return expanded;
    }
    
    QSize NewsPanel::preferredSize(int maxWidth, int maxHeight) const {
        if (expanded) {
            // Expanded: larger panel
            return QSize(std::min(maxWidth, 320), std::min(maxHeight, 400));
        } else {
            // Collapsed banner: single line height
            return QSize(std::min(maxWidth, 280), 40);
        }
    }

    void NewsPanel::setupTheme(const Theme &theme) {
        currentTheme = theme;

        const bool dark = theme.info.type == ThemeType::Dark;
        const QString panelBg = dark ? "rgba(15,23,42,0.85)" : "rgba(255,255,255,0.88)";
        const QString panelBorder = dark ? "rgba(255,255,255,0.12)" : "rgba(15,23,42,0.15)";
        const QString hoverBg = dark ? "rgba(255,255,255,0.10)" : "rgba(79,147,255,0.12)";
        const QString bannerBg = dark ? "rgba(15,23,42,0.80)" : "rgba(255,255,255,0.85)";

        setStyleSheet(
            QString(
                "NewsPanel {"
                "   background-color: %1;"
                "   border: 1px solid %2;"
                "   border-radius: 12px;"
                "}"
                // Banner styles
                "#newsBanner {"
                "   background-color: transparent;"
                "   border-radius: 12px;"
                "}"
                "#newsBanner:hover {"
                "   background-color: %5;"
                "}"
                "#newsBannerIcon {"
                "   font-size: 16px;"
                "}"
                "#newsBannerText {"
                "   color: %3;"
                "   font-weight: 500;"
                "}"
                // Expanded panel styles
                "#newsExpandedWidget {"
                "   background-color: transparent;"
                "}"
                "#newsPanelHeader {"
                "   color: %3;"
                "   font-weight: bold;"
                "}"
                "#newsToggleBtn, #newsCloseBtn {"
                "   background-color: transparent;"
                "   border: none;"
                "   color: %4;"
                "   font-size: 16px;"
                "}"
                "#newsToggleBtn:hover, #newsCloseBtn:hover {"
                "   background-color: %5;"
                "   border-radius: 8px;"
                "}"
                "#newsScrollArea {"
                "   background-color: transparent;"
                "   border: none;"
                "   border-radius: 12px;"
                "}"
                "#newsScrollArea QWidget#newsContent {"
                "   background-color: transparent;"
                "   border: none;"
                "}"
                "#newsScrollArea > QWidget {"
                "   background-color: transparent;"
                "}")
                .arg(panelBg)
                .arg(panelBorder)
                .arg(theme.colors.text.data())
                .arg(theme.colors.text.data())
                .arg(hoverBg));

        // Update existing news items
        for (auto *widget : newsWidgets) {
            widget->setupTheme(theme);
        }
    }

    void NewsPanel::setupFont(const QFont &textFont, const QFont &titleFont) {
        headerLabel->setFont(titleFont);
        bannerText->setFont(textFont);
    }

    void NewsPanel::onToggleClicked() {
        setExpanded(!expanded);
    }

    void NewsPanel::onCloseClicked() {
        emit closeRequested();
    }

    void NewsPanel::updateToggleButton() {
        toggleButton->setText(expanded ? "▼" : "▲");
        toggleButton->setToolTip(expanded ? "Collapse" : "Expand");
    }

} // namespace neko::ui::widget
