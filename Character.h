#ifndef CHARACTER_H
#define CHARACTER_H

#include <QGraphicsItem>
#include <QKeyEvent>
#include "GameConfig.h"
#include <QGraphicsObject>

class GameScene;

class Character : public QGraphicsObject   // 改为 QGraphicsObject
{
    Q_OBJECT
public:
    explicit Character(QGraphicsItem *parent = nullptr);
    virtual ~Character() = default;

    // 每帧更新（由 GameScene::advance 或手动调用）
    virtual void updateCharacter();

    // 获取基础速度（像素/帧），子类必须实现
    virtual qreal baseSpeed() const = 0;

    // 设置移动方向（由玩家输入或 AI 设置）
    void setMoveDirection(Direction dir);
    Direction moveDirection() const { return m_direction; }

    // 设置/查询是否允许移动
    void setMovementEnabled(bool enabled) { m_canMove = enabled; }
    bool isMovementEnabled() const { return m_canMove; }

    // 设置/查询全局启用（准备阶段禁用）
    void setEnabled(bool enabled) { m_enabled = enabled; }
    bool isEnabled() const { return m_enabled; }

    // 设置速度倍率（受伤、逃脱阶段加速等）
    void setSpeedMultiplier(qreal multiplier) { m_speedMultiplier = multiplier; }
    qreal speedMultiplier() const { return m_speedMultiplier; }

    // 当前实际速度
    qreal currentSpeed() const;

    // 设置面向方向（用于攻击判定等）
    void setFacingDirection(Direction dir) { m_facing = dir; }
    Direction facingDirection() const { return m_facing; }

    // 场景引用（用于碰撞检测）
    void setGameScene(GameScene *scene) { m_scene = scene; }
    GameScene* gameScene() const { return m_scene; }

    // 键盘事件处理（由 GameScene 转发，子类可重写）
    virtual void handleKeyPress(QKeyEvent *event) { Q_UNUSED(event); }
    virtual void handleKeyRelease(QKeyEvent *event) { Q_UNUSED(event); }

    // QGraphicsItem 必需接口
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

protected:
    // 移动方向（由输入/AI 设置）
    Direction m_direction;
    // 当前面向方向（根据移动方向自动更新）
    Direction m_facing;
    // 是否允许移动（眩晕、破译等状态下为 false）
    bool m_canMove;
    // 全局启用标志
    bool m_enabled;
    // 速度倍率
    qreal m_speedMultiplier;
    // 游戏场景（用于碰撞查询）
    GameScene *m_scene;

    // 默认角色尺寸（子类可覆盖）
    static constexpr int DEFAULT_WIDTH = 40;
    static constexpr int DEFAULT_HEIGHT = 40;

    // 移动与碰撞
    void moveWithCollision(const QPointF &delta);
    bool wouldCollideWithObstacle(const QPointF &newPos) const;
};

#endif // CHARACTER_H