#include "AISurvivor.h"
#include "ai/SurvivorBehavior.h"
#include "CipherMachine.h"
#include "Gate.h"
#include "Hunter.h"
//#include "MechanicalPuppet.h"   // 如果没有可注释掉相关部分
#include "GameScene.h"
#include "GameConfig.h"
#include <QLineF>
#include <QRandomGenerator>
#include <QDebug>
#include <QtMath>
#include <QTimer>
// 在顶端添加
#include "MechanicalPuppet.h"

AISurvivor::AISurvivor(SurvivorType type, QGraphicsItem *parent)
    : Survivor(type, parent)
    , m_behavior(nullptr)
    , m_hasTarget(false)
    , m_forcedCipher(nullptr)
    , m_forcedRescueTarget(nullptr)
    , m_forcedGate(nullptr)
    , m_beingChased(false)
    , m_skillCooldownTimer(0)
{
    setEnabled(true);
}

void AISurvivor::updateCharacter()
{
    // 先处理基类：燃烧、救助进度等
    Survivor::updateCharacter();

    if (!m_enabled || m_eliminated || m_burning) {
        return;
    }

    // 更新技能冷却
    if (m_skillCooldownTimer > 0) {
        m_skillCooldownTimer--;
    }

    // 如果正在交互（破译、开门、救助），维持状态，并检查距离/有效性
    if (m_decoding || m_openingGate || m_rescuing) {
        // 交互有效性检查
        if (m_decoding && m_currentCipher) {
            qreal dist = QLineF(pos(), m_currentCipher->pos()).length();
            if (dist > GameConfig::INTERACT_CIPHER_DIST || m_currentCipher->isCompleted()) {
                stopDecoding();
            }
        }
        if (m_openingGate && m_currentGate) {
            qreal dist = QLineF(pos(), m_currentGate->pos()).length();
            if (dist > GameConfig::INTERACT_GATE_DIST || !m_currentGate->isUnlocked()) {
                stopOpeningGate();
            }
        }
        if (m_rescuing && m_rescueTarget) {
            qreal dist = QLineF(pos(), m_rescueTarget->pos()).length();
            if (dist > GameConfig::INTERACT_RESCUE_DIST || !m_rescueTarget->isBurning()) {
                stopRescuing();
            }
        }
        return; // 交互中不移动
    }

    // 优先处理强制指令
    if (m_forcedCipher) {
        qreal dist = QLineF(pos(), m_forcedCipher->pos()).length();
        if (dist <= GameConfig::INTERACT_CIPHER_DIST && !m_forcedCipher->isCompleted() &&
            m_forcedCipher->currentDecoders() < GameConfig::MAX_SURVIVORS_PER_CIPHER) {
            startDecoding(m_forcedCipher);
            m_forcedCipher = nullptr;
            clearTarget();
        } else {
            setTargetPosition(m_forcedCipher->pos());
        }
    } else if (m_forcedRescueTarget) {
        qreal dist = QLineF(pos(), m_forcedRescueTarget->pos()).length();
        if (dist <= GameConfig::INTERACT_RESCUE_DIST && m_forcedRescueTarget->isBurning()) {
            startRescuing(m_forcedRescueTarget);
            m_forcedRescueTarget = nullptr;
            clearTarget();
        } else {
            setTargetPosition(m_forcedRescueTarget->pos());
        }
    } else if (m_forcedGate) {
        qreal dist = QLineF(pos(), m_forcedGate->pos()).length();
        if (dist <= GameConfig::INTERACT_GATE_DIST && m_forcedGate->isUnlocked()) {
            if (m_forcedGate->isFullyOpen()) {
                escape();
            } else {
                startOpeningGate(m_forcedGate);
            }
            m_forcedGate = nullptr;
            clearTarget();
        } else {
            setTargetPosition(m_forcedGate->pos());
        }
    }

    // 执行移动（向目标点）
    if (m_hasTarget && m_canMove) {
        moveTowardsTarget();
    } else {
        setMoveDirection(Direction::None);
    }
}

void AISurvivor::setTargetPosition(const QPointF &pos)
{
    m_targetPos = pos;
    m_hasTarget = true;
}

void AISurvivor::clearTarget()
{
    m_hasTarget = false;
    setMoveDirection(Direction::None);
}

void AISurvivor::forceDecode(CipherMachine *cipher)
{
    m_forcedCipher = cipher;
    m_forcedRescueTarget = nullptr;
    m_forcedGate = nullptr;
}

void AISurvivor::forceRescue(Survivor *target)
{
    m_forcedRescueTarget = target;
    m_forcedCipher = nullptr;
    m_forcedGate = nullptr;
}

void AISurvivor::forceEscape(Gate *gate)
{
    m_forcedGate = gate;
    m_forcedCipher = nullptr;
    m_forcedRescueTarget = nullptr;
}

void AISurvivor::forceHide()
{
    // 寻找最近的草丛
    if (!m_scene) return;
    QList<Bush*> bushes = m_scene->getBushes();
    if (!bushes.isEmpty()) {
        Bush *nearest = nullptr;
        qreal minDist = 1e9;
        for (Bush *b : bushes) {
            qreal d = QLineF(pos(), b->sceneBoundingRect().center()).length();
            if (d < minDist) {
                minDist = d;
                nearest = b;
            }
        }
        if (nearest) {
            setTargetPosition(nearest->sceneBoundingRect().center());
        }
    }
    // 否则由 SurvivorBehavior 继续计算
}

void AISurvivor::moveTowardsTarget()
{
    QPointF dir = m_targetPos - pos();
    qreal length = QLineF(pos(), m_targetPos).length();
    if (length < 5.0) {
        clearTarget();
        return;
    }
    if (qAbs(dir.x()) > qAbs(dir.y())) {
        setMoveDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
    } else {
        setMoveDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
    }
}

void AISurvivor::setSkillCooldown()
{
    switch (m_type) {
    case SurvivorType::Doctor:   m_skillCooldownTimer = GameConfig::FRAMES_DOCTOR_COOLDOWN; break;
    case SurvivorType::Mechanic: m_skillCooldownTimer = GameConfig::FRAMES_MECHANIC_COOLDOWN; break;
    case SurvivorType::AirForce: m_skillCooldownTimer = GameConfig::FRAMES_AIRFORCE_COOLDOWN; break;
    default: break;
    }
}

void AISurvivor::useSkill()
{
    if (!isSkillReady()) return;

    if (m_type == SurvivorType::Doctor) {
        // 治疗自身或周围受伤队友
        Survivor *target = nullptr;
        qreal minDist = 30.0;
        if (isHurt()) {
            target = this;
        } else {
            QList<QGraphicsItem*> items = m_scene->items();
            for (QGraphicsItem *item : items) {
                Survivor *s = dynamic_cast<Survivor*>(item);
                if (s && s != this && s->isHurt() && !s->isEliminated()) {
                    qreal d = QLineF(pos(), s->pos()).length();
                    if (d < minDist) {
                        minDist = d;
                        target = s;
                    }
                }
            }
        }
        if (target) {
            target->heal();
            revealPosition(GameConfig::FRAMES_DOCTOR_HEAL);
        }
    }
    // 在 useSkill() 的 Mechanic 分支中
    else if (m_type == SurvivorType::Mechanic) {
        if (m_scene) {
            MechanicalPuppet *puppet = new MechanicalPuppet(pos(), m_scene);
            // 寻找最近未完成的密码机
            CipherMachine *target = nullptr;
            qreal minDist = 1e9;
            QList<QGraphicsItem*> items = m_scene->items();
            for (QGraphicsItem *item : items) {
                CipherMachine *cipher = dynamic_cast<CipherMachine*>(item);
                if (cipher && !cipher->isCompleted()) {
                    qreal d = QLineF(pos(), cipher->pos()).length();
                    if (d < minDist) {
                        minDist = d;
                        target = cipher;
                    }
                }
            }
            if (target) {
                puppet->moveToTarget(target->pos());
            }
        }
    }
    else if (m_type == SurvivorType::AirForce) {
        if (!m_scene) return;
        Hunter *hunter = nullptr;
        QList<QGraphicsItem*> items = m_scene->items();
        for (auto *item : items) {
            Hunter *h = dynamic_cast<Hunter*>(item);
            if (h) { hunter = h; break; }
        }
        if (!hunter) return;
        qreal dist = QLineF(pos(), hunter->pos()).length();
        if (dist > 120) return;

        QPointF dir = hunter->pos() - pos();
        qreal angleToHunter = atan2(dir.y(), dir.x()) * 180.0 / M_PI;
        if (angleToHunter < 0) angleToHunter += 360.0;
        qreal facingAngle = 0;
        switch (m_facing) {
        case Direction::Right: facingAngle = 0; break;
        case Direction::Down:  facingAngle = 90; break;
        case Direction::Left:  facingAngle = 180; break;
        case Direction::Up:    facingAngle = 270; break;
        default: return;
        }
        qreal diff = fabs(angleToHunter - facingAngle);
        if (diff > 180) diff = 360 - diff;
        if (diff <= 30 && !m_scene->lineIntersectsObstacle(pos(), hunter->pos())) {
            hunter->stun(GameConfig::FRAMES_AIRFORCE_STUN);
            // 后撤：计算后退位置
            qreal len = sqrt(dir.x() * dir.x() + dir.y() * dir.y());
            if (len > 0.0001) {
                QPointF unit = dir / len;
                QPointF back = pos() - unit * 20;
                setPos(back);
            }
            revealPosition(GameConfig::FRAMES_AIRFORCE_STUN);
        }
    }

    setSkillCooldown();
}

void AISurvivor::takeDamage()
{
    Survivor::takeDamage();
    // 通知行为模块（如果存在）
    if (m_behavior) {
        // m_behavior->onDamageReceived(this);  // 可选
    }
}