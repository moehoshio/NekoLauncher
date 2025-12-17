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
                "   font-weight: bold;"
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
                .arg(theme.colors.surface.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.hover.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.primary.data()));
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
        : QWidget(parent), expanded(true) {
        setupUi();
    }

    void NewsPanel::setupUi() {
        auto *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(0, 0, 0, 0);
        mainLayout->setSpacing(0);

        // Header
        headerWidget = new QWidget(this);
        auto *headerLayout = new QHBoxLayout(headerWidget);
        headerLayout->setContentsMargins(12, 8, 8, 8);

        headerLabel = new QLabel(QString::fromStdString(
            lang::tr(lang::keys::news::category, lang::keys::news::title, "News")), this);
        headerLabel->setObjectName("newsPanelHeader");
        headerLayout->addWidget(headerLabel);

        headerLayout->addStretch();

        toggleButton = new QPushButton(this);
        toggleButton->setFixedSize(24, 24);
        toggleButton->setObjectName("newsToggleBtn");
        toggleButton->setFocusPolicy(Qt::NoFocus);
        connect(toggleButton, &QPushButton::clicked, this, &NewsPanel::onToggleClicked);
        headerLayout->addWidget(toggleButton);

        closeButton = new QPushButton("×", this);
        closeButton->setFixedSize(24, 24);
        closeButton->setObjectName("newsCloseBtn");
        closeButton->setFocusPolicy(Qt::NoFocus);
        connect(closeButton, &QPushButton::clicked, this, &NewsPanel::onCloseClicked);
        headerLayout->addWidget(closeButton);

        mainLayout->addWidget(headerWidget);

        // Scroll area for content
        scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(true);
        scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        scrollArea->setObjectName("newsScrollArea");

        contentWidget = new QWidget();
        contentLayout = new QVBoxLayout(contentWidget);
        contentLayout->setContentsMargins(8, 4, 8, 8);
        contentLayout->setSpacing(8);
        contentLayout->addStretch();

        scrollArea->setWidget(contentWidget);
        mainLayout->addWidget(scrollArea);

        updateToggleButton();
    }

    void NewsPanel::setNews(const std::vector<api::NewsItem> &items) {
        clearNews();

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

    void NewsPanel::clearNews() {
        for (auto *widget : newsWidgets) {
            contentLayout->removeWidget(widget);
            widget->deleteLater();
        }
        newsWidgets.clear();
    }

    void NewsPanel::setExpanded(bool expand) {
        if (expanded != expand) {
            expanded = expand;
            scrollArea->setVisible(expanded);
            updateToggleButton();
            emit toggleExpanded(expanded);
        }
    }

    bool NewsPanel::isExpanded() const {
        return expanded;
    }

    void NewsPanel::setupTheme(const Theme &theme) {
        currentTheme = theme;

        setStyleSheet(
            QString(
                "NewsPanel {"
                "   background-color: %1;"
                "   border: 1px solid %2;"
                "   border-radius: 10px;"
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
                "   border-radius: 4px;"
                "}"
                "#newsScrollArea {"
                "   background-color: transparent;"
                "   border: none;"
                "}")
                .arg(theme.colors.surface.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.text.data())
                .arg(theme.colors.disabled.data())
                .arg(theme.colors.hover.data()));

        // Update existing news items
        for (auto *widget : newsWidgets) {
            widget->setupTheme(theme);
        }
    }

    void NewsPanel::setupFont(const QFont &textFont, const QFont &titleFont) {
        headerLabel->setFont(titleFont);
    }

    void NewsPanel::onToggleClicked() {
        setExpanded(!expanded);
    }

    void NewsPanel::onCloseClicked() {
        emit closeRequested();
    }

    void NewsPanel::updateToggleButton() {
        toggleButton->setText(expanded ? "▲" : "▼");
    }

} // namespace neko::ui::widget
