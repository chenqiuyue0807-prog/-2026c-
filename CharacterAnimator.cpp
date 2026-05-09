#include "CharacterAnimator.h"

CharacterAnimator::CharacterAnimator(QObject *parent)
    : QObject(parent), m_timer(new QTimer(this))
{
    m_timer->setInterval(80);   // 每150ms切换一次帧
    connect(m_timer, &QTimer::timeout, this, &CharacterAnimator::advanceFrame);
}

void CharacterAnimator::setUpPixmap(const QPixmap &pm)    { m_up = pm; }
void CharacterAnimator::setDownPixmap(const QPixmap &pm)  { m_down = pm; }
void CharacterAnimator::setLeftPixmap(const QPixmap &pm)  { m_left = pm; }
void CharacterAnimator::setRightPixmap(const QPixmap &pm) { m_right = pm; }

void CharacterAnimator::setFrameInterval(int ms)
{
    m_timer->setInterval(ms);
}

void CharacterAnimator::startAnimation()
{
    m_moving = true;
    if (!m_timer->isActive())
        m_timer->start();
}

void CharacterAnimator::stopAnimation()
{
    m_moving = false;
    m_timer->stop();
    m_currentFrame = 0;    // 回到第一帧（站立）
    emit pixmapChanged();
}

void CharacterAnimator::setDirection(Direction dir)
{
    if (m_direction != dir) {
        m_direction = dir;
        if (!m_moving)
            emit pixmapChanged();  // 即使不移动，方向改变也要刷新
    }
}

QPixmap CharacterAnimator::currentPixmap() const
{
    // 如果只有一张图，可以忽略 m_currentFrame
    switch (m_direction) {
    case Direction::Up:    return m_up;
    case Direction::Down:  return m_down;
    case Direction::Left:  return m_left;
    case Direction::Right: return m_right;
    default:               return m_down;
    }
}

void CharacterAnimator::advanceFrame()
{
    if (!m_moving) return;
    m_currentFrame = (m_currentFrame + 1) % 2;   // 0→1→0…
    emit pixmapChanged();
}

void CharacterAnimator::reset()
{
    stopAnimation();
    m_direction = Direction::Down;
}