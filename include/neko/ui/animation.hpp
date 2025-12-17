// SPDX-License-Identifier: AGPL-3.0-only
// NekoLauncher Animation Utilities
// iOS/Apple-style Transition Animations

#pragma once

#include <QtCore/QObject>
#include <QtCore/QEasingCurve>
#include <QtCore/QPointF>

class QWidget;

namespace neko::ui::anim {

// ============================================================================
// Animation Style Types
// ============================================================================
enum class AnimationStyle {
    None,       // No animations
    Minimal,    // Simple fade/slide
    Smooth,     // Standard smooth transitions
    iOS,        // iOS-style spring animations
    Bounce      // Bouncy playful animations
};

// ============================================================================
// Animation Duration Presets (milliseconds)
// ============================================================================
namespace Duration {
    constexpr int Fast = 180;
    constexpr int Normal = 280;
    constexpr int Slow = 350;
}

// ============================================================================
// Direction Enum for Slide Animations
// ============================================================================
enum class Direction {
    Left,
    Right,
    Up,
    Down
};

// ============================================================================
// iOS-style Easing Curves
// ============================================================================
namespace Easing {
    // Standard iOS-style easing curve (for enter animations)
    inline QEasingCurve iOS() {
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(QPointF(0.25, 0.1), QPointF(0.25, 1.0), QPointF(1.0, 1.0));
        return curve;
    }
    
    // iOS exit easing curve (for leave animations)
    inline QEasingCurve iOSExit() {
        QEasingCurve curve(QEasingCurve::BezierSpline);
        curve.addCubicBezierSegment(QPointF(0.4, 0.0), QPointF(1.0, 1.0), QPointF(1.0, 1.0));
        return curve;
    }
    
    // Spring effect (for popup animations)
    inline QEasingCurve Spring() {
        return QEasingCurve(QEasingCurve::OutBack);
    }
    
    // Bounce effect
    inline QEasingCurve Bounce() {
        return QEasingCurve(QEasingCurve::OutBounce);
    }
    
    // Simple linear
    inline QEasingCurve Linear() {
        return QEasingCurve(QEasingCurve::Linear);
    }
    
    // Simple ease out
    inline QEasingCurve EaseOut() {
        return QEasingCurve(QEasingCurve::OutQuad);
    }
}

// ============================================================================
// Animation Functions
// ============================================================================

/**
 * @brief Get the current animation style from config
 * @return Current AnimationStyle setting
 */
AnimationStyle getCurrentStyle();

/**
 * @brief Check if animations are enabled
 * @return true if animations are enabled
 */
bool isEnabled();

/**
 * @brief Page transition animation
 * 
 * New page slides in from specified direction, old page slides out
 * 
 * @param oldPage Current page (can be nullptr)
 * @param newPage Target page
 * @param direction Direction for new page to enter from
 */
void pageTransition(QWidget *oldPage, QWidget *newPage, Direction direction = Direction::Right);

/**
 * @brief Slide in animation
 * 
 * @param widget Target widget
 * @param direction Enter direction
 * @param duration Animation duration (defaults to Duration::Normal)
 */
void slideIn(QWidget *widget, Direction direction = Direction::Right, int duration = 0);

/**
 * @brief Slide out animation
 * 
 * @param widget Target widget
 * @param direction Exit direction
 * @param duration Animation duration (defaults to Duration::Normal)
 */
void slideOut(QWidget *widget, Direction direction = Direction::Left, int duration = 0);

/**
 * @brief Pop in animation (for dialogs)
 * 
 * Scales up from center with spring effect
 * 
 * @param widget Target widget
 * @param duration Animation duration (defaults to Duration::Normal)
 */
void popIn(QWidget *widget, int duration = 0);

/**
 * @brief Pop out animation (for dialogs)
 * 
 * @param widget Target widget
 * @param duration Animation duration (defaults to Duration::Fast)
 */
void popOut(QWidget *widget, int duration = 0);

/**
 * @brief Tab switch fade in effect
 * 
 * Slides up slightly from below
 * 
 * @param widget Target widget
 * @param duration Animation duration (defaults to Duration::Fast)
 */
void tabFadeIn(QWidget *widget, int duration = 0);

class TransitionManager : public QObject {
    Q_OBJECT
    
public:
    explicit TransitionManager(QObject *parent = nullptr);
    
    /**
     * @brief Switch to a new page
     * 
     * @param newPage Target page
     * @param direction Transition direction
     */
    void switchTo(QWidget *newPage, Direction direction = Direction::Right);
    
    /**
     * @brief Set current page (no animation)
     */
    void setCurrentPage(QWidget *page);
    
private:
    QWidget *m_currentPage;
};

} // namespace neko::ui::anim
