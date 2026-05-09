#include "SurvivorBehavior.h"
#include "GameScene.h"
#include "entities/AISurvivor.h"
#include "entities/PlayerSurvivor.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include "entities/Hunter.h"
#include "entities/Survivor.h"
#include "GameConfig.h"
#include <QLineF>
#include <QRandomGenerator>
#include <QDebug>

SurvivorBehavior::SurvivorBehavior(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_hunter(nullptr)
    , m_player(nullptr)
    , m_gamePhase(1)
{
}

void SurvivorBehavior::reset() { }

void SurvivorBehavior::setSurvivors(const QList<AISurvivor*> &ai, PlayerSurvivor *player)
{
    m_aiSurvivors = ai;
    m_player = player;
}

// ====== 核心决策函数 ======
void SurvivorBehavior::updateDecision(AISurvivor *ai)
{
    if (!ai || !m_scene || ai->isEliminated() || !ai->isEnabled()) return;

    // 如果正在交互（破译/开门/救人/治疗），不打扰
    if (ai->isDecoding() || ai->isOpeningGate() || ai->isRescuing() || ai->isHealing())
        return;

    // 空军技能优先判断
    if (ai->survivortype() == SurvivorType::AirForce) {
        if (shouldUseSkill(ai)) useSkillIfPossible(ai);
    }

    // ---------- 优先级 1：救助倒地队友 ----------
    Survivor *burningMate = findBurningTeammate(ai, true);
    if (burningMate) {
        performRescueBehavior(ai, burningMate);
        return;
    }

    // ---------- 优先级 2：躲避监管者 ----------
    if (ai->isBeingChased() || isHunterNearby(ai, 150.0)) {
        ai->setBeingChased(true);
        if (m_gamePhase == 2) {
            Gate *gate = findNearestUnlockedGate(ai);
            if (gate && QLineF(ai->pos(), gate->pos()).length() < 300) {
                ai->setTargetPosition(gate->pos());
                return;
            }
        }
        performHideBehavior(ai);
        return;
    } else {
        ai->setBeingChased(false);
    }

    // ---------- 优先级 3：安全且附近有受伤玩家时去治疗 ----------
    if (!ai->isBeingChased() && !isHunterNearby(ai, 150.0)) {
        Survivor *injuredPlayer = findInjuredPlayerNearby(ai);
        if (injuredPlayer) {
            performHealBehavior(ai, injuredPlayer);
            return;
        }
    }

    // ---------- 优先级 4：常规任务（破译 / 逃脱）----------
    if (m_gamePhase == 1) {
        performDecodeBehavior(ai);
    } else if (m_gamePhase == 2) {
        performEscapeBehavior(ai);
    } else {
        ai->clearTarget();
    }

    // 强制分配目标，避免 AI 发呆
    if (!ai->hasTarget() && !ai->isDecoding() && !ai->isOpeningGate()
        && !ai->isRescuing() && !ai->isHealing()) {
        if (m_gamePhase == 1) {
            CipherMachine *cipher = findBestCipher(ai);
            if (cipher) ai->setTargetPosition(cipher->pos());
        } else if (m_gamePhase == 2) {
            Gate *gate = findNearestUnlockedGate(ai);
            if (gate) ai->setTargetPosition(gate->pos());
        }
    }
}

Survivor* SurvivorBehavior::findInjuredPlayerNearby(AISurvivor *ai)
{
    // 仅当人类玩家存在、受伤且未倒地
    if (!m_player || !m_player->isHurt() || m_player->isBurning())
        return nullptr;

    // 距离必须非常近
    qreal dist = QLineF(ai->pos(), m_player->pos()).length();
    if (dist > 80.0)  return nullptr;

    // 监管者不能靠近玩家或 AI 自身
    if (m_hunter) {
        qreal hunterToPlayer = QLineF(m_hunter->pos(), m_player->pos()).length();
        qreal hunterToAI = QLineF(m_hunter->pos(), ai->pos()).length();
        if (hunterToPlayer < 150.0 || hunterToAI < 150.0)
            return nullptr;
    }
    return m_player;
}
// ----- 辅助查找函数 -----
Survivor* SurvivorBehavior::findBurningTeammate(AISurvivor *ai, bool prioritizePlayer)
{
    if (prioritizePlayer && m_player && m_player->isBurning() && !m_player->isEliminated())
        return m_player;

    Survivor *nearest = nullptr;
    qreal minDist = 1e9;
    for (AISurvivor *other : m_aiSurvivors) {
        if (other == ai || other->isEliminated() || !other->isBurning()) continue;
        qreal d = QLineF(ai->pos(), other->pos()).length();
        if (d < minDist) { minDist = d; nearest = other; }
    }
    return nearest;
}

Survivor* SurvivorBehavior::findInjuredTeammate(AISurvivor *ai, bool prioritizePlayer)
{
    if (prioritizePlayer && m_player && m_player->isHurt() && !m_player->isEliminated())
        return m_player;

    Survivor *nearest = nullptr;
    qreal minDist = 1e9;
    for (AISurvivor *other : m_aiSurvivors) {
        if (other == ai || other->isEliminated() || !other->isHurt()) continue;
        qreal d = QLineF(ai->pos(), other->pos()).length();
        if (d < minDist) { minDist = d; nearest = other; }
    }
    return nearest;
}

bool SurvivorBehavior::isHunterNearby(AISurvivor *ai, qreal threshold)
{
    if (!m_hunter) return false;
    return QLineF(ai->pos(), m_hunter->pos()).length() < threshold;
}

bool SurvivorBehavior::isBeingChased(AISurvivor *ai)
{
    if (!m_hunter) return false;
    return QLineF(ai->pos(), m_hunter->pos()).length() < 150.0;
}

CipherMachine* SurvivorBehavior::findBestCipher(AISurvivor *ai)
{
    CipherMachine *best = nullptr;
    qreal minDist = 1e9;
    for (CipherMachine *c : m_ciphers) {
        if (c->isCompleted()) continue;
        qreal d = QLineF(ai->pos(), c->pos()).length();
        if (d < minDist) { minDist = d; best = c; }
    }
    return best;
}

Gate* SurvivorBehavior::findNearestUnlockedGate(AISurvivor *ai)
{
    Gate *nearest = nullptr;
    qreal minDist = 1e9;
    for (Gate *g : m_gates) {
        if (!g->isUnlocked()) continue;          // 只要已解锁（包括已完全打开的门）
        qreal d = QLineF(ai->pos(), g->pos()).length();
        if (d < minDist) { minDist = d; nearest = g; }
    }
    return nearest;
}

// ----- 躲藏点选择 -----
QPointF SurvivorBehavior::findHidingSpot(AISurvivor *ai)
{
    if (!m_hunter || !m_scene) return ai->pos();

    // 1. 优先找远离监管者的草丛
    Bush *bestBush = nullptr;
    qreal bestScore = -1e9;
    QList<Bush*> bushes = m_scene->getBushes();
    for (Bush *b : bushes) {
        QPointF bushCenter = b->sceneBoundingRect().center();
        qreal distToHunter = QLineF(bushCenter, m_hunter->pos()).length();
        qreal distToAI = QLineF(ai->pos(), bushCenter).length();
        qreal score = distToHunter * 0.7 - distToAI * 0.3;   // 远监管者 + 近自己
        if (score > bestScore) {
            bestScore = score;
            bestBush = b;
        }
    }
    if (bestBush) return bestBush->sceneBoundingRect().center();

    // 2. 找障碍物后方遮断视线的位置
    QList<QGraphicsItem*> obsList = m_scene->getObstaclesInRadius(ai->pos(), 250);
    QPointF bestSpot = ai->pos();
    bestScore = -1e9;
    for (auto *item : obsList) {
        Obstacle *obs = dynamic_cast<Obstacle*>(item);
        if (!obs) continue;
        QPointF obsCenter = obs->sceneBoundingRect().center();
        QPointF dirToHunter = m_hunter->pos() - obsCenter;
        qreal len = QLineF(obsCenter, m_hunter->pos()).length();
        if (len < 1) continue;
        QPointF hideSpot = obsCenter - dirToHunter / len * 70;
        qreal distToAI = QLineF(ai->pos(), hideSpot).length();
        qreal distToHunter = QLineF(hideSpot, m_hunter->pos()).length();
        bool blockView = m_scene->lineIntersectsObstacle(hideSpot, m_hunter->pos());
        qreal score = (blockView ? 200 : 0) + distToHunter * 0.5 - distToAI * 0.5;
        if (score > bestScore) {
            bestScore = score;
            bestSpot = hideSpot;
        }
    }
    if (bestScore > 0) return bestSpot;

    // 3. 兜底：远离监管者方向跑150像素
    QPointF away = ai->pos() - m_hunter->pos();
    qreal len = QLineF(ai->pos(), m_hunter->pos()).length();
    if (len > 0) {
        away /= len;
        return ai->pos() + away * 150.0;
    }
    return ai->pos();
}

Bush* SurvivorBehavior::findNearestBush(AISurvivor *ai)
{
    if (!m_scene) return nullptr;
    QList<Bush*> bushes = m_scene->getBushes();
    Bush *nearest = nullptr;
    qreal minDist = 1e9;
    for (Bush *b : bushes) {
        qreal d = QLineF(ai->pos(), b->sceneBoundingRect().center()).length();
        if (d < minDist) { minDist = d; nearest = b; }
    }
    return nearest;
}

// ----- 各项行为实现 -----
void SurvivorBehavior::performDecodeBehavior(AISurvivor *ai)
{
    CipherMachine *cipher = findBestCipher(ai);
    if (cipher) {
        qreal dist = QLineF(ai->pos(), cipher->pos()).length();
        if (dist <= GameConfig::INTERACT_CIPHER_DIST) {
            ai->forceDecode(cipher);
        } else {
            ai->setTargetPosition(cipher->pos());
        }
    } else {
        performEscapeBehavior(ai);
    }
}

void SurvivorBehavior::performEscapeBehavior(AISurvivor *ai)
{
    Gate *gate = findNearestUnlockedGate(ai);
    if (gate) {
        qreal dist = QLineF(ai->pos(), gate->pos()).length();
        if (dist <= GameConfig::INTERACT_GATE_DIST) {
            if (gate->isFullyOpen()) {
                ai->escape();
            } else {
                ai->forceEscape(gate);
            }
        } else {
            ai->setTargetPosition(gate->pos());
        }
    } else {
        ai->clearTarget();
    }
}

void SurvivorBehavior::performHideBehavior(AISurvivor *ai)
{
    QPointF hideSpot = findHidingSpot(ai);
    ai->setTargetPosition(hideSpot);

    Bush *nearestBush = findNearestBush(ai);
    if (nearestBush && nearestBush->contains(nearestBush->mapFromScene(ai->pos()))) {
        ai->clearTarget();   // 已进入草丛，停止移动

        // ✅ 安全判断：如果监管者距离较远（>250），主动离开草丛，重回破译/逃脱任务
        if (m_hunter) {
            qreal distToHunter = QLineF(ai->pos(), m_hunter->pos()).length();
            if (distToHunter > 250.0) {
                ai->setBeingChased(false);   // 解除追击状态，让行为树重新分配任务
                // 下一帧的 updateDecision 会检测到没有目标且未被追击，自动分配新目标
            }
        }
    }
}

void SurvivorBehavior::performRescueBehavior(AISurvivor *ai, Survivor *target)
{
    if (!target) return;
    qreal dist = QLineF(ai->pos(), target->pos()).length();
    if (dist <= GameConfig::INTERACT_RESCUE_DIST) {
        ai->forceRescue(target);
    } else {
        ai->setTargetPosition(target->pos());
    }
}

void SurvivorBehavior::performHealBehavior(AISurvivor *ai, Survivor *target)
{
    if (!target) return;
    qreal dist = QLineF(ai->pos(), target->pos()).length();
    if (dist <= GameConfig::INTERACT_RESCUE_DIST) {
        ai->startHealing(target);   // 距离足够，开始治疗
    } else {
        ai->setTargetPosition(target->pos());
    }
}

// ----- 技能判断（空军）-----
bool SurvivorBehavior::shouldUseSkill(AISurvivor *ai)
{
    if (!ai->isSkillReady()) return false;
    if (!m_hunter) return false;

    qreal dist = QLineF(ai->pos(), m_hunter->pos()).length();
    if (dist > 130) return false;
    if (m_hunter->isStunned()) return false;

    // 自己受伤或监管者很近
    if (ai->isHurt() || dist < 80) return true;

    // 附近队友受伤或倒地
    for (AISurvivor *other : m_aiSurvivors) {
        if (other == ai || other->isEliminated()) continue;
        if (other->isHurt() || other->isBurning()) {
            qreal d = QLineF(ai->pos(), other->pos()).length();
            if (d < 200) return true;
        }
    }
    return false;
}

void SurvivorBehavior::useSkillIfPossible(AISurvivor *ai)
{
    if (!ai->isSkillReady()) return;
    ai->useSkill();
}