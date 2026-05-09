#include "AISurvivor.h"
#include "ai/SurvivorBehavior.h"
#include "CipherMachine.h"
#include "Gate.h"
#include "Hunter.h"
#include "GameScene.h"
#include "GameConfig.h"
#include <QLineF>
#include <QRandomGenerator>
#include <QtMath>
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
    , m_chaseTimer(0)
{
    setEnabled(true);
}

void AISurvivor::updateCharacter()
{
    constexpr int halfW = 12;
    constexpr int halfH = 12;

    Survivor::updateCharacter();

    if (!m_enabled || m_eliminated || m_burning) return;
    if (m_skillCooldownTimer > 0) m_skillCooldownTimer--;

    // 被追击计时器递减
    if (m_chaseTimer > 0) {
        m_chaseTimer--;
        if (m_chaseTimer == 0)
            setBeingChased(false);
        if (m_hunter) {
            qreal d = QLineF(pos(), m_hunter->pos()).length();
            if (d > 200.0) {
                m_chaseTimer = 0;
                setBeingChased(false);
            }
        }
    }
    // 若正在交互（包括治疗），但监管者靠近则中断并逃跑
    if (m_hunter && (m_decoding || m_openingGate || m_rescuing || m_healing)) {
        qreal distToHunter = QLineF(pos(), m_hunter->pos()).length();
        if (distToHunter < 180.0) {
            if (m_decoding) stopDecoding();
            if (m_openingGate) stopOpeningGate();
            if (m_rescuing) stopRescuing();
            if (m_healing) stopHealing();        // 新增：中断治疗
            QPointF away = pos() - m_hunter->pos();
            qreal len = QLineF(pos(), m_hunter->pos()).length();
            if (len > 0) {
                qreal angle = atan2(away.y(), away.x());
                qreal randAngle = (QRandomGenerator::global()->bounded(60) - 30) * M_PI / 180.0;
                QPointF dir(cos(angle + randAngle), sin(angle + randAngle));
                setTargetPosition(pos() + dir * 120.0);
            }
            setBeingChased(true);
            m_chaseTimer = 120;
            return;
        }
    }

    // 强制指令处理 (略，与之前相同)
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
            if (m_forcedGate->isFullyOpen()) escape();
            else startOpeningGate(m_forcedGate);
            m_forcedGate = nullptr;
            clearTarget();
        } else {
            setTargetPosition(m_forcedGate->pos());
        }
    }
    // 移动
    if (m_hasTarget && m_canMove) {
        QPointF dir = m_targetPos - pos();
        qreal len = QLineF(pos(), m_targetPos).length();
        if (len < 5.0) {
            clearTarget();
        } else {
            QPointF step = dir / len * currentSpeed();
            QPointF newPos = pos() + step;

            if (newPos.x() < halfW) newPos.setX(halfW);
            if (newPos.x() > GameConfig::MAP_WIDTH - halfW) newPos.setX(GameConfig::MAP_WIDTH - halfW);
            if (newPos.y() < halfH) newPos.setY(halfH);
            if (newPos.y() > GameConfig::MAP_HEIGHT - halfH) newPos.setY(GameConfig::MAP_HEIGHT - halfH);
            setPos(newPos);

            if (qAbs(dir.x()) > qAbs(dir.y())) {
                setFacingDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
            } else {
                setFacingDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
            }
        }
    }
    // 动画
    if (m_animator) {
        if (m_direction != Direction::None && m_canMove) {
            m_animator->setDirection(m_direction);
            m_animator->startAnimation();
        } else {
            m_animator->stopAnimation();
        }
    }
}

void AISurvivor::setTargetPosition(const QPointF &pos) { m_targetPos = pos; m_hasTarget = true; }
void AISurvivor::clearTarget() { m_hasTarget = false; setMoveDirection(Direction::None); }
void AISurvivor::forceDecode(CipherMachine *cipher) { m_forcedCipher = cipher; m_forcedRescueTarget = nullptr; m_forcedGate = nullptr; }
void AISurvivor::forceRescue(Survivor *target) { m_forcedRescueTarget = target; m_forcedCipher = nullptr; m_forcedGate = nullptr; }
void AISurvivor::forceEscape(Gate *gate) { m_forcedGate = gate; m_forcedCipher = nullptr; m_forcedRescueTarget = nullptr; }
void AISurvivor::forceHide() { }

void AISurvivor::setSkillCooldown() {
    switch (m_type) {
    case SurvivorType::Doctor:   m_skillCooldownTimer = GameConfig::FRAMES_DOCTOR_COOLDOWN; break;
    case SurvivorType::Mechanic: m_skillCooldownTimer = GameConfig::FRAMES_MECHANIC_COOLDOWN; break;
    case SurvivorType::AirForce: m_skillCooldownTimer = GameConfig::FRAMES_AIRFORCE_COOLDOWN; break;
    default: break;
    }
}

void AISurvivor::useSkill() {
    if (!isSkillReady()) return;
    if (m_type == SurvivorType::Doctor) {
        Survivor *target = nullptr;
        qreal minDist = 30.0;
        if (isHurt()) target = this;
        else {
            QList<QGraphicsItem*> items = m_scene->items();
            for (QGraphicsItem *item : items) {
                Survivor *s = dynamic_cast<Survivor*>(item);
                if (s && s != this && s->isHurt() && !s->isEliminated()) {
                    qreal d = QLineF(pos(), s->pos()).length();
                    if (d < minDist) { minDist = d; target = s; }
                }
            }
        }
        if (target) {
            target->heal();
            revealPosition(GameConfig::FRAMES_DOCTOR_HEAL);
        }
    }
    else if (m_type == SurvivorType::Mechanic) {
        if (m_scene) {
            MechanicalPuppet *puppet = new MechanicalPuppet(pos(), m_scene);
            CipherMachine *target = nullptr;
            qreal minDist = 1e9;
            for (CipherMachine *cipher : m_ciphers) {
                if (!cipher->isCompleted()) {
                    qreal d = QLineF(pos(), cipher->pos()).length();
                    if (d < minDist) { minDist = d; target = cipher; }
                }
            }
            if (target) puppet->moveToTarget(target->pos());
        }
    }
    else if (m_type == SurvivorType::AirForce) {
        if (!m_hunter) return;
        qreal dist = QLineF(pos(), m_hunter->pos()).length();
        if (dist > 120) return;
        QPointF dir = m_hunter->pos() - pos();
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
        if (diff <= 30 && !m_scene->lineIntersectsObstacle(pos(), m_hunter->pos())) {
            m_hunter->stun(GameConfig::FRAMES_AIRFORCE_STUN);
            qreal len = sqrt(dir.x()*dir.x() + dir.y()*dir.y());
            if (len > 0.0001) {
                QPointF unit = dir / len;
                QPointF back = pos() - unit * 20;
                moveWithCollision(back);
            }
            revealPosition(GameConfig::FRAMES_AIRFORCE_STUN);
        }
    }
    setSkillCooldown();
}

void AISurvivor::takeDamage() {
    Survivor::takeDamage();
    if (!m_eliminated && m_hunter) {
        m_forcedCipher = nullptr;
        m_forcedRescueTarget = nullptr;
        m_forcedGate = nullptr;
        QPointF away = pos() - m_hunter->pos();
        qreal len = QLineF(pos(), m_hunter->pos()).length();
        if (len < 1.0) len = 1.0;
        qreal angle = atan2(away.y(), away.x());
        qreal randAngle = (QRandomGenerator::global()->bounded(40) - 20) * M_PI / 180.0;
        QPointF dir(cos(angle + randAngle), sin(angle + randAngle));
        setTargetPosition(pos() + dir * 130.0);
        setBeingChased(true);
        m_chaseTimer = 150;
    }
}