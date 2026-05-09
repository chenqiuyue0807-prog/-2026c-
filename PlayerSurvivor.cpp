#include "PlayerSurvivor.h"
#include "GameEngine.h"
#include "GameScene.h"
#include "entities/Hunter.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include "entities/AISurvivor.h"
#include "GameConfig.h"
#include <QLineF>
#include <QtMath>
#include "entities/MechanicalPuppet.h"

PlayerSurvivor::PlayerSurvivor(SurvivorType type, QGraphicsItem *parent)
    : Survivor(type, parent)
{
    // 使该 item 可以接收键盘焦点
    setFlag(QGraphicsItem::ItemIsFocusable);
    setFocus();
}

// ---------- 键盘按下 ----------
void PlayerSurvivor::handleKeyPress(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;    // 忽略持续按住产生的自动重复事件
    if (!m_enabled || m_eliminated) return;

    switch (event->key()) {
    case Qt::Key_W: m_keyW = true; break;
    case Qt::Key_S: m_keyS = true; break;
    case Qt::Key_A: m_keyA = true; break;
    case Qt::Key_D: m_keyD = true; break;
    case Qt::Key_Space:
        checkInteraction();               // 空格触发交互
        break;
    case Qt::Key_F:
        if (isSkillReady() && m_canMove && !m_decoding && !m_rescuing && !m_openingGate)
            useSkill();
        break;
    default: break;
    }
    updateMoveDirection();
}

// ---------- 键盘释放 ----------
void PlayerSurvivor::handleKeyRelease(QKeyEvent *event)
{
    if (event->isAutoRepeat()) return;
    if (!m_enabled || m_eliminated) return;

    switch (event->key()) {
    case Qt::Key_W: m_keyW = false; break;
    case Qt::Key_S: m_keyS = false; break;
    case Qt::Key_A: m_keyA = false; break;
    case Qt::Key_D: m_keyD = false; break;
    case Qt::Key_Space:
        // 松开空格时停止当前交互（破译/开门）
        if (m_decoding) {
            if (m_currentCipher) m_currentCipher->removeDecoder(this);
            stopDecoding();
        }
        if (m_openingGate) {
            if (m_currentGate) m_currentGate->removeOpener(this);
            stopOpeningGate();
        }
        break;
    default: break;
    }
    updateMoveDirection();
}

// 根据按键组合计算移动方向
void PlayerSurvivor::updateMoveDirection()
{
    if (!m_canMove) { setMoveDirection(Direction::None); return; }

    if (m_keyW && !m_keyS)      setMoveDirection(Direction::Up);
    else if (m_keyS && !m_keyW)  setMoveDirection(Direction::Down);
    else if (m_keyA && !m_keyD)  setMoveDirection(Direction::Left);
    else if (m_keyD && !m_keyA)  setMoveDirection(Direction::Right);
    else                         setMoveDirection(Direction::None);
}

// ---------- 每帧更新 ----------
void PlayerSurvivor::updateCharacter()
{
    // 冷却递减
    if (m_skillCooldownTimer > 0) --m_skillCooldownTimer;

    // 父类通用更新（移动、状态检测）
    Survivor::updateCharacter();

    // 动画控制
    if (m_animator) {
        if (m_direction != Direction::None && m_canMove) {
            m_animator->setDirection(m_direction);
            m_animator->startAnimation();
        } else {
            m_animator->stopAnimation();
        }
    }

    // 交互提示：仅在自由移动且不在任何交互中时，显示附近可交互对象的提示
    if (!m_decoding && !m_rescuing && !m_openingGate && !m_eliminated) {
        qreal minDist = 99999;
        CipherMachine *nearestCipher = nullptr;
        for (CipherMachine *c : m_gameEngine->getCiphers()) {
            if (!c->isCompleted()) {
                qreal d = QLineF(pos(), c->pos()).length();
                if (d < minDist) {
                    minDist = d;
                    nearestCipher = c;
                }
            }
        }

        bool shouldPrompt = (nearestCipher && minDist <= GameConfig::INTERACT_CIPHER_DIST);
        // 【性能优化】只在状态改变时发射信号，避免每帧重复发送导致 UI 高频刷新
        if (shouldPrompt != m_lastPromptActive) {
            m_lastPromptActive = shouldPrompt;
            if (shouldPrompt)
                emit interactionPrompt("按下空格破译");
            else
                emit interactionPrompt("");   // 清空提示
        }
    }
}

// ---------- 交互检查（空格键）----------
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
    case EscapeGate: {
        Gate *gate = static_cast<Gate*>(targetPtr);
        if (gate && gate->isUnlocked() && !gate->isFullyOpen()) {
            startOpeningGate(gate);
            emit interactionPrompt("开启大门...");
        } else if (gate && gate->isFullyOpen()) {
            escape();
        }
        break;
    }
    default: break;
    }
}

// 获取最近的交互对象
PlayerSurvivor::InteractionType PlayerSurvivor::getNearestInteraction(qreal &distance, void **targetPtr)
{
    InteractionType nearest = None;
    qreal minDist = 99999;
    void *best = nullptr;
    if (!m_gameEngine || !m_scene) return None;

    // 1. 最近可破译的密码机
    for (CipherMachine *c : m_gameEngine->getCiphers()) {
        if (c->isCompleted()) continue;
        qreal d = QLineF(pos(), c->pos()).length();
        if (d < minDist) { minDist = d; nearest = Cipher; best = c; }
    }

    // 2. 最近需要救助的求生者（包括玩家和其他 AI）
    QList<AISurvivor*> aiList = m_gameEngine->getAISurvivors();
    PlayerSurvivor *player = m_gameEngine->getPlayer();
    auto checkRescue = [&](Survivor *s){
        if (!s || s==this || s->isEliminated() || !s->isBurning()) return;
        qreal d = QLineF(pos(), s->pos()).length();
        if (d < minDist) { minDist = d; nearest = Rescue; best = s; }
    };
    if (player && player != this) checkRescue(player);
    for (AISurvivor *ai : aiList) checkRescue(ai);

    // 3. 最近的解锁大门
    for (Gate *g : m_gameEngine->getGates()) {
        if (!g->isUnlocked()) continue;
        qreal d = QLineF(pos(), g->pos()).length();
        if (d < minDist) { minDist = d; nearest = EscapeGate; best = g; }
    }

    distance = minDist;
    if (targetPtr) *targetPtr = best;
    return nearest;
}

// ---------- 技能释放 ----------
void PlayerSurvivor::useSkill()
{
    if (!isSkillReady()) return;

    switch (m_type) {
    case SurvivorType::Doctor:   useDoctorSkill(); break;
    case SurvivorType::Mechanic: useMechanicSkill(); break;
    case SurvivorType::AirForce: useAirForceSkill(); break;
    default: return;
    }

    // 根据角色类型设置冷却时间
    switch (m_type) {
    case SurvivorType::Doctor:   m_skillCooldownTimer = GameConfig::FRAMES_DOCTOR_COOLDOWN; break;
    case SurvivorType::Mechanic: m_skillCooldownTimer = GameConfig::FRAMES_MECHANIC_COOLDOWN; break;
    case SurvivorType::AirForce: m_skillCooldownTimer = GameConfig::FRAMES_AIRFORCE_COOLDOWN; break;
    default: break;
    }
    emit skillUsed();
}

// 医生技能：治疗自己或附近受伤队友
void PlayerSurvivor::useDoctorSkill()
{
    Survivor *target = nullptr;
    qreal minDist = 30.0;
    if (isHurt()) target = this;
    else {
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
        revealPosition(GameConfig::FRAMES_DOCTOR_HEAL); // 治疗时暴露位置
    }
}

// 机械师技能：召唤傀儡去破译最近密码机
void PlayerSurvivor::useMechanicSkill()
{
    if (!m_scene) return;
    MechanicalPuppet *puppet = new MechanicalPuppet(pos(), m_scene);
    CipherMachine *target = nullptr;
    qreal minDist = 1e9;
    QList<QGraphicsItem*> items = m_scene->items();
    for (auto *item : items) {
        if (auto *cipher = dynamic_cast<CipherMachine*>(item)) {
            if (!cipher->isCompleted()) {
                qreal d = QLineF(pos(), cipher->pos()).length();
                if (d < minDist) { minDist = d; target = cipher; }
            }
        }
    }
    if (target) puppet->moveToTarget(target->pos());
}

// 空军技能：近距离眩晕监管者（需要朝向角度和视野无阻挡）
void PlayerSurvivor::useAirForceSkill()
{
    if (!m_gameEngine) return;
    Hunter *hunter = m_gameEngine->getHunter();
    if (!hunter) return;
    qreal dist = QLineF(pos(), hunter->pos()).length();
    if (dist > 120) { emit interactionPrompt("未命中"); return; }

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
        QPointF back = (pos() - hunter->pos()) * 0.2; // 轻微击退
        setPos(pos() + back);
        revealPosition(GameConfig::FRAMES_AIRFORCE_STUN);
        emit interactionPrompt("命中！监管者眩晕");
    } else {
        emit interactionPrompt("未命中");
    }
}

// 技能冷却比例（0~1），用于 HUD 进度条
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