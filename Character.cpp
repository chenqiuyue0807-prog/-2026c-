#include "GameConfig.h"
#include "Character.h"
#include "GameScene.h"
#include <QPainter>
#include <QtMath>

Character::Character(QGraphicsItem *parent)
    : QGraphicsObject(parent)
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
    if (!m_enabled || !m_canMove || m_direction == Direction::None)
        return;

    qreal speed = currentSpeed();
    QPointF delta;

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

// 简单沿墙滑动，不强制推出
void Character::moveWithCollision(const QPointF &delta)
{
    if (delta.isNull()) return;

    QPointF newPos = pos() + delta;
    if (!wouldCollideWithObstacle(newPos)) {
        setPos(newPos);
        return;
    }

    // 尝试仅水平移动
    QPointF horizPos = pos() + QPointF(delta.x(), 0);
    if (!wouldCollideWithObstacle(horizPos)) {
        setPos(horizPos);
        return;
    }

    // 尝试仅垂直移动
    QPointF vertPos = pos() + QPointF(0, delta.y());
    if (!wouldCollideWithObstacle(vertPos)) {
        setPos(vertPos);
        return;
    }

    // 如果都失败，就不移动（避免滑动和卡墙）
}

// 寻找最近的安全位置（简单环绕搜索）
void Character::unstuck()
{
    const int searchRadius = 80;
    QPointF cur = pos();
    for (int r = 5; r < searchRadius; r += 5) {
        for (int angle = 0; angle < 360; angle += 30) {
            QPointF test = cur + QPointF(r * cos(angle * M_PI / 180.0), r * sin(angle * M_PI / 180.0));
            if (!wouldCollideWithObstacle(test)) {
                setPos(test);
                return;
            }
        }
    }
    setPos(GameConfig::getSurvivorSpawnPoint(0));
}

bool Character::wouldCollideWithObstacle(const QPointF &newPos) const
{
    if (!m_scene) return false;

    // 使用稍小的矩形进行检测（内缩2像素），防止像素级嵌入
    const qreal margin = 2.0;
    QRectF characterRect(newPos.x() - DEFAULT_WIDTH / 2 + margin,
                         newPos.y() - DEFAULT_HEIGHT / 2 + margin,
                         DEFAULT_WIDTH - margin * 2,
                         DEFAULT_HEIGHT - margin * 2);

    const QList<QGraphicsItem*>& obstacles = m_scene->getObstacles();
    for (QGraphicsItem *obs : obstacles) {
        if (obs->sceneBoundingRect().intersects(characterRect))
            return true;
    }

    // 地图边界（内缩角色半径）
    qreal halfW = DEFAULT_WIDTH / 2.0;
    qreal halfH = DEFAULT_HEIGHT / 2.0;
    if (newPos.x() < halfW || newPos.x() > GameConfig::MAP_WIDTH - halfW ||
        newPos.y() < halfH || newPos.y() > GameConfig::MAP_HEIGHT - halfH)
        return true;

    return false;
}

QRectF Character::boundingRect() const
{
    return QRectF(-DEFAULT_WIDTH / 2, -DEFAULT_HEIGHT / 2,
                  DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

void Character::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setBrush(Qt::gray);
    painter->setPen(Qt::black);
    painter->drawRect(boundingRect());

    painter->setPen(Qt::red);
    QPointF center(0, 0);
    QPointF arrow;
    switch (m_facing) {
    // 将箭头数值从15改为20左右
    case Direction::Up:    arrow = QPointF(0, -20); break;
    case Direction::Down:  arrow = QPointF(0, 20); break;
    case Direction::Left:  arrow = QPointF(-20, 0); break;
    case Direction::Right: arrow = QPointF(20, 0); break;
    default: return;
    }
    painter->drawLine(center, arrow);
}