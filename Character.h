#ifndef CHARACTER_H
#define CHARACTER_H

#include <QGraphicsItem>
#include <QKeyEvent>
#include "GameConfig.h"
#include <QGraphicsObject>

class GameScene;
class Character : public QGraphicsObject
{
    Q_OBJECT
public:
    explicit Character(QGraphicsItem *parent = nullptr);
    virtual ~Character() = default;

    virtual void updateCharacter();
    virtual qreal baseSpeed() const = 0;

    void setMoveDirection(Direction dir);
    Direction moveDirection() const { return m_direction; }
    void setMovementEnabled(bool enabled) { m_canMove = enabled; }
    bool isMovementEnabled() const { return m_canMove; }
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }
    void setSpeedMultiplier(qreal multiplier) { m_speedMultiplier = multiplier; }
    qreal speedMultiplier() const { return m_speedMultiplier; }
    qreal currentSpeed() const;

    void setFacingDirection(Direction dir) { m_facing = dir; }
    Direction facingDirection() const { return m_facing; }
    void setGameScene(GameScene *scene) { m_scene = scene; }
    GameScene* gameScene() const { return m_scene; }

    virtual void handleKeyPress(QKeyEvent *event) { Q_UNUSED(event); }
    virtual void handleKeyRelease(QKeyEvent *event) { Q_UNUSED(event); }

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    Direction m_direction = Direction::None;
    Direction m_facing = Direction::Down;
    bool m_canMove = true;
    bool m_enabled = true;
    qreal m_speedMultiplier = 1.0;
    GameScene *m_scene = nullptr;

    static constexpr int DEFAULT_WIDTH = 50;   // 增大角色尺寸
    static constexpr int DEFAULT_HEIGHT = 50;

    void moveWithCollision(const QPointF &delta);
    bool wouldCollideWithObstacle(const QPointF &newPos) const;

private:
    void unstuck();
    int m_stuckCounter = 0;
    QPointF m_lastPos;
};

#endif