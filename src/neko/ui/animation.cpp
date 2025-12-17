// SPDX-License-Identifier: AGPL-3.0-only
// NekoLauncher Animation Utilities

#include "neko/ui/animation.hpp"
#include "neko/bus/configBus.hpp"

#include <QWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>

namespace neko::ui::anim {

// Get animation style from config string
AnimationStyle styleFromString(const std::string &str) {
    if (str == "none") return AnimationStyle::None;
    if (str == "minimal") return AnimationStyle::Minimal;
    if (str == "smooth") return AnimationStyle::Smooth;
    if (str == "bounce") return AnimationStyle::Bounce;
    return AnimationStyle::iOS; // default
}

AnimationStyle getCurrentStyle() {
    auto config = bus::config::getClientConfig();
    return styleFromString(config.style.animation);
}

bool isEnabled() {
    return getCurrentStyle() != AnimationStyle::None;
}

// Get easing curve based on current style
QEasingCurve getEnterCurve() {
    switch (getCurrentStyle()) {
        case AnimationStyle::None:
        case AnimationStyle::Minimal:
            return Easing::Linear();
        case AnimationStyle::Smooth:
            return Easing::EaseOut();
        case AnimationStyle::Bounce:
            return Easing::Bounce();
        case AnimationStyle::iOS:
        default:
            return Easing::iOS();
    }
}

QEasingCurve getExitCurve() {
    switch (getCurrentStyle()) {
        case AnimationStyle::None:
        case AnimationStyle::Minimal:
            return Easing::Linear();
        case AnimationStyle::Smooth:
            return Easing::EaseOut();
        case AnimationStyle::Bounce:
        case AnimationStyle::iOS:
        default:
            return Easing::iOSExit();
    }
}

QEasingCurve getPopCurve() {
    switch (getCurrentStyle()) {
        case AnimationStyle::None:
        case AnimationStyle::Minimal:
            return Easing::Linear();
        case AnimationStyle::Smooth:
            return Easing::EaseOut();
        case AnimationStyle::Bounce:
            return Easing::Bounce();
        case AnimationStyle::iOS:
        default:
            return Easing::Spring();
    }
}

int getAdjustedDuration(int baseDuration) {
    switch (getCurrentStyle()) {
        case AnimationStyle::None:
            return 0;
        case AnimationStyle::Minimal:
            return baseDuration / 2;
        case AnimationStyle::Bounce:
            return static_cast<int>(baseDuration * 1.2);
        default:
            return baseDuration;
    }
}

// Page transition animation
void pageTransition(QWidget *oldPage, QWidget *newPage, Direction direction) {
    if (!newPage) return;
    
    QWidget *parent = newPage->parentWidget();
    if (!parent) return;
    
    // If animations disabled, just show/hide
    if (!isEnabled()) {
        if (oldPage && oldPage != newPage) {
            oldPage->hide();
        }
        newPage->setGeometry(0, 0, parent->width(), parent->height());
        newPage->show();
        newPage->raise();
        return;
    }
    
    const int width = parent->width();
    const int height = parent->height();
    const int duration = getAdjustedDuration(Duration::Slow);
    
    newPage->show();
    newPage->raise();
    
    int startX = (direction == Direction::Right) ? width : -width;
    newPage->setGeometry(startX, 0, width, height);
    
    auto *newPageAnim = new QPropertyAnimation(newPage, "geometry");
    newPageAnim->setDuration(duration);
    newPageAnim->setStartValue(QRect(startX, 0, width, height));
    newPageAnim->setEndValue(QRect(0, 0, width, height));
    newPageAnim->setEasingCurve(getEnterCurve());
    
    if (oldPage && oldPage != newPage) {
        int endX = (direction == Direction::Right) ? -width / 3 : width / 3;
        
        auto *oldPageAnim = new QPropertyAnimation(oldPage, "geometry");
        oldPageAnim->setDuration(duration);
        oldPageAnim->setStartValue(QRect(0, 0, width, height));
        oldPageAnim->setEndValue(QRect(endX, 0, width, height));
        oldPageAnim->setEasingCurve(getEnterCurve());
        
        auto *group = new QParallelAnimationGroup(newPage);
        group->addAnimation(newPageAnim);
        group->addAnimation(oldPageAnim);
        
        QObject::connect(group, &QParallelAnimationGroup::finished, oldPage, [oldPage]() {
            oldPage->hide();
        });
        
        group->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        newPageAnim->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

// Slide in animation
void slideIn(QWidget *widget, Direction direction, int duration) {
    if (!widget) return;
    
    QWidget *parent = widget->parentWidget();
    if (!parent) return;
    
    const int width = parent->width();
    const int height = parent->height();
    
    // If animations disabled, just show
    if (!isEnabled()) {
        widget->setGeometry(0, 0, width, height);
        widget->show();
        widget->raise();
        return;
    }
    
    widget->show();
    widget->raise();
    
    int startX = 0, startY = 0;
    switch (direction) {
        case Direction::Left:
            startX = -width;
            break;
        case Direction::Right:
            startX = width;
            break;
        case Direction::Up:
            startY = -height;
            break;
        case Direction::Down:
            startY = height;
            break;
    }
    
    widget->setGeometry(startX, startY, width, height);
    
    int dur = duration > 0 ? duration : Duration::Normal;
    dur = getAdjustedDuration(dur);
    
    auto *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(dur);
    anim->setStartValue(QRect(startX, startY, width, height));
    anim->setEndValue(QRect(0, 0, width, height));
    anim->setEasingCurve(getEnterCurve());
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Slide out animation
void slideOut(QWidget *widget, Direction direction, int duration) {
    if (!widget) return;
    
    QWidget *parent = widget->parentWidget();
    if (!parent) return;
    
    const int width = parent->width();
    const int height = parent->height();
    
    // If animations disabled, just hide
    if (!isEnabled()) {
        widget->hide();
        return;
    }
    
    int endX = 0, endY = 0;
    switch (direction) {
        case Direction::Left:
            endX = -width;
            break;
        case Direction::Right:
            endX = width;
            break;
        case Direction::Up:
            endY = -height;
            break;
        case Direction::Down:
            endY = height;
            break;
    }
    
    int dur = duration > 0 ? duration : Duration::Normal;
    dur = getAdjustedDuration(dur);
    
    auto *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(dur);
    anim->setStartValue(widget->geometry());
    anim->setEndValue(QRect(endX, endY, width, height));
    anim->setEasingCurve(getExitCurve());
    
    QObject::connect(anim, &QPropertyAnimation::finished, widget, [widget]() {
        widget->hide();
    });
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Pop in animation (for dialogs)
void popIn(QWidget *widget, int duration) {
    if (!widget) return;
    
    QRect targetRect = widget->geometry();
    
    // If animations disabled, just show
    if (!isEnabled()) {
        widget->show();
        widget->raise();
        return;
    }
    
    QPoint center = targetRect.center();
    
    // Scale factor based on style
    int scaleFactor = (getCurrentStyle() == AnimationStyle::Bounce) ? 80 : 92;
    int smallWidth = targetRect.width() * scaleFactor / 100;
    int smallHeight = targetRect.height() * scaleFactor / 100;
    QRect startRect(
        center.x() - smallWidth / 2,
        center.y() - smallHeight / 2,
        smallWidth,
        smallHeight
    );
    
    widget->setGeometry(startRect);
    widget->show();
    widget->raise();
    
    int dur = duration > 0 ? duration : Duration::Normal;
    dur = getAdjustedDuration(dur);
    
    auto *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(dur);
    anim->setStartValue(startRect);
    anim->setEndValue(targetRect);
    anim->setEasingCurve(getPopCurve());
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Pop out animation (for dialogs)
void popOut(QWidget *widget, int duration) {
    if (!widget) return;
    
    QRect originalRect = widget->geometry();
    
    // If animations disabled, just hide
    if (!isEnabled()) {
        widget->hide();
        return;
    }
    
    QPoint center = originalRect.center();
    
    int smallWidth = originalRect.width() * 92 / 100;
    int smallHeight = originalRect.height() * 92 / 100;
    QRect endRect(
        center.x() - smallWidth / 2,
        center.y() - smallHeight / 2,
        smallWidth,
        smallHeight
    );
    
    int dur = duration > 0 ? duration : Duration::Fast;
    dur = getAdjustedDuration(dur);
    
    auto *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(dur);
    anim->setStartValue(originalRect);
    anim->setEndValue(endRect);
    anim->setEasingCurve(getExitCurve());
    
    // Restore original geometry after animation and hide
    QObject::connect(anim, &QPropertyAnimation::finished, widget, [widget, originalRect]() {
        widget->setGeometry(originalRect);
        widget->hide();
    });
    
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// Tab fade in effect
void tabFadeIn(QWidget *widget, int duration) {
    if (!widget) return;
    
    widget->show();
    
    // If animations disabled, nothing more to do
    if (!isEnabled()) {
        return;
    }
    
    QRect targetRect = widget->geometry();
    
    // Offset based on style
    int offset = (getCurrentStyle() == AnimationStyle::Bounce) ? 25 : 15;
    QRect startRect = targetRect;
    startRect.moveTop(targetRect.top() + offset);
    
    widget->setGeometry(startRect);
    
    int dur = duration > 0 ? duration : Duration::Fast;
    dur = getAdjustedDuration(dur);
    
    auto *anim = new QPropertyAnimation(widget, "geometry");
    anim->setDuration(dur);
    anim->setStartValue(startRect);
    anim->setEndValue(targetRect);
    anim->setEasingCurve(getEnterCurve());
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

// TransitionManager implementation
TransitionManager::TransitionManager(QObject *parent)
    : QObject(parent), m_currentPage(nullptr) {}

void TransitionManager::switchTo(QWidget *newPage, Direction direction) {
    if (!newPage || newPage == m_currentPage) return;
    
    pageTransition(m_currentPage, newPage, direction);
    m_currentPage = newPage;
}

void TransitionManager::setCurrentPage(QWidget *page) {
    m_currentPage = page;
}

} // namespace neko::ui::anim
