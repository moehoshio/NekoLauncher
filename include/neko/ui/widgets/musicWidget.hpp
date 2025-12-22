#pragma once

#include "neko/ui/theme.hpp"

#include <QtWidgets/QWidget>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtCore/QTimer>

#include <string>

namespace neko::ui::widget {

    /**
     * @brief A compact music control widget that displays current BGM status and provides playback controls.
     * 
     * Features:
     * - Shows current track name (scrolling if too long)
     * - Play/Pause button
     * - Stop button
     * - Volume slider
     * - Collapsible design (can be minimized to just an icon)
     */
    class MusicWidget : public QFrame {
        Q_OBJECT

    public:
        explicit MusicWidget(QWidget *parent = nullptr);
        ~MusicWidget() override;

        /**
         * @brief Apply theme styling to the widget.
         * @param theme The theme to apply
         */
        void setupTheme(const Theme &theme);

        /**
         * @brief Set the current track name to display.
         * @param trackName Name or path of the current track
         */
        void setTrackName(const QString &trackName);

        /**
         * @brief Update the playback state display.
         * @param state The current state (0=Stopped, 1=Playing, 2=Paused, 3=Loading, 4=Error)
         */
        void setState(int state);

        /**
         * @brief Update the volume slider position.
         * @param volume Volume level (0.0 to 1.0)
         */
        void setVolume(float volume);

        /**
         * @brief Set whether the widget is expanded or collapsed.
         * @param expanded True for expanded view, false for collapsed
         */
        void setExpanded(bool expanded);

        /**
         * @brief Check if the widget is currently expanded.
         * @return True if expanded
         */
        bool isExpanded() const;

        /**
         * @brief Set visibility based on config option.
         * @param visible Whether the widget should be visible
         */
        void setMusicControlVisible(bool visible);

    signals:
        /**
         * @brief Emitted when the play/pause button is clicked.
         */
        void playPauseClicked();

        /**
         * @brief Emitted when the stop button is clicked.
         */
        void stopClicked();

        /**
         * @brief Emitted when the volume slider value changes.
         * @param volume New volume level (0.0 to 1.0)
         */
        void volumeChanged(float volume);

        /**
         * @brief Emitted when the expand/collapse button is clicked.
         */
        void toggleExpanded();

    private slots:
        void onPlayPauseClicked();
        void onStopClicked();
        void onVolumeSliderChanged(int value);
        void onToggleExpand();
        void updateTrackScroll();

    private:
        void setupUI();
        void updateStateIcon();

        // UI Elements
        QHBoxLayout *mainLayout;
        QWidget *expandedContent;
        QVBoxLayout *expandedLayout;
        
        QPushButton *expandButton;      // Toggle expand/collapse
        QPushButton *playPauseButton;
        QPushButton *stopButton;
        QSlider *volumeSlider;
        QLabel *trackLabel;
        QLabel *stateIcon;
        
        // State
        bool m_expanded = false;
        bool m_visible = false;
        int m_currentState = 0;  // BgmState enum value
        QString m_trackName;
        
        // Scrolling text animation
        QTimer *scrollTimer;
        int scrollPosition = 0;
        int scrollDirection = 1;
    };

} // namespace neko::ui::widget
