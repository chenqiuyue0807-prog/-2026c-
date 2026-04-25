#ifndef HUNTER_H
#define HUNTER_H

#include "Character.h"
#include "GameConfig.h"

class Obstacle;
class Survivor;

class Hunter : public Character
{
    Q_OBJECT

public:
    explicit Hunter(QGraphicsItem *parent = nullptr);

    // 每帧更新
    void updateCharacter() override;

    // 攻击（由玩家鼠标左键或 AI 调用）
    void attack();

    // 破坏障碍物
    void startDestroying(Obstacle *obstacle);
    void stopDestroying();
    bool isDestroying() const { return m_destroying; }

    // 眩晕
    void stun(int durationFrames);
    bool isStunned() const { return m_stunned; }

    // 攻击冷却
    bool isAttackReady() const { return m_attackCooldownTimer <= 0; }

    // 速度倍率（逃脱阶段加速等）
    void setSpeedMultiplier(qreal multiplier) { m_speedMultiplier = multiplier; }

    // AI 目标移动
    void setTargetPosition(const QPointF &pos);
    void clearTarget();
    bool hasTarget() const { return m_hasTarget; }
    QPointF targetPosition() const { return m_targetPos; }

    // 基础速度
    qreal baseSpeed() const override { return GameConfig::HUNTER_BASE_SPEED; }

    // 绘制
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void attackCooldownChanged(qreal ratio);
    void obstacleDestroyed();

private:
    void performAttack();
    bool isSurvivorInAttackRange(Survivor *survivor, qreal &angleDiff) const;
    void updateDestroyProgress();
    void moveTowardsTarget();

    // 攻击状态
    bool m_attacking;
    int m_attackCooldownTimer;      // 攻击冷却帧数
    int m_attackStunTimer;          // 攻击命中后自身停顿帧数

    // 破坏障碍物
    bool m_destroying;
    Obstacle *m_currentObstacle;
    int m_destroyTimer;             // 破坏进度帧数

    // 眩晕
    bool m_stunned;
    int m_stunTimer;

    // AI 移动
    bool m_hasTarget;
    QPointF m_targetPos;
};

#endif // HUNTER_H