#include "CharacterAnimator.h"

CharacterAnimator::CharacterAnimator(QObject *parent) : QObject(parent), m_timer(new QTimer(this)) {
    m_timer->setInterval(150);
    connect(m_timer, &QTimer::timeout, this, &CharacterAnimator::advanceFrame);
}
void CharacterAnimator::setUpPixmap(const QPixmap &pm) { m_up = pm; }
void CharacterAnimator::setDownPixmap(const QPixmap &pm) { m_down = pm; }
void CharacterAnimator::setLeftPixmap(const QPixmap &pm) { m_left = pm; }
void CharacterAnimator::setRightPixmap(const QPixmap &pm) { m_right = pm; }
void CharacterAnimator::setFrameInterval(int ms) { m_timer->setInterval(ms); }
void CharacterAnimator::startAnimation() { m_moving = true; if (!m_timer->isActive()) m_timer->start(); }
void CharacterAnimator::stopAnimation() { m_moving = false; m_timer->stop(); m_frame = 0; emit pixmapChanged(); }
void CharacterAnimator::setDirection(Direction dir) { if (m_dir != dir) { m_dir = dir; if (!m_moving) emit pixmapChanged(); } }
QPixmap CharacterAnimator::currentPixmap() const {
    switch (m_dir) {
    case Direction::Up: return m_up; case Direction::Down: return m_down;
    case Direction::Left: return m_left; case Direction::Right: return m_right;
    default: return m_down;
    }
}
void CharacterAnimator::advanceFrame() { if (m_moving) { m_frame = (m_frame + 1) % 2; emit pixmapChanged(); } }
void CharacterAnimator::reset() { stopAnimation(); m_dir = Direction::Down; }