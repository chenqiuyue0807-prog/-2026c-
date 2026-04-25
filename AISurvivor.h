#ifndef AISURVIVOR_H
#define AISURVIVOR_H

#include "Survivor.h"
#include "GameConfig.h"
#include <QPointF>

class CipherMachine;
class Gate;
class SurvivorBehavior;
class Hunter;

class AISurvivor : public Survivor
{
    Q_OBJECT

public:
    explicit AISurvivor(SurvivorType type, QGraphicsItem *parent = nullptr);

    // 每帧更新（移动、维持交互、冷却等）
    void updateCharacter() override;

    // 移动目标设置（由 SurvivorBehavior 调用）
    void setTargetPosition(const QPointF &pos);
    void clearTarget();
    bool hasTarget() const { return m_hasTarget; }
    QPointF targetPosition() const { return m_targetPos; }

    // 强制指令（优先级高于常规决策）
    void forceDecode(CipherMachine *cipher);
    void forceRescue(Survivor *target);
    void forceEscape(Gate *gate);
    void forceHide();

    // 被追击状态
    void setBeingChased(bool chased) { m_beingChased = chased; }
    bool isBeingChased() const { return m_beingChased; }

    // 技能相关
    bool isSkillReady() const { return m_skillCooldownTimer <= 0; }
    void useSkill();                            // 释放技能（F键）

    // 重写受伤，通知行为模块（如有）
    void takeDamage() override;

    // 设置行为模块（可选）
    void setBehavior(SurvivorBehavior *behavior) { m_behavior = behavior; }

private:
    // 移动辅助
    void moveTowardsTarget();
    void onInteractComplete();      // 交互完成清除强制目标

    SurvivorBehavior *m_behavior;   // 行为决策模块（可空）

    // 移动目标
    bool m_hasTarget;
    QPointF m_targetPos;

    // 强制交互目标
    CipherMachine *m_forcedCipher;
    Survivor *m_forcedRescueTarget;
    Gate *m_forcedGate;

    bool m_beingChased;             // 是否正被监管者追击

    // 技能冷却（帧计数）
    int m_skillCooldownTimer;
    void setSkillCooldown();
};

#endif // AISURVIVOR_H