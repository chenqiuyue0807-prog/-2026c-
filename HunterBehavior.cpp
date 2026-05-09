#include "HunterBehavior.h"
#include "GameScene.h"
#include "entities/Hunter.h"
#include "entities/Survivor.h"
#include "entities/AISurvivor.h"
#include "entities/PlayerSurvivor.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include "GameConfig.h"
#include <QLineF>
#include <QtMath>

HunterBehavior::HunterBehavior(QObject *parent)
    : QObject(parent)
    , m_patrolIndex(0)
    , m_patrolOrder({0, 1, 2})
    , m_state(Patrolling)
    , m_chaseTarget(nullptr)
    , m_lostTimer(0)
    , m_searchTimer(0)
    , m_scene(nullptr)
    , m_hunter(nullptr)
    , m_gamePhase(1)
{
}

void HunterBehavior::setSurvivors(const QList<AISurvivor *> &aiSurvivors, PlayerSurvivor *player)
{
    m_allSurvivors.clear();
    for (AISurvivor *ai : aiSurvivors) {
        if (ai && !ai->isEliminated() && !ai->isBurning())
            m_allSurvivors.append(ai);
    }
    if (player && !player->isEliminated() && !player->isBurning())
        m_allSurvivors.append(player);
}

void HunterBehavior::reset()
{
    m_state = Patrolling;
    m_patrolIndex = 0;
    m_chaseTarget = nullptr;
    m_lostTimer = 0;
    m_searchTimer = 0;
    if (m_hunter) m_hunter->clearTarget();
}

void HunterBehavior::updateDecision()
{
    if (!m_hunter || !m_scene) return;
    if (!m_hunter->isEnabled() || m_hunter->isStunned()) return;
    if (m_hunter->isDestroying()) return;

    handleBushes();

    switch (m_state) {
    case Patrolling: patrolUpdate(); break;
    case Chasing:    chaseUpdate(); break;
    case Searching:  searchUpdate(); break;
    }
}

void HunterBehavior::patrolUpdate()
{
    Survivor *spotted = findNearestSurvivorInRange(GameConfig::AI_HUNTER_CHASE_DIST);
    if (spotted && canSeeSurvivor(spotted)) {
        m_state = Chasing;
        m_chaseTarget = spotted;
        m_lastSeenPos = spotted->pos();
        m_lastSeePos = spotted->pos();   // 初始化预测用位置
        m_lostTimer = 0;
        return;
    }

    if (m_ciphers.isEmpty()) return;
    int attempts = 0;
    while (attempts < m_patrolOrder.size()) {
        int idx = m_patrolOrder[m_patrolIndex];
        if (idx < m_ciphers.size() && !m_ciphers[idx]->isCompleted()) break;
        m_patrolIndex = (m_patrolIndex + 1) % m_patrolOrder.size();
        attempts++;
    }
    CipherMachine *targetCipher = m_ciphers.value(m_patrolOrder[m_patrolIndex], nullptr);
    if (targetCipher) {
        m_hunter->setTargetPosition(targetCipher->pos());
        qreal dist = QLineF(m_hunter->pos(), targetCipher->pos()).length();
        if (dist < 50) m_patrolIndex = (m_patrolIndex + 1) % m_patrolOrder.size();
    }
    tryDestroyBlockingObstacle();
}

void HunterBehavior::chaseUpdate()
{
    if (!m_chaseTarget || m_chaseTarget->isEliminated() || m_chaseTarget->isBurning()) {
        if (m_chaseTarget && m_chaseTarget->isBurning()) {
            m_allSurvivors.removeOne(m_chaseTarget);
        }
        m_state = Patrolling;
        m_chaseTarget = nullptr;
        m_hunter->clearTarget();
        return;
    }

    bool visible = canSeeSurvivor(m_chaseTarget);
    qreal dist = QLineF(m_hunter->pos(), m_chaseTarget->pos()).length();

    if (visible && dist <= GameConfig::AI_HUNTER_CHASE_DIST) {
        // 更新最后一次看到的位置
        m_lastSeenPos = m_chaseTarget->pos();

        // 预测目标移动
        QPointF targetPos = m_chaseTarget->pos();
        if (!m_lastSeePos.isNull()) {
            QPointF moveVec = targetPos - m_lastSeePos;
            qreal moveLen = QLineF(m_lastSeePos, targetPos).length();
            if (moveLen > 1.0) {
                QPointF predictVec = moveVec / moveLen * 30.0;   // 预测领先30像素
                targetPos = targetPos + predictVec;
            }
        }
        m_lastSeePos = m_chaseTarget->pos();   // 记录本次位置供下次用

        tryAttack(m_chaseTarget);
        m_hunter->setTargetPosition(targetPos);   // 向预测点移动
        m_lostTimer = 0;
    } else {
        m_lostTimer++;
        bool giveUp = false;
        if (dist > GameConfig::AI_HUNTER_LOSE_DIST && m_lostTimer >= 50) giveUp = true;
        else if (m_lostTimer >= LOSE_TIMEOUT_TICKS) giveUp = true;
        if (giveUp) {
            m_state = Searching;
            m_searchTimer = 0;
        } else {
            m_hunter->setTargetPosition(m_lastSeenPos);
        }
    }
    tryDestroyBlockingObstacle();
}

void HunterBehavior::searchUpdate()
{
    m_hunter->setTargetPosition(m_lastSeenPos);
    qreal dist = QLineF(m_hunter->pos(), m_lastSeenPos).length();
    if (dist < 30) {
        m_hunter->clearTarget();
        m_searchTimer++;
    } else {
        m_searchTimer = 0;
        tryDestroyBlockingObstacle();
    }
    Survivor *spotted = findNearestSurvivorInRange(GameConfig::AI_HUNTER_CHASE_DIST);
    if (spotted && canSeeSurvivor(spotted)) {
        m_state = Chasing;
        m_chaseTarget = spotted;
        m_lastSeenPos = spotted->pos();
        m_lastSeePos = spotted->pos();
        m_lostTimer = 0;
        m_searchTimer = 0;
        return;
    }
    if (m_searchTimer >= SEARCH_DURATION_TICKS) {
        m_state = Patrolling;
        m_chaseTarget = nullptr;
        m_searchTimer = 0;
    }
}

Survivor* HunterBehavior::findNearestSurvivorInRange(qreal range)
{
    Survivor *nearest = nullptr;
    qreal minDist = range;
    for (Survivor *s : m_allSurvivors) {
        if (s->isEliminated() || s->isBurning()) continue;
        qreal d = QLineF(m_hunter->pos(), s->pos()).length();
        if (d < minDist) {
            minDist = d;
            nearest = s;
        }
    }
    return nearest;
}

bool HunterBehavior::canSeeSurvivor(Survivor *survivor)
{
    if (!m_scene) return false;
    if (survivor->isInBush() && !survivor->isEliminated()) return false;
    return !m_scene->lineIntersectsObstacle(m_hunter->pos(), survivor->pos());
}

void HunterBehavior::tryDestroyBlockingObstacle()
{
    if (!m_hunter || !m_scene) return;
    if (m_hunter->isDestroying()) return;
    QPointF hunterPos = m_hunter->pos();
    QList<QGraphicsItem*> obsList = m_scene->getObstaclesInRadius(hunterPos, GameConfig::INTERACT_DESTROY_DIST + 10);
    for (QGraphicsItem *item : obsList) {
        Obstacle *obs = dynamic_cast<Obstacle*>(item);
        if (!obs) continue;
        qreal dist = QLineF(hunterPos, obs->sceneBoundingRect().center()).length();
        if (dist <= GameConfig::INTERACT_DESTROY_DIST) {
            m_hunter->startDestroying(obs);
            return;
        }
    }
}

void HunterBehavior::handleBushes()
{
    if (!m_hunter || !m_scene) return;
    QPointF pos = m_hunter->pos();
    QList<Bush*> nearby = m_scene->getBushesInRadius(pos, GameConfig::BUSH_SHAKE_DIST);
    for (Bush *bush : nearby) bush->shake();
}

void HunterBehavior::tryAttack(Survivor *target)
{
    if (!m_hunter || !target) return;
    if (!m_hunter->isAttackReady()) return;

    qreal dist = QLineF(m_hunter->pos(), target->pos()).length();
    if (dist > GameConfig::ATTACK_RANGE) return;

    QPointF dir = target->pos() - m_hunter->pos();
    qreal angleToTarget = atan2(dir.y(), dir.x()) * 180.0 / M_PI;
    if (angleToTarget < 0) angleToTarget += 360.0;
    qreal facingAngle = 0;
    switch (m_hunter->facingDirection()) {
    case Direction::Right: facingAngle = 0; break;
    case Direction::Down:  facingAngle = 90; break;
    case Direction::Left:  facingAngle = 180; break;
    case Direction::Up:    facingAngle = 270; break;
    default: return;
    }
    qreal diff = fabs(angleToTarget - facingAngle);
    if (diff > 180.0) diff = 360.0 - diff;
    if (diff <= GameConfig::ATTACK_HIT_ANGLE_TOLERANCE) {
        m_hunter->attack();
    }
}