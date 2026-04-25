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
#include <QDebug>

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
    , m_gamePhase(1) // 默认破译阶段
{
}

void HunterBehavior::setSurvivors(const QList<AISurvivor *> &aiSurvivors, PlayerSurvivor *player)
{
    m_allSurvivors.clear();
    for (AISurvivor *ai : aiSurvivors) {
        m_allSurvivors.append(ai);
    }
    if (player) {
        m_allSurvivors.append(player);
    }
}

void HunterBehavior::reset()
{
    m_state = Patrolling;
    m_patrolIndex = 0;
    m_chaseTarget = nullptr;
    m_lostTimer = 0;
    m_searchTimer = 0;
    if (m_hunter) {
        m_hunter->clearTarget();
    }
}

void HunterBehavior::updateDecision()
{
    if (!m_hunter || !m_scene) return;
    if (!m_hunter->isEnabled() || m_hunter->isStunned()) return;

    // 如果正在破坏障碍物，不打断
    if (m_hunter->isDestroying()) {
        return;
    }

    // 处理草丛交互（晃动）
    handleBushes();

    // 状态机
    switch (m_state) {
    case Patrolling:
        patrolUpdate();
        break;
    case Chasing:
        chaseUpdate();
        break;
    case Searching:
        searchUpdate();
        break;
    }
}

// ---------- 巡逻 ----------
void HunterBehavior::patrolUpdate()
{
    // 检查附近是否有可追击的求生者
    Survivor *spotted = findNearestSurvivorInRange(GameConfig::AI_HUNTER_CHASE_DIST);
    if (spotted && canSeeSurvivor(spotted)) {
        m_state = Chasing;
        m_chaseTarget = spotted;
        m_lastSeenPos = spotted->pos();
        m_lostTimer = 0;
        return;
    }

    // 没有求生者，按固定顺序巡逻密码机
    if (m_ciphers.isEmpty()) return;

    // 跳过已完成的密码机
    int attempts = 0;
    while (attempts < m_patrolOrder.size()) {
        int idx = m_patrolOrder[m_patrolIndex];
        if (idx < m_ciphers.size() && !m_ciphers[idx]->isCompleted()) {
            break;
        }
        m_patrolIndex = (m_patrolIndex + 1) % m_patrolOrder.size();
        attempts++;
    }

    CipherMachine *targetCipher = m_ciphers.value(m_patrolOrder[m_patrolIndex], nullptr);
    if (targetCipher) {
        moveTo(targetCipher->pos());

        // 到达附近（距离 < 50）则换下一台
        qreal dist = QLineF(m_hunter->pos(), targetCipher->pos()).length();
        if (dist < 50) {
            m_patrolIndex = (m_patrolIndex + 1) % m_patrolOrder.size();
        }
    }

    // 如果路径被障碍物阻挡，尝试破坏
    tryDestroyBlockingObstacle();
}

// ---------- 追击 ----------
void HunterBehavior::chaseUpdate()
{
    if (!m_chaseTarget || m_chaseTarget->isEliminated()) {
        m_state = Patrolling;
        m_chaseTarget = nullptr;
        m_hunter->clearTarget();
        return;
    }

    bool visible = canSeeSurvivor(m_chaseTarget);
    qreal dist = QLineF(m_hunter->pos(), m_chaseTarget->pos()).length();

    if (visible && dist <= GameConfig::AI_HUNTER_CHASE_DIST) {
        // 仍然可见，更新位置和丢失计时
        m_lastSeenPos = m_chaseTarget->pos();
        m_lostTimer = 0;

        // 如果进入攻击范围且角度合适，尝试攻击
        tryAttack(m_chaseTarget);

        // 向目标移动
        moveTo(m_chaseTarget->pos());
    } else {
        // 丢失视野或超出追击距离
        m_lostTimer++;

        bool giveUp = false;
        if (dist > GameConfig::AI_HUNTER_LOSE_DIST && m_lostTimer >= 50) {
            giveUp = true; // 距离 >200 且丢失超过 10 秒 (50*0.2s=10s)
        } else if (m_lostTimer >= LOSE_TIMEOUT_TICKS) {
            giveUp = true; // 丢失超过 8 秒
        }

        if (giveUp) {
            // 切换到搜索状态
            m_state = Searching;
            m_searchTimer = 0;
        } else {
            // 继续向最后位置移动
            moveTo(m_lastSeenPos);
        }
    }

    tryDestroyBlockingObstacle();
}

// ---------- 搜索 ----------
void HunterBehavior::searchUpdate()
{
    moveTo(m_lastSeenPos);

    qreal dist = QLineF(m_hunter->pos(), m_lastSeenPos).length();
    if (dist < 30) {
        // 到达搜索点，停留
        m_hunter->clearTarget();
        m_searchTimer++;
    } else {
        m_searchTimer = 0;
        tryDestroyBlockingObstacle();
    }

    // 搜索过程中再次发现求生者？
    Survivor *spotted = findNearestSurvivorInRange(GameConfig::AI_HUNTER_CHASE_DIST);
    if (spotted && canSeeSurvivor(spotted)) {
        m_state = Chasing;
        m_chaseTarget = spotted;
        m_lastSeenPos = spotted->pos();
        m_lostTimer = 0;
        m_searchTimer = 0;
        return;
    }

    // 搜索结束，返回巡逻
    if (m_searchTimer >= SEARCH_DURATION_TICKS) {
        m_state = Patrolling;
        m_chaseTarget = nullptr;
        m_searchTimer = 0;
    }
}

// ---------- 辅助函数 ----------

Survivor* HunterBehavior::findNearestSurvivorInRange(qreal range)
{
    Survivor *nearest = nullptr;
    qreal minDist = range;
    for (Survivor *s : m_allSurvivors) {
        if (s->isEliminated()) continue;
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
    // 草丛隐身：求生者在草丛内且未被显形时不可见
    if (survivor->isInBush() && !survivor->isEliminated()) {
        return false;
    }
    // 射线检测障碍物
    return !m_scene->lineIntersectsObstacle(m_hunter->pos(), survivor->pos());
}

void HunterBehavior::moveTo(const QPointF &target)
{
    if (!m_hunter) return;
    // 设置移动目标，角色每帧通过 updateCharacter 移动
    // 这里使用简单的方向设置，也可使用 Hunter::setTargetPosition 如果实现了
    QPointF dir = target - m_hunter->pos();
    if (dir.manhattanLength() < 5) {
        m_hunter->setMoveDirection(Direction::None);
    } else {
        if (qAbs(dir.x()) > qAbs(dir.y()))
            m_hunter->setMoveDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
        else
            m_hunter->setMoveDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
    }
}

bool HunterBehavior::isPathBlocked(const QPointF &from, const QPointF &to)
{
    if (!m_scene) return false;
    return m_scene->lineIntersectsObstacle(from, to);
}

void HunterBehavior::tryDestroyBlockingObstacle()
{
    if (!m_hunter || !m_scene) return;
    if (m_hunter->isDestroying()) return;

    // 检查前方是否有阻碍移动的障碍物（距离 ≤20）
    QPointF hunterPos = m_hunter->pos();
    QList<QGraphicsItem*> obsList = m_scene->getObstaclesInRadius(hunterPos, GameConfig::INTERACT_DESTROY_DIST + 10);
    for (QGraphicsItem *item : obsList) {
        Obstacle *obs = dynamic_cast<Obstacle*>(item);
        if (!obs) continue;
        qreal dist = QLineF(hunterPos, obs->sceneBoundingRect().center()).length();
        if (dist <= GameConfig::INTERACT_DESTROY_DIST) {
            // 检查是否确实阻挡了去路（简化：如果监管者有移动方向且障碍物在方向上）
            m_hunter->startDestroying(obs);
            return; // 一次只破坏一个
        }
    }
}

void HunterBehavior::handleBushes()
{
    if (!m_hunter || !m_scene) return;
    QPointF pos = m_hunter->pos();
    QList<Bush*> nearby = m_scene->getBushesInRadius(pos, GameConfig::BUSH_SHAKE_DIST);
    for (Bush *bush : nearby) {
        bush->shake();
    }
    // 注意：攻击扫草在 Hunter::performAttack 中处理
}

void HunterBehavior::tryAttack(Survivor *target)
{
    if (!m_hunter || !target) return;
    if (!m_hunter->isAttackReady()) return;

    // 检查角度（简化：使用 Hunter 自身的攻击判断，这里只做基本距离和方向预判）
    qreal dist = QLineF(m_hunter->pos(), target->pos()).length();
    if (dist > GameConfig::ATTACK_RANGE) return;

    // 简单朝向判断：监管者朝向与目标方向夹角是否在 ±45° 内
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