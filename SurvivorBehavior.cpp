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
    , m_gamePhase(1) // 默认破译阶段
{
}

void SurvivorBehavior::reset()
{
    // 无持久状态需重置
}

void SurvivorBehavior::updateDecision(AISurvivor *ai)
{
    if (!ai || !m_scene || ai->isEliminated() || !ai->isEnabled()) return;

    // 如果正在交互（破译、开门、救助），则维持当前行为，除非被打断
    // （打断由 AISurvivor 自身处理，如被攻击或距离过远）
    if (ai->isDecoding() || ai->isOpeningGate() || ai->isRescuing()) {
        return;
    }

    // 空军技能：判断是否使用信号枪
    if (ai->survivortype() == SurvivorType::AirForce) {
        if (shouldUseSkill(ai)) {
            useSkillIfPossible(ai);
        }
    }

    // 优先级1：救助燃烧队友（优先玩家）
    Survivor *burningMate = findBurningTeammate(ai, true);
    if (burningMate) {
        performRescueBehavior(ai, burningMate);
        return;
    }

    // 优先级2：躲避监管者（被追击或附近有监管者）
    if (isBeingChased(ai) || isHunterNearby(ai, 150.0)) {
        ai->setBeingChased(true);
        performHideBehavior(ai);
        return;
    } else {
        ai->setBeingChased(false);
    }

    // 根据游戏阶段选择主要目标
    if (m_gamePhase == 1) { // 破译阶段
        performDecodeBehavior(ai);
    } else if (m_gamePhase == 2) { // 逃脱阶段
        performEscapeBehavior(ai);
    } else {
        ai->clearTarget(); // 准备或结算阶段，不做复杂行为
    }
}

bool SurvivorBehavior::isHunterNearby(AISurvivor *ai, qreal threshold)
{
    if (!m_hunter) return false;
    qreal dist = QLineF(ai->pos(), m_hunter->pos()).length();
    return dist < threshold;
}

bool SurvivorBehavior::isBeingChased(AISurvivor *ai)
{
    if (!m_hunter) return false;
    // 简单判断：监管者是否在附近且大致朝向我方
    qreal dist = QLineF(ai->pos(), m_hunter->pos()).length();
    return dist < 150.0; // 若距离小于150像素则认为被追击
}

CipherMachine* SurvivorBehavior::findBestCipher(AISurvivor *ai)
{
    CipherMachine *best = nullptr;
    qreal minDist = 1e9;
    for (CipherMachine *c : m_ciphers) {
        if (c->isCompleted()) continue;
        qreal d = QLineF(ai->pos(), c->pos()).length();
        if (d < minDist) {
            minDist = d;
            best = c;
        }
    }
    return best;
}

Gate* SurvivorBehavior::findNearestUnlockedGate(AISurvivor *ai)
{
    Gate *nearest = nullptr;
    qreal minDist = 1e9;
    for (Gate *g : m_gates) {
        if (!g->isUnlocked() || g->isFullyOpen()) continue;
        qreal d = QLineF(ai->pos(), g->pos()).length();
        if (d < minDist) {
            minDist = d;
            nearest = g;
        }
    }
    return nearest;
}

Survivor* SurvivorBehavior::findBurningTeammate(AISurvivor *ai, bool prioritizePlayer)
{
    // 优先玩家
    if (prioritizePlayer && m_player && m_player->isBurning() && !m_player->isEliminated()) {
        return m_player;
    }
    // 场景中查找燃烧的 AI 求生者
    QList<QGraphicsItem*> items = m_scene->items();
    Survivor *nearest = nullptr;
    qreal minDist = 1e9;
    for (QGraphicsItem *item : items) {
        Survivor *s = dynamic_cast<Survivor*>(item);
        if (!s || s == ai || s->isEliminated() || !s->isBurning()) continue;
        qreal d = QLineF(ai->pos(), s->pos()).length();
        if (d < minDist) {
            minDist = d;
            nearest = s;
        }
    }
    return nearest;
}

QPointF SurvivorBehavior::findHidingSpot(AISurvivor *ai)
{
    // 优先找最近的草丛
    Bush *bush = findNearestBush(ai);
    if (bush) {
        return bush->sceneBoundingRect().center();
    }
    // 远离监管者方向
    if (m_hunter) {
        QPointF dir = ai->pos() - m_hunter->pos();
        qreal len = QLineF(ai->pos(), m_hunter->pos()).length();
        if (len > 0) {
            dir /= len;
            return ai->pos() + dir * 80.0;
        }
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
        if (d < minDist) {
            minDist = d;
            nearest = b;
        }
    }
    return nearest;
}

void SurvivorBehavior::performDecodeBehavior(AISurvivor *ai)
{
    CipherMachine *cipher = findBestCipher(ai);
    if (cipher) {
        qreal dist = QLineF(ai->pos(), cipher->pos()).length();
        if (dist <= GameConfig::INTERACT_CIPHER_DIST) {
            // 到达密码机附近，开始破译
            ai->forceDecode(cipher);
        } else {
            ai->setTargetPosition(cipher->pos());
        }
    } else {
        // 所有密码机完成，切换到逃脱行为
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
                ai->escape(); // 大门已开，直接逃脱
            } else {
                ai->forceEscape(gate); // 开始开门
            }
        } else {
            ai->setTargetPosition(gate->pos());
        }
    } else {
        // 没有可用大门，可能都已开启，尝试直接逃脱（如果已在门前）
        // 或者闲逛
        ai->clearTarget();
    }
}

void SurvivorBehavior::performHideBehavior(AISurvivor *ai)
{
    QPointF hideSpot = findHidingSpot(ai);
    ai->setTargetPosition(hideSpot);

    // 如果已经在草丛中，可以停止移动
    Bush *nearestBush = findNearestBush(ai);
    if (nearestBush && nearestBush->contains(nearestBush->mapFromScene(ai->pos()))) {
        ai->clearTarget(); // 已在草丛中，停止移动
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

bool SurvivorBehavior::shouldUseSkill(AISurvivor *ai)
{
    if (ai->survivortype() == SurvivorType::AirForce)  return false;
    if (!ai->isSkillReady()) return false;
    if (!m_hunter) return false;

    qreal dist = QLineF(ai->pos(), m_hunter->pos()).length();
    if (dist > 120) return false; // 空军信号枪最大射程
    if (m_hunter->isStunned()) return false;

    // 自身被追击或附近有队友受伤/燃烧
    bool should = isBeingChased(ai);
    if (!should) {
        QList<QGraphicsItem*> items = m_scene->items();
        for (QGraphicsItem *item : items) {
            Survivor *s = dynamic_cast<Survivor*>(item);
            if (s && s != ai && (s->isHurt() || s->isBurning())) {
                qreal d = QLineF(ai->pos(), s->pos()).length();
                if (d < 200) {
                    should = true;
                    break;
                }
            }
        }
    }
    return should;
}

void SurvivorBehavior::useSkillIfPossible(AISurvivor *ai)
{
    if (!ai->isSkillReady()) return;
    // 调用 AI 求生者的技能释放方法（需在 AISurvivor 中实现 useSkill()）
    ai->useSkill();
}