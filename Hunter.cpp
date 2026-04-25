#include "Hunter.h"
#include "Survivor.h"
#include "GameScene.h"
#include "GameScene.h"
#include <QPainter>
#include <QLineF>
#include <QtMath>
#include <QDebug>

Hunter::Hunter(QGraphicsItem *parent)
    : Character(parent)
    , m_attacking(false)
    , m_attackCooldownTimer(0)
    , m_attackStunTimer(0)
    , m_destroying(false)
    , m_currentObstacle(nullptr)
    , m_destroyTimer(0)
    , m_stunned(false)
    , m_stunTimer(0)
    , m_hasTarget(false)
{
    setSpeedMultiplier(1.0);
    setEnabled(true);
    m_canMove = true;
}

void Hunter::updateCharacter()
{
    // 眩晕处理
    if (m_stunned) {
        if (m_stunTimer > 0) m_stunTimer--;
        if (m_stunTimer <= 0) {
            m_stunned = false;
            m_canMove = true;
        } else {
            return; // 眩晕中不能做任何动作
        }
    }

    // 攻击冷却
    if (m_attackCooldownTimer > 0) m_attackCooldownTimer--;

    // 攻击后停顿
    if (m_attackStunTimer > 0) {
        m_attackStunTimer--;
        return; // 停顿期间不能移动和做其他动作
    }

    // 破坏状态
    if (m_destroying) {
        updateDestroyProgress();
        return; // 破坏中不能移动和攻击
    }

    // 移动（由 AI 或玩家设置的目标位置）
    if (m_hasTarget && m_canMove && m_enabled) {
        moveTowardsTarget();
    } else {
        setMoveDirection(Direction::None);
    }

    // 基类移动（根据方向移动）
    Character::updateCharacter();
}

void Hunter::attack()
{
    if (!m_enabled || m_stunned || m_destroying || m_attacking) return;
    if (!isAttackReady()) return;
    if (m_attackStunTimer > 0) return;

    m_attacking = true;
    m_attackCooldownTimer = GameConfig::FRAMES_ATTACK_COOLDOWN;
    emit attackCooldownChanged(1.0);

    performAttack();
    m_attacking = false;
}

void Hunter::performAttack()
{
    QPointF hunterPos = pos();
    qreal attackRadius = GameConfig::ATTACK_RANGE;
    qreal halfAngle = GameConfig::ATTACK_ANGLE / 2.0;

    // 获取场景中所有求生者
    if (!m_scene) return;
    QList<QGraphicsItem*> items = m_scene->items();
    for (QGraphicsItem *item : items) {
        Survivor *survivor = dynamic_cast<Survivor*>(item);
        if (!survivor || survivor->isEliminated()) continue;

        QLineF line(hunterPos, survivor->pos());
        if (line.length() > attackRadius) continue;

        qreal angleDiff = 0;
        if (!isSurvivorInAttackRange(survivor, angleDiff)) continue;

        // 射线检测障碍物遮挡
        if (m_scene->lineIntersectsObstacle(hunterPos, survivor->pos())) continue;

        // 命中
        survivor->takeDamage();
        m_attackStunTimer = GameConfig::HUNTER_ATTACK_STUN_FRAMES; // 自身停顿1秒

        // 扫草逻辑：暴露草丛内求生者
        QList<Bush*> bushes = m_scene->getBushesInRadius(hunterPos, attackRadius);
        for (Bush *bush : bushes) {
            if (bush->contains(bush->mapFromScene(survivor->pos()))) {
                survivor->revealPosition(GameConfig::FRAMES_BUSH_REVEAL);
            }
        }

        break; // 单次攻击只命中一个目标
    }
}

bool Hunter::isSurvivorInAttackRange(Survivor *survivor, qreal &angleDiff) const
{
    QPointF dir = survivor->pos() - pos();
    qreal angleToSurvivor = atan2(dir.y(), dir.x()) * 180.0 / M_PI;
    if (angleToSurvivor < 0) angleToSurvivor += 360.0;

    qreal facingAngle = 0;
    switch (m_facing) {
    case Direction::Right: facingAngle = 0; break;
    case Direction::Down:  facingAngle = 90; break;
    case Direction::Left:  facingAngle = 180; break;
    case Direction::Up:    facingAngle = 270; break;
    default: return false;
    }

    angleDiff = fabs(angleToSurvivor - facingAngle);
    if (angleDiff > 180.0) angleDiff = 360.0 - angleDiff;

    return angleDiff <= GameConfig::ATTACK_HIT_ANGLE_TOLERANCE;
}

void Hunter::startDestroying(Obstacle *obstacle)
{
    if (!obstacle || m_destroying || m_stunned || !m_enabled) return;
    if (m_attackStunTimer > 0) return;

    qreal dist = QLineF(pos(), obstacle->sceneBoundingRect().center()).length();
    if (dist > GameConfig::INTERACT_DESTROY_DIST) return;

    m_destroying = true;
    m_canMove = false;
    m_currentObstacle = obstacle;

    switch (obstacle->obstacleType()) {
    case Obstacle::Wall:  m_destroyTimer = GameConfig::FRAMES_DESTROY_WALL; break;
    case Obstacle::Box:   m_destroyTimer = GameConfig::FRAMES_DESTROY_BOX; break;
    case Obstacle::Board: m_destroyTimer = GameConfig::FRAMES_DESTROY_BOARD; break;
    default: m_destroyTimer = 60; break;
    }
}

void Hunter::stopDestroying()
{
    m_destroying = false;
    m_canMove = true;
    m_currentObstacle = nullptr;
    m_destroyTimer = 0;
}

void Hunter::updateDestroyProgress()
{
    if (!m_currentObstacle) {
        stopDestroying();
        return;
    }
    // 距离过远则中断
    qreal dist = QLineF(pos(), m_currentObstacle->sceneBoundingRect().center()).length();
    if (dist > GameConfig::INTERACT_DESTROY_DIST * 1.5) {
        stopDestroying();
        return;
    }

    if (m_destroyTimer > 0) m_destroyTimer--;
    if (m_destroyTimer <= 0) {
        // 破坏完成，移除障碍物
        if (m_scene) {
            m_scene->removeItem(m_currentObstacle);
            delete m_currentObstacle; // 从场景移除并删除
        }
        emit obstacleDestroyed();
        stopDestroying();
    }
}

void Hunter::stun(int durationFrames)
{
    if (m_stunned) return;
    m_stunned = true;
    m_stunTimer = durationFrames;
    m_canMove = false;
    if (m_destroying) stopDestroying();
    m_attacking = false;
}

void Hunter::setTargetPosition(const QPointF &pos)
{
    m_targetPos = pos;
    m_hasTarget = true;
}

void Hunter::clearTarget()
{
    m_hasTarget = false;
    setMoveDirection(Direction::None);
}

void Hunter::moveTowardsTarget()
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

void Hunter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor color = QColor(180, 60, 60); // 暗红色
    if (m_stunned) {
        color = QColor(100, 100, 100);
    } else if (m_destroying) {
        color = QColor(200, 120, 60);   // 破坏中橙色
    }

    painter->setBrush(color);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawRect(boundingRect());

    // 状态文字
    if (m_destroying) {
        painter->setPen(Qt::white);
        painter->drawText(boundingRect(), Qt::AlignCenter, "破坏");
    } else if (m_stunned) {
        painter->setPen(Qt::white);
        painter->drawText(boundingRect(), Qt::AlignCenter, "眩晕");
    }
}