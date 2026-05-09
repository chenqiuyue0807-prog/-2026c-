#include "Hunter.h"
#include "Survivor.h"
#include "GameScene.h"
#include "entities/MechanicalPuppet.h"
#include <QPainter>
#include <QLineF>
#include <QtMath>
#include <QDebug>

Hunter::Hunter(QGraphicsItem *parent)
    : Character(parent)
    , m_attacking(false)
    , m_attackCooldownTimer(0)
    // 删除 m_attackStunTimer(0)
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

    m_animator = new CharacterAnimator(this);
    m_animator->setFrameInterval(80);

    auto loadImg = [&](const QString &path, const QString &direction) {
        QPixmap pm(path);
        if (pm.isNull()) {
            qDebug() << "❌ Hunter: " << direction << "图片加载失败:" << path;
        }
        return pm.scaled(DEFAULT_WIDTH, DEFAULT_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    };
    m_animator->setDownPixmap (loadImg(":/new/prefix2/images/hongz.png",   "下"));
    m_animator->setUpPixmap   (loadImg(":/new/prefix2/images/hongh.png", "上"));
    m_animator->setLeftPixmap (loadImg(":/new/prefix2/images/hongzc.png",  "左"));
    m_animator->setRightPixmap(loadImg(":/new/prefix2/images/hongyc.png",  "右"));

    // 攻击特效图片
    m_attackEffectPixmap.load(":/new/prefix1/images/gj.png");
    if (!m_attackEffectPixmap.isNull()) {
        m_attackEffectPixmap = m_attackEffectPixmap.scaled(GameConfig::ATTACK_RANGE * 2,
                                                           GameConfig::ATTACK_RANGE * 2,
                                                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
}

void Hunter::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 绘制角色本体
    if (m_animator) {
        QPixmap current = m_animator->currentPixmap();
        if (!current.isNull()) {
            painter->drawPixmap(boundingRect().toRect(), current);
        }
    }

    // 攻击特效
    if (m_attackEffectTimer > 0 && !m_attackEffectPixmap.isNull()) {
        qreal alpha = m_attackEffectTimer / 15.0;
        painter->setOpacity(alpha * 0.6);
        QPointF localCenter = m_attackEffectPos - pos();
        QSizeF half = m_attackEffectPixmap.size() / 2;
        QPointF topLeft = localCenter - QPointF(half.width(), half.height());
        painter->drawPixmap(topLeft, m_attackEffectPixmap);
        painter->setOpacity(1.0);
    }

    // 状态文字（破坏或眩晕）—— 加大眩晕显示
    if (m_destroying) {
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 12, QFont::Bold));
        painter->drawText(boundingRect(), Qt::AlignCenter, "破坏");
    } else if (m_stunned) {
        // 眩晕：加大字号，半透明背景
        QRectF rect = boundingRect();
        painter->setBrush(QColor(0, 0, 0, 160));
        painter->setPen(Qt::NoPen);
        painter->drawRect(rect);
        painter->setPen(Qt::yellow);
        QFont font("Arial", 16, QFont::Bold);
        painter->setFont(font);
        painter->drawText(rect, Qt::AlignCenter, "眩晕");
    }
}

void Hunter::updateCharacter()
{
    if (m_attackEffectTimer > 0) { m_attackEffectTimer--; update(); }

    if (m_stunned) {
        if (m_stunTimer > 0) m_stunTimer--;
        if (m_stunTimer <= 0) { m_stunned = false; m_canMove = true; }
        else return;
    }

    if (m_attackCooldownTimer > 0) m_attackCooldownTimer--;

    if (m_destroying) { updateDestroyProgress(); return; }

    if (m_hasTarget && m_canMove && m_enabled) {
        QPointF dir = m_targetPos - pos();
        qreal len = QLineF(pos(), m_targetPos).length();
        if (len < 5.0) {
            clearTarget();
        } else {
            QPointF step = dir / len * currentSpeed();
            QPointF newPos = pos() + step;
            int halfW = DEFAULT_WIDTH / 2;
            int halfH = DEFAULT_HEIGHT / 2;
            if (newPos.x() < halfW) newPos.setX(halfW);
            if (newPos.x() > GameConfig::MAP_WIDTH - halfW) newPos.setX(GameConfig::MAP_WIDTH - halfW);
            if (newPos.y() < halfH) newPos.setY(halfH);
            if (newPos.y() > GameConfig::MAP_HEIGHT - halfH) newPos.setY(GameConfig::MAP_HEIGHT - halfH);
            setPos(newPos);

            if (qAbs(dir.x()) > qAbs(dir.y()))
                setFacingDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
            else
                setFacingDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
        }
        setMoveDirection(Direction::None);
    }

    Character::updateCharacter();

    if (m_animator) {
        if (m_hasTarget && m_canMove) {
            m_animator->setDirection(m_facing);
            m_animator->startAnimation();
        } else {
            m_animator->stopAnimation();
        }
    }
}

void Hunter::attack()
{
    if (!m_enabled || m_stunned || m_destroying || m_attacking) return;
    if (!isAttackReady()) return;

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

    if (!m_scene) return;

    m_attackEffectTimer = 15;
    m_attackEffectPos = hunterPos;

    QList<QGraphicsItem*> items = m_scene->items();
    for (QGraphicsItem *item : items) {
        Survivor *survivor = dynamic_cast<Survivor*>(item);
        if (survivor && !survivor->isEliminated()) {
            QLineF line(hunterPos, survivor->pos());
            if (line.length() <= attackRadius) {
                qreal angleDiff = 0;
                if (isSurvivorInAttackRange(survivor, angleDiff) &&
                    !m_scene->lineIntersectsObstacle(hunterPos, survivor->pos())) {
                    survivor->takeDamage();
                    // 攻击命中后监管者硬直
                    stun(GameConfig::HUNTER_STUN_AFTER_ATTACK);   // 使用新增常量

                    QList<Bush*> bushes = m_scene->getBushesInRadius(hunterPos, attackRadius);
                    for (Bush *bush : bushes) {
                        if (bush->contains(bush->mapFromScene(survivor->pos()))) {
                            survivor->revealPosition(GameConfig::FRAMES_BUSH_REVEAL);
                        }
                    }
                    return;
                }
            }
        }

        MechanicalPuppet *puppet = dynamic_cast<MechanicalPuppet*>(item);
        if (puppet && !puppet->m_destroyed) {
            QLineF line(hunterPos, puppet->pos());
            if (line.length() <= attackRadius &&
                !m_scene->lineIntersectsObstacle(hunterPos, puppet->pos())) {
                puppet->destroy();
                return;
            }
        }
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
    Q_UNUSED(obstacle);
    return;
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
    qreal dist = QLineF(pos(), m_currentObstacle->sceneBoundingRect().center()).length();
    if (dist > GameConfig::INTERACT_DESTROY_DIST * 1.5) {
        stopDestroying();
        return;
    }

    if (m_destroyTimer > 0) m_destroyTimer--;
    if (m_destroyTimer <= 0) {
        if (m_scene) {
            m_scene->removeItem(m_currentObstacle);
            delete m_currentObstacle;
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