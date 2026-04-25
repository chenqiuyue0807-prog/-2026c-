#include "RoleItem.h"
#include <QtMath>

RoleItem::RoleItem(RoleType type, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_state(RoleState::Idle)
    , m_health(2)
    , m_speed(3.0)
    , m_baseSpeed(3.0)
    , m_isHurt(false)
    , m_moveDx(0)
    , m_moveDy(0)
{
    setupByType();
}

void RoleItem::setupByType()
{
    // 临时用纯色方块代替图片（后续可替换为真实图片）
    QPixmap pixmap(30, 30);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    switch (m_type) {
    case RoleType::Hunter:
        painter.setBrush(Qt::darkRed);
        m_speed = 3.5;      // 监管者稍快
        m_baseSpeed = 3.5;
        m_health = 999;     // 监管者无生命值概念
        break;
    case RoleType::Doctor:
        painter.setBrush(Qt::green);
        m_speed = 3.0;
        m_baseSpeed = 3.0;
        break;
    case RoleType::Mechanic:
        painter.setBrush(Qt::yellow);
        m_speed = 3.0;
        m_baseSpeed = 3.0;
        break;
    case RoleType::AirForce:
        painter.setBrush(Qt::blue);
        m_speed = 3.0;
        m_baseSpeed = 3.0;
        break;
    }
    painter.drawRect(0, 0, 30, 30);
    setPixmap(pixmap);

    // 禁止图形项自动移动（我们自己控制）
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

void RoleItem::setHealth(int health)
{
    if (m_health != health) {
        m_health = health;
        emit healthChanged(m_health);

        // 如果生命值归零，进入死亡状态（求生者）
        if (m_type != RoleType::Hunter && m_health <= 0) {
            setState(RoleState::Dead);
        }
    }
}

void RoleItem::setState(RoleState state)
{
    if (m_state != state) {
        m_state = state;
        emit stateChanged(m_state);

        // 进入交互状态时停止移动
        if (state == RoleState::Interacting || state == RoleState::Stunned || state == RoleState::Dead) {
            stopMoving();
        }
    }
}

void RoleItem::setMoveDirection(qreal dx, qreal dy)
{
    // 归一化
   qreal length = qSqrt(dx*dx + dy*dy);
    if (length > 0.001) {
        m_moveDx = dx / length;
        m_moveDy = dy / length;
    } else {
        m_moveDx = 0;
        m_moveDy = 0;
    }

    // 更新状态
    if (m_state != RoleState::Interacting && m_state != RoleState::Stunned && m_state != RoleState::Dead) {
        if (length > 0.001) {
            m_state = RoleState::Moving;
        } else {
            m_state = RoleState::Idle;
        }
    }
}

void RoleItem::stopMoving()
{
    m_moveDx = 0;
    m_moveDy = 0;
    if (m_state == RoleState::Moving) {
        m_state = RoleState::Idle;
    }
}

void RoleItem::updatePosition(const QRectF &bounds)
{
    // 死亡或眩晕状态不能移动
    if (m_state == RoleState::Dead || m_state == RoleState::Stunned) {
        return;
    }

    // 交互状态不能移动（由玩家输入控制）
    if (m_state == RoleState::Interacting) {
        return;
    }

    if (m_moveDx != 0 || m_moveDy != 0) {
        QPointF newPos = pos() + QPointF(m_moveDx * m_speed, m_moveDy * m_speed);

        // 边界限制
        qreal w = boundingRect().width();
        qreal h = boundingRect().height();
        if (newPos.x() < bounds.left()) newPos.setX(bounds.left());
        if (newPos.y() < bounds.top()) newPos.setY(bounds.top());
        if (newPos.x() + w > bounds.right()) newPos.setX(bounds.right() - w);
        if (newPos.y() + h > bounds.bottom()) newPos.setY(bounds.bottom() - h);

        setPos(newPos);
    }
}

void RoleItem::applyHurtSpeedDebuff()
{
    if (!m_isHurt) {
        m_isHurt = true;
        m_speed = m_baseSpeed * 0.8;  // 受伤减速 20%
    }
}

void RoleItem::removeHurtSpeedDebuff()
{
    if (m_isHurt) {
        m_isHurt = false;
        m_speed = m_baseSpeed;
    }
}