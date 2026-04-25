#include "PlayerSurvivor.h"
#include "GameEngine.h"
#include "GameScene.h"
#include "entities/Hunter.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include "entities/AISurvivor.h"
#include "GameConfig.h"
#include <QLineF>
#include <QDebug>
#include <QtMath>
#include "entities/MechanicalPuppet.h"

PlayerSurvivor::PlayerSurvivor(SurvivorType type, QGraphicsItem *parent)
    : Survivor(type, parent)
    , m_keyW(false), m_keyS(false), m_keyA(false), m_keyD(false)
    , m_keySpacePressed(false), m_keySpaceWasPressed(false)
    , m_keyF(false)
    , m_skillCooldownTimer(0)
    , m_gameEngine(nullptr)
{
}

void PlayerSurvivor::handleKeyPress(QKeyEvent *event)
{
    if (!m_enabled || m_eliminated) return;
    switch (event->key()) {
    case Qt::Key_W: m_keyW = true; break;
    case Qt::Key_S: m_keyS = true; break;
    case Qt::Key_A: m_keyA = true; break;
    case Qt::Key_D: m_keyD = true; break;
    case Qt::Key_Space:
        if (!event->isAutoRepeat()) {
            m_keySpacePressed = true;
            m_keySpaceWasPressed = true;
        }
        break;
    case Qt::Key_F:
        if (!event->isAutoRepeat() && isSkillReady() && m_canMove && !m_decoding && !m_rescuing && !m_openingGate) {
            useSkill();
        }
        break;
    default: break;
    }
    updateMoveDirection();
}

void PlayerSurvivor::useMechanicSkill()
{
    if (!m_scene) return;
    MechanicalPuppet *puppet = new MechanicalPuppet(pos(), m_scene);
    // 同样寻找最近的未完成密码机
    CipherMachine *target = nullptr;
    // 从引擎或场景获取密码机列表（此处简化，可以调用 m_gameEngine->getCiphers() 但类型可能不匹配）
    // 建议在 GameEngine 中提供获取密码机列表的方法，或者从场景遍历
    QList<QGraphicsItem*> items = m_scene->items();
    for (auto *item : items) {
        if (auto *cipher = dynamic_cast<CipherMachine*>(item)) {
            if (!cipher->isCompleted()) {
                target = cipher;
                break;
            }
        }
    }
    if (target) puppet->moveToTarget(target->pos());
}

void PlayerSurvivor::handleKeyRelease(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_W: m_keyW = false; break;
    case Qt::Key_S: m_keyS = false; break;
    case Qt::Key_A: m_keyA = false; break;
    case Qt::Key_D: m_keyD = false; break;
    case Qt::Key_Space:
        if (!event->isAutoRepeat()) {
            m_keySpacePressed = false;
            if (m_decoding) stopDecoding();
            if (m_openingGate) stopOpeningGate();
        }
        break;
    default: break;
    }
    updateMoveDirection();
}

void PlayerSurvivor::updateMoveDirection()
{
    if (!m_canMove) {
        setMoveDirection(Direction::None);
        return;
    }
    if (m_keyW && !m_keyS) setMoveDirection(Direction::Up);
    else if (m_keyS && !m_keyW) setMoveDirection(Direction::Down);
    else if (m_keyA && !m_keyD) setMoveDirection(Direction::Left);
    else if (m_keyD && !m_keyA) setMoveDirection(Direction::Right);
    else setMoveDirection(Direction::None);
}

void PlayerSurvivor::updateCharacter()
{
    // 技能冷却
    if (m_skillCooldownTimer > 0) m_skillCooldownTimer--;

    // 空格交互检测
    if (m_enabled && !m_eliminated && m_canMove && !m_decoding && !m_rescuing && !m_openingGate) {
        if (m_keySpacePressed && !m_keySpaceWasPressed) {
            checkInteraction();
        }
    }
    m_keySpaceWasPressed = m_keySpacePressed;

    // 基类更新
    Survivor::updateCharacter();
}

void PlayerSurvivor::checkInteraction()
{
    qreal dist;
    void *targetPtr = nullptr;
    InteractionType type = getNearestInteraction(dist, &targetPtr);
    if (type == None || dist > GameConfig::INTERACT_CIPHER_DIST) return;

    switch (type) {
    case Cipher: {
        CipherMachine *cipher = static_cast<CipherMachine*>(targetPtr);
        if (cipher && !cipher->isCompleted() && cipher->currentDecoders() < GameConfig::MAX_SURVIVORS_PER_CIPHER) {
            startDecoding(cipher);
            emit interactionPrompt("破译中...");
        }
        break;
    }
    case Rescue: {
        Survivor *target = static_cast<Survivor*>(targetPtr);
        if (target && target->isBurning() && !target->isEliminated()) {
            startRescuing(target);
            emit interactionPrompt("救助中...");
        }
        break;
    }
    case Gate: {
       class Gate *gate = static_cast<class Gate*>(targetPtr);
        if (gate && gate->isUnlocked() && !gate->isFullyOpen()) {
            startOpeningGate(gate);
            emit interactionPrompt("开启大门...");
        } else if (gate && gate->isFullyOpen()) {
            escape(); // 大门已开，直接逃脱
        }
        break;
    }
    default: break;
    }
}

PlayerSurvivor::InteractionType PlayerSurvivor::getNearestInteraction(qreal &distance, void **targetPtr)
{
    InteractionType nearest = None;
    qreal minDist = 99999;
    void *best = nullptr;

    if (!m_gameEngine || !m_scene) return None;

    // 密码机
    for (CipherMachine *c : m_gameEngine->getCiphers()) {
        if (c->isCompleted()) continue;
        qreal d = QLineF(pos(), c->pos()).length();
        if (d < minDist) { minDist = d; nearest = Cipher; best = c; }
    }
    // 燃烧队友
    QList<AISurvivor*> aiList = m_gameEngine->getAISurvivors();
    PlayerSurvivor *player = m_gameEngine->getPlayer();
    auto checkSurvivor = [&](Survivor *s) {
        if (!s || s == this || s->isEliminated() || !s->isBurning()) return;
        qreal d = QLineF(pos(), s->pos()).length();
        if (d < minDist) { minDist = d; nearest = Rescue; best = s; }
    };
    if (player && player != this) checkSurvivor(player);
    for (AISurvivor *ai : aiList) checkSurvivor(ai);

    // 大门
   for (class Gate *g : m_gameEngine->getGates()) {
        if (!g->isUnlocked()) continue;
        qreal d = QLineF(pos(), g->pos()).length();
        if (d < minDist) { minDist = d; nearest = Gate; best = g; }
    }

    distance = minDist;
    if (targetPtr) *targetPtr = best;
    return nearest;
}

void PlayerSurvivor::useSkill()
{
    if (!isSkillReady()) return;

    switch (m_type) {
    case SurvivorType::Doctor:   useDoctorSkill(); break;
    case SurvivorType::Mechanic: useMechanicSkill(); break;
    case SurvivorType::AirForce: useAirForceSkill(); break;
    default: return;
    }

    // 设置冷却
    switch (m_type) {
    case SurvivorType::Doctor:   m_skillCooldownTimer = GameConfig::FRAMES_DOCTOR_COOLDOWN; break;
    case SurvivorType::Mechanic: m_skillCooldownTimer = GameConfig::FRAMES_MECHANIC_COOLDOWN; break;
    case SurvivorType::AirForce: m_skillCooldownTimer = GameConfig::FRAMES_AIRFORCE_COOLDOWN; break;
    default: break;
    }
    emit skillUsed();
}

void PlayerSurvivor::useDoctorSkill()
{
    Survivor *target = nullptr;
    qreal minDist = 30.0;
    if (isHurt()) {
        target = this;
    } else {
        QList<QGraphicsItem*> items = m_scene->items();
        for (auto *item : items) {
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

// void PlayerSurvivor::useMechanicSkill()
// {
//     if (!m_scene) return;
//     // 简化傀儡：创建一个蓝色方形傀儡，15秒后自动删除，向最近密码机移动
//     QGraphicsRectItem *puppet = m_scene->addRect(QRectF(-10, -10, 20, 20), QPen(Qt::blue), QBrush(Qt::cyan));
//     puppet->setPos(pos());
//     QTimer::singleShot(GameConfig::FRAMES_PUPPET_LIFETIME * 1000 / 60, puppet, &QObject::deleteLater);
//     // 在真实项目中可扩展为机械傀儡AI
// }

void PlayerSurvivor::useAirForceSkill()
{
    if (!m_gameEngine) return;
    Hunter *hunter = m_gameEngine->getHunter();
    if (!hunter) return;

    qreal dist = QLineF(pos(), hunter->pos()).length();
    if (dist > 120) {
        emit interactionPrompt("未命中");
        return;
    }

    // 角度判断
    QPointF dir = hunter->pos() - pos();
    qreal angleTo = atan2(dir.y(), dir.x()) * 180.0 / M_PI;
    if (angleTo < 0) angleTo += 360;
    qreal facing = 0;
    switch (m_facing) {
    case Direction::Right: facing = 0; break;
    case Direction::Down:  facing = 90; break;
    case Direction::Left:  facing = 180; break;
    case Direction::Up:    facing = 270; break;
    default: return;
    }
    qreal diff = fabs(angleTo - facing);
    if (diff > 180) diff = 360 - diff;
    if (diff <= 30 && !m_scene->lineIntersectsObstacle(pos(), hunter->pos())) {
        hunter->stun(GameConfig::FRAMES_AIRFORCE_STUN);
        QPointF back = (pos() - hunter->pos()) * 0.2; // 后撤
        setPos(pos() + back);
        revealPosition(GameConfig::FRAMES_AIRFORCE_STUN);
        emit interactionPrompt("命中！监管者眩晕");
    } else {
        emit interactionPrompt("未命中");
    }
}

qreal PlayerSurvivor::skillCooldownRatio() const
{
    int maxCd = 0;
    switch (m_type) {
    case SurvivorType::Doctor:   maxCd = GameConfig::FRAMES_DOCTOR_COOLDOWN; break;
    case SurvivorType::Mechanic: maxCd = GameConfig::FRAMES_MECHANIC_COOLDOWN; break;
    case SurvivorType::AirForce: maxCd = GameConfig::FRAMES_AIRFORCE_COOLDOWN; break;
    default: return 0.0;
    }
    return maxCd ? 1.0 - (qreal)m_skillCooldownTimer / maxCd : 0.0;
}