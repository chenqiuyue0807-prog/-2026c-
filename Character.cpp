#include "Character.h"
#include "GameScene.h"
#include <QPainter>
#include <QtMath>

Character::Character(QGraphicsItem *parent)
    : QGraphicsObject(parent)   // 正确
    , m_direction(Direction::None)
    , m_facing(Direction::Down)
    , m_canMove(true)
    , m_enabled(true)
    , m_speedMultiplier(1.0)
    , m_scene(nullptr)
{
    setFlag(QGraphicsItem::ItemIsFocusable);
}

void Character::updateCharacter()
{
    // 若全局禁用、禁止移动或无移动方向，则不移动
    if (!m_enabled || !m_canMove || m_direction == Direction::None)
        return;

    qreal speed = currentSpeed();
    QPointF delta;

    // 根据移动方向计算位移，并更新面向方向
    switch (m_direction) {
    case Direction::Up:
        delta = QPointF(0, -speed);
        m_facing = Direction::Up;
        break;
    case Direction::Down:
        delta = QPointF(0, speed);
        m_facing = Direction::Down;
        break;
    case Direction::Left:
        delta = QPointF(-speed, 0);
        m_facing = Direction::Left;
        break;
    case Direction::Right:
        delta = QPointF(speed, 0);
        m_facing = Direction::Right;
        break;
    default:
        return;
    }

    // 尝试移动（含碰撞回退）
    moveWithCollision(delta);
}

qreal Character::currentSpeed() const
{
    return baseSpeed() * m_speedMultiplier;
}

void Character::setMoveDirection(Direction dir)
{
    m_direction = dir;
}

void Character::moveWithCollision(const QPointF &delta)
{
    if (delta.isNull()) return;

    QPointF newPos = pos() + delta;

    // 完整位移未碰撞则直接移动
    if (!wouldCollideWithObstacle(newPos)) {
        setPos(newPos);
        return;
    }

    // 尝试单独沿 X 轴或 Y 轴移动（分离轴处理）
    QPointF horizPos = pos() + QPointF(delta.x(), 0);
    QPointF vertPos  = pos() + QPointF(0, delta.y());

    bool horizOk = !wouldCollideWithObstacle(horizPos);
    bool vertOk  = !wouldCollideWithObstacle(vertPos);

    if (horizOk && vertOk) {
        // 两个轴都可移动，选择位移较大的轴
        if (qAbs(delta.x()) > qAbs(delta.y()))
            setPos(horizPos);
        else
            setPos(vertPos);
    } else if (horizOk) {
        setPos(horizPos);
    } else if (vertOk) {
        setPos(vertPos);
    }
    // 否则完全卡住，位置不变
}

bool Character::wouldCollideWithObstacle(const QPointF &newPos) const
{
    if (!m_scene) return false;

    // 构建角色在新位置的矩形
    QRectF characterRect(newPos.x() - DEFAULT_WIDTH / 2,
                         newPos.y() - DEFAULT_HEIGHT / 2,
                         DEFAULT_WIDTH, DEFAULT_HEIGHT);

    // 检查与所有障碍物的矩形相交
    const QList<QGraphicsItem*>& obstacles = m_scene->getObstacles();
    for (QGraphicsItem *obs : obstacles) {
        if (obs->sceneBoundingRect().intersects(characterRect))
            return true;
    }

    // 地图边界限制
    if (newPos.x() < DEFAULT_WIDTH / 2 ||
        newPos.x() > GameConfig::MAP_WIDTH - DEFAULT_WIDTH / 2 ||
        newPos.y() < DEFAULT_HEIGHT / 2 ||
        newPos.y() > GameConfig::MAP_HEIGHT - DEFAULT_HEIGHT / 2) {
        return true;
    }

    return false;
}

QRectF Character::boundingRect() const
{
    return QRectF(-DEFAULT_WIDTH / 2, -DEFAULT_HEIGHT / 2,
                  DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void Character::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 默认绘制灰色矩形，子类可重写以显示具体图标
    painter->setBrush(Qt::gray);
    painter->setPen(Qt::black);
    painter->drawRect(boundingRect());

    // 绘制朝向指示（简单箭头）
    painter->setPen(Qt::red);
    QPointF center(0, 0);
    QPointF arrow;
    switch (m_facing) {
    case Direction::Up:    arrow = QPointF(0, -15); break;
    case Direction::Down:  arrow = QPointF(0, 15); break;
    case Direction::Left:  arrow = QPointF(-15, 0); break;
    case Direction::Right: arrow = QPointF(15, 0); break;
    default: return;
    }
    painter->drawLine(center, arrow);
}