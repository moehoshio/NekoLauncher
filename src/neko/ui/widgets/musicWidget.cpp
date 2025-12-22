/**
 * @file musicWidget.cpp
 * @brief Music control widget implementation
 * @author moehoshio
 */

#include "neko/ui/widgets/musicWidget.hpp"
#include "neko/core/bgm.hpp"
#include "neko/bus/eventBus.hpp"
#include "neko/event/eventTypes.hpp"

#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtCore/QFileInfo>

#include <algorithm>

namespace neko::ui::widget {

    MusicWidget::MusicWidget(QWidget *parent) : QFrame(parent) {
        setupUI();

        // Subscribe to BGM state changes
        bus::event::subscribe<event::BgmStateChangedEvent>([this](const event::BgmStateChangedEvent &ev) {
            // Use Qt's thread-safe mechanism to update UI
            QMetaObject::invokeMethod(this, [this, state = ev.state, track = ev.track]() {
                setState(state);
                if (!track.empty()) {
                    setTrackName(QString::fromStdString(track));
                }
            }, Qt::QueuedConnection);
        });

        // Start hidden by default
        setVisible(false);
    }

    MusicWidget::~MusicWidget() {
        if (scrollTimer) {
            scrollTimer->stop();
        }
    }

    void MusicWidget::setupUI() {
        setObjectName("musicWidget");
        setFixedHeight(40);
        setMinimumWidth(60);
        setMaximumWidth(300);

        mainLayout = new QHBoxLayout(this);
        mainLayout->setContentsMargins(8, 4, 8, 4);
        mainLayout->setSpacing(6);

        // State icon (shows play/pause/stop status)
        stateIcon = new QLabel(this);
        stateIcon->setFixedSize(20, 20);
        stateIcon->setAlignment(Qt::AlignCenter);
        stateIcon->setText("♪");
        mainLayout->addWidget(stateIcon);

        // Track label (shows current track name)
        trackLabel = new QLabel(this);
        trackLabel->setObjectName("musicTrackLabel");
        trackLabel->setText(tr("No music"));
        trackLabel->setMinimumWidth(60);
        trackLabel->setMaximumWidth(120);
        trackLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        mainLayout->addWidget(trackLabel, 1);

        // Play/Pause button
        playPauseButton = new QPushButton(this);
        playPauseButton->setObjectName("musicPlayPauseButton");
        playPauseButton->setFixedSize(24, 24);
        playPauseButton->setText("▶");
        playPauseButton->setToolTip(tr("Play/Pause"));
        connect(playPauseButton, &QPushButton::clicked, this, &MusicWidget::onPlayPauseClicked);
        mainLayout->addWidget(playPauseButton);

        // Stop button
        stopButton = new QPushButton(this);
        stopButton->setObjectName("musicStopButton");
        stopButton->setFixedSize(24, 24);
        stopButton->setText("■");
        stopButton->setToolTip(tr("Stop"));
        connect(stopButton, &QPushButton::clicked, this, &MusicWidget::onStopClicked);
        mainLayout->addWidget(stopButton);

        // Volume slider
        volumeSlider = new QSlider(Qt::Horizontal, this);
        volumeSlider->setObjectName("musicVolumeSlider");
        volumeSlider->setRange(0, 100);
        volumeSlider->setValue(70);
        volumeSlider->setFixedWidth(60);
        volumeSlider->setToolTip(tr("Volume"));
        connect(volumeSlider, &QSlider::valueChanged, this, &MusicWidget::onVolumeSliderChanged);
        mainLayout->addWidget(volumeSlider);

        // Setup scroll timer for long track names
        scrollTimer = new QTimer(this);
        scrollTimer->setInterval(150);
        connect(scrollTimer, &QTimer::timeout, this, &MusicWidget::updateTrackScroll);

        // Add shadow effect
        auto *shadow = new QGraphicsDropShadowEffect(this);
        shadow->setBlurRadius(10);
        shadow->setColor(QColor(0, 0, 0, 80));
        shadow->setOffset(0, 2);
        setGraphicsEffect(shadow);

        updateStateIcon();
    }

    void MusicWidget::setupTheme(const Theme &theme) {
        // Apply theme colors
        QString frameStyle = QString(
            "QFrame#musicWidget {"
            "  background-color: %1;"
            "  border: 1px solid %2;"
            "  border-radius: 8px;"
            "}"
        ).arg(theme.colors.surface.data())
         .arg(theme.colors.disabled.data());

        QString labelStyle = QString(
            "QLabel#musicTrackLabel {"
            "  color: %1;"
            "  font-size: 11px;"
            "}"
        ).arg(theme.colors.text.data());

        QString buttonStyle = QString(
            "QPushButton#musicPlayPauseButton, QPushButton#musicStopButton {"
            "  background-color: %1;"
            "  color: %2;"
            "  border: none;"
            "  border-radius: 12px;"
            "  font-size: 12px;"
            "}"
            "QPushButton#musicPlayPauseButton:hover, QPushButton#musicStopButton:hover {"
            "  background-color: %3;"
            "}"
        ).arg(theme.colors.accent.data())
         .arg(theme.colors.text.data())
         .arg(theme.colors.hover.data());

        QString sliderStyle = QString(
            "QSlider#musicVolumeSlider::groove:horizontal {"
            "  background: %1;"
            "  height: 4px;"
            "  border-radius: 2px;"
            "}"
            "QSlider#musicVolumeSlider::handle:horizontal {"
            "  background: %2;"
            "  width: 12px;"
            "  height: 12px;"
            "  margin: -4px 0;"
            "  border-radius: 6px;"
            "}"
            "QSlider#musicVolumeSlider::sub-page:horizontal {"
            "  background: %2;"
            "  border-radius: 2px;"
            "}"
        ).arg(theme.colors.disabled.data())
         .arg(theme.colors.accent.data());

        setStyleSheet(frameStyle + labelStyle + buttonStyle + sliderStyle);

        stateIcon->setStyleSheet(QString("color: %1;").arg(theme.colors.accent.data()));
    }

    void MusicWidget::setTrackName(const QString &trackName) {
        m_trackName = trackName;
        
        // Extract just the filename
        QFileInfo fileInfo(trackName);
        QString displayName = fileInfo.baseName();
        if (displayName.isEmpty()) {
            displayName = trackName;
        }
        
        trackLabel->setText(displayName);
        
        // Start scrolling if text is too long
        QFontMetrics fm(trackLabel->font());
        if (fm.horizontalAdvance(displayName) > trackLabel->width()) {
            scrollPosition = 0;
            scrollTimer->start();
        } else {
            scrollTimer->stop();
        }
    }

    void MusicWidget::setState(int state) {
        m_currentState = state;
        updateStateIcon();
    }

    void MusicWidget::setVolume(float volume) {
        volumeSlider->blockSignals(true);
        volumeSlider->setValue(static_cast<int>(volume * 100));
        volumeSlider->blockSignals(false);
    }

    void MusicWidget::setExpanded(bool expanded) {
        m_expanded = expanded;
        // Future: toggle between compact and full view
    }

    bool MusicWidget::isExpanded() const {
        return m_expanded;
    }

    void MusicWidget::setMusicControlVisible(bool visible) {
        m_visible = visible;
        setVisible(visible);
    }

    void MusicWidget::onPlayPauseClicked() {
        auto &bgm = core::getBgmManager();
        if (m_currentState == 1) { // Playing
            bgm.pause();
        } else if (m_currentState == 2) { // Paused
            bgm.resume();
        }
        emit playPauseClicked();
    }

    void MusicWidget::onStopClicked() {
        core::getBgmManager().stop(500);
        emit stopClicked();
    }

    void MusicWidget::onVolumeSliderChanged(int value) {
        float volume = static_cast<float>(value) / 100.0f;
        core::getBgmManager().setVolume(volume);
        emit volumeChanged(volume);
    }

    void MusicWidget::onToggleExpand() {
        setExpanded(!m_expanded);
        emit toggleExpanded();
    }

    void MusicWidget::updateTrackScroll() {
        // Simple text scrolling animation
        if (m_trackName.isEmpty()) {
            return;
        }

        QFontMetrics fm(trackLabel->font());
        int textWidth = fm.horizontalAdvance(m_trackName);
        int labelWidth = trackLabel->width();

        if (textWidth <= labelWidth) {
            scrollTimer->stop();
            trackLabel->setText(m_trackName);
            return;
        }

        // Scroll the text
        scrollPosition += scrollDirection;
        int maxScroll = textWidth - labelWidth + 20;

        if (scrollPosition >= maxScroll) {
            scrollDirection = -1;
        } else if (scrollPosition <= 0) {
            scrollDirection = 1;
        }

        // Create scrolled substring
        QString displayText = m_trackName;
        QFileInfo fileInfo(m_trackName);
        displayText = fileInfo.baseName();
        
        int charOffset = (scrollPosition * static_cast<int>(displayText.length())) / maxScroll;
        charOffset = std::clamp(charOffset, 0, static_cast<int>(displayText.length()) - 1);
        
        trackLabel->setText(displayText.mid(charOffset));
    }

    void MusicWidget::updateStateIcon() {
        switch (m_currentState) {
            case 0: // Stopped
                stateIcon->setText("♪");
                playPauseButton->setText("▶");
                playPauseButton->setEnabled(false);
                break;
            case 1: // Playing
                stateIcon->setText("♫");
                playPauseButton->setText("⏸");
                playPauseButton->setEnabled(true);
                break;
            case 2: // Paused
                stateIcon->setText("♪");
                playPauseButton->setText("▶");
                playPauseButton->setEnabled(true);
                break;
            case 3: // Loading
                stateIcon->setText("⏳");
                playPauseButton->setText("▶");
                playPauseButton->setEnabled(false);
                break;
            case 4: // Error
                stateIcon->setText("⚠");
                playPauseButton->setText("▶");
                playPauseButton->setEnabled(false);
                break;
            default:
                stateIcon->setText("♪");
                break;
        }
    }

} // namespace neko::ui::widget
