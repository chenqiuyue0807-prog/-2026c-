#ifndef HUNTER_H
#define HUNTER_H

#include "Character.h"
#include "GameConfig.h"
#include "utils/CharacterAnimator.h"

class Obstacle;
class Survivor;

class Hunter : public Character
{
    Q_OBJECT

public:
    explicit Hunter(QGraphicsItem *parent = nullptr);
    void updateCharacter() override;
    void attack();
    void startDestroying(Obstacle *obstacle);
    void stopDestroying();
    bool isDestroying() const { return m_destroying; }
    void stun(int durationFrames);
    bool isStunned() const { return m_stunned; }
    bool isAttackReady() const { return m_attackCooldownTimer <= 0; }
    void setSpeedMultiplier(qreal multiplier) { m_speedMultiplier = multiplier; }
    void setTargetPosition(const QPointF &pos);
    void clearTarget();
    bool hasTarget() const { return m_hasTarget; }
    QPointF targetPosition() const { return m_targetPos; }
    qreal baseSpeed() const override { return GameConfig::HUNTER_BASE_SPEED; }
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void attackCooldownChanged(qreal ratio);
    void obstacleDestroyed();

private:
    QPixmap m_attackEffectPixmap;
    int m_attackEffectTimer = 0;
    QPointF m_attackEffectPos;

    void performAttack();
    bool isSurvivorInAttackRange(Survivor *survivor, qreal &angleDiff) const;
    void updateDestroyProgress();
    void moveTowardsTarget();

    bool m_attacking;
    int m_attackCooldownTimer;
    // 删除无用 m_attackStunTimer

    bool m_destroying;
    Obstacle *m_currentObstacle;
    int m_destroyTimer;

    bool m_stunned;
    int m_stunTimer;

    bool m_hasTarget;
    QPointF m_targetPos;

    CharacterAnimator* m_animator;
};

#endif