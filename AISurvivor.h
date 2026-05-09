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

    void updateCharacter() override;

    void setTargetPosition(const QPointF &pos);
    void clearTarget();
    bool hasTarget() const { return m_hasTarget; }
    QPointF targetPosition() const { return m_targetPos; }

    void forceDecode(CipherMachine *cipher);
    void forceRescue(Survivor *target);
    void forceEscape(Gate *gate);
    void forceHide();

    void setBeingChased(bool chased) { m_beingChased = chased; }
    bool isBeingChased() const { return m_beingChased; }

    bool isSkillReady() const { return m_skillCooldownTimer <= 0; }
    void useSkill();

    void takeDamage() override;

    void setBehavior(SurvivorBehavior *behavior) { m_behavior = behavior; }

    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setCiphers(const QList<CipherMachine*> &ciphers) { m_ciphers = ciphers; }

private:
    SurvivorBehavior *m_behavior;
    bool m_hasTarget;
    QPointF m_targetPos;
    CipherMachine *m_forcedCipher = nullptr;
    Survivor *m_forcedRescueTarget = nullptr;
    Gate *m_forcedGate = nullptr;
    bool m_beingChased;
    int m_skillCooldownTimer;
    int m_chaseTimer = 0;               // 强制逃跑持续时间
    Hunter *m_hunter = nullptr;
    QList<CipherMachine*> m_ciphers;

    void setSkillCooldown();
    void moveTowardsTarget();
    void onInteractComplete();
};

#endif