#include "Survivor.h"
#include "CipherMachine.h"
#include "Gate.h"
#include "GameScene.h"
#include "utils/CharacterAnimator.h"
#include <QPainter>

Survivor::Survivor(SurvivorType type, QGraphicsItem *parent)
    : Character(parent), m_type(type),
    m_health(GameConfig::SURVIVOR_MAX_HEALTH),
    m_eliminated(false), m_burning(false),
    m_decoding(false), m_openingGate(false), m_rescuing(false),
    m_hasBeenRescued(false), m_inBush(false),
    m_burnTimer(0), m_rescueTimer(0), m_revealTimer(0),
    m_downCount(0), m_selfReviveTimer(0), m_canSelfRevive(true)
{
    setSpeedMultiplier(1.0);

    m_animator = new CharacterAnimator(this);
    m_animator->setFrameInterval(80);

    // 图片加载（同之前）
    QString prefix;
    switch (m_type) {
    case SurvivorType::Doctor:   prefix = ":/new/prefix2/images/yi"; break;
    case SurvivorType::Mechanic: prefix = ":/new/prefix2/images/ji"; break;
    case SurvivorType::AirForce: prefix = ":/new/prefix2/images/kon"; break;
    default: prefix = ":/new/prefix2/images/yi"; break;
    }
    auto load = [&](const QString &suffix) {
        QPixmap pm(prefix + suffix);
        return pm.scaled(DEFAULT_WIDTH, DEFAULT_HEIGHT, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    };
    m_animator->setDownPixmap(load("z.png"));
    m_animator->setUpPixmap(load("hou.png"));
    m_animator->setLeftPixmap(load("zc.png"));
    m_animator->setRightPixmap(load("yc.png"));
}

qreal Survivor::baseSpeed() const {
    qreal b = GameConfig::SURVIVOR_BASE_SPEED;
    if (isHurt()) b *= GameConfig::HURT_SPEED_RATIO;
    return b;
}
qreal Survivor::decodeSpeedMultiplier() const {
    return isHurt() ? GameConfig::HURT_DECODE_RATIO : 1.0;
}

void Survivor::takeDamage() {
    if (m_eliminated) return;
    if (m_health > 0) {
        m_health--;
        emit healthChanged(m_health);
        if (m_decoding) stopDecoding();
        if (m_openingGate) stopOpeningGate();
        if (m_rescuing) stopRescuing();
        if (m_healing) stopHealing();          // 受伤中断治疗
        if (m_health == 0) startBurning();
        else setSpeedMultiplier(GameConfig::HURT_SPEED_RATIO);
    }
}
void Survivor::heal() {
    if (m_eliminated || m_health != 1) return;
    m_health = 2;
    setSpeedMultiplier(1.0);
    emit healthChanged(m_health);
}

void Survivor::startBurning() {
    if (m_burning || m_eliminated) return;
    m_burning = true;
    m_burnTimer = GameConfig::FRAMES_BURN;
    m_canMove = false;
    m_decoding = m_openingGate = m_rescuing = false;
    m_healing = false;                         // 倒地中断治疗
    m_downCount++;
    m_canSelfRevive = false;
}

void Survivor::checkBurnTimeout() {
    if (!m_burning) return;
    if (m_burnTimer > 0) m_burnTimer--;
    if (m_burnTimer <= 0 && !m_eliminated) {
        m_eliminated = true;
        m_burning = false;
        m_canMove = false;
        m_showEliminatedText = 60;
        update();
        emit eliminated();
        return;
    }
}
void Survivor::selfRevive() { /* 空 */ }

// ---------- 救助（倒地） ----------
void Survivor::startRescuing(Survivor *target) {
    if (!target || !target->isBurning() || m_rescuing) return;
    m_rescueTarget = target;
    m_rescuing = true;
    m_canMove = false;
    m_rescueTimer = GameConfig::FRAMES_RESCUE;
    revealPosition(GameConfig::FRAMES_RESCUE);
}
void Survivor::stopRescuing() { m_rescuing = false; m_canMove = true; m_rescueTarget = nullptr; m_rescueTimer = 0; }
void Survivor::updateRescueProgress() {
    if (!m_rescuing || !m_rescueTarget) return;
    if (m_rescueTimer > 0) m_rescueTimer--;
    if (m_rescueTimer <= 0) completeRescue();
}
void Survivor::completeRescue() {
    if (!m_rescueTarget || m_rescueTarget->isEliminated() || !m_rescueTarget->isBurning()) {
        stopRescuing(); return;
    }
    m_rescueTarget->beingRescued();
    stopRescuing();
}
void Survivor::beingRescued() {
    if (!m_burning || m_eliminated) return;
    m_burning = false;
    m_burnTimer = 0;
    m_health = 1;
    m_canMove = true;
    m_hasBeenRescued = true;
    m_canSelfRevive = false;
    setSpeedMultiplier(GameConfig::HURT_SPEED_RATIO);
    emit healthChanged(m_health);
}

// ---------- 治疗（轻伤） ----------
void Survivor::startHealing(Survivor *target) {
    if (!target || !target->isHurt() || m_healing) return;
    m_healTarget = target;
    m_healing = true;
    m_canMove = false;
    m_healTimer = GameConfig::FRAMES_DOCTOR_HEAL;   // 用医生的治疗时间（2秒）
    revealPosition(GameConfig::FRAMES_DOCTOR_HEAL);
}
void Survivor::stopHealing() {
    m_healing = false;
    m_canMove = true;
    m_healTarget = nullptr;
    m_healTimer = 0;
}
void Survivor::updateHealProgress() {
    if (!m_healing || !m_healTarget) return;
    if (m_healTimer > 0) m_healTimer--;
    if (m_healTimer <= 0) completeHeal();
}
void Survivor::completeHeal() {
    if (!m_healTarget || m_healTarget->isEliminated() || !m_healTarget->isHurt()) {
        stopHealing(); return;
    }
    m_healTarget->heal();
    stopHealing();
}

// ---------- 破译/开门/逃脱 ----------
void Survivor::startDecoding(CipherMachine *cipher) {
    if (!cipher || m_decoding || m_eliminated || m_burning) return;
    m_currentCipher = cipher;
    m_decoding = true;
    m_canMove = false;
    cipher->addDecoder(this);
}
void Survivor::stopDecoding() {
    if (m_currentCipher) m_currentCipher->removeDecoder(this);
    m_decoding = false; m_canMove = true; m_currentCipher = nullptr;
}
void Survivor::startOpeningGate(Gate *gate) {
    if (!gate || m_openingGate || m_eliminated || m_burning) return;
    m_currentGate = gate;
    m_openingGate = true;
    m_canMove = false;
    gate->addOpener(this);
}
void Survivor::stopOpeningGate() {
    if (m_currentGate) m_currentGate->removeOpener(this);
    m_openingGate = false; m_canMove = true; m_currentGate = nullptr;
}
void Survivor::escape() {
    if (m_eliminated) return;
    m_escaped = true;
    m_showEscapedText = 60;
    m_eliminated = true;
    m_canMove = false;
    m_decoding = m_openingGate = m_rescuing = false;
    m_healing = false;
    m_burning = false;
    update();
    emit escaped();
}

void Survivor::updateCharacter() {
    if (m_escaped && m_showEscapedText > 0) {
        m_showEscapedText--;
        update();
        if (m_showEscapedText <= 0 && scene()) scene()->removeItem(this);
        return;
    }
    if (m_eliminated && m_showEliminatedText > 0) {
        m_showEliminatedText--;
        update();
        if (m_showEliminatedText <= 0 && scene()) scene()->removeItem(this);
        return;
    }
    if (m_burning) { checkBurnTimeout(); return; }
    if (m_rescuing) updateRescueProgress();
    if (m_healing) updateHealProgress();    // 治疗进度更新
    if (m_revealTimer > 0) m_revealTimer--;
    if (!m_enabled) return;

    Character::updateCharacter();

    if (m_decoding && m_currentCipher && QLineF(pos(), m_currentCipher->pos()).length() > GameConfig::INTERACT_CIPHER_DIST)
        stopDecoding();
    if (m_openingGate && m_currentGate && QLineF(pos(), m_currentGate->pos()).length() > GameConfig::INTERACT_GATE_DIST)
        stopOpeningGate();
    if (m_healing && m_healTarget && QLineF(pos(), m_healTarget->pos()).length() > GameConfig::INTERACT_RESCUE_DIST)
        stopHealing();
}

void Survivor::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    if (m_escaped) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 200, 0, 200));
        painter->drawRect(boundingRect());
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 12, QFont::Bold));
        painter->drawText(boundingRect(), Qt::AlignCenter, "逃脱");
        return;
    }
    if (m_eliminated && m_showEliminatedText > 0) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(50, 50, 50, 200));
        painter->drawRect(boundingRect());
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 12, QFont::Bold));
        painter->drawText(boundingRect(), Qt::AlignCenter, "淘汰");
        return;
    }
    if (m_inBush && !m_eliminated) painter->setOpacity(0.5);
    if (m_animator) {
        QPixmap cur = m_animator->currentPixmap();
        if (!cur.isNull()) {
            painter->drawPixmap(boundingRect().toRect(), cur);
        } else {
            QColor col = Qt::gray;
            if (m_eliminated) col = Qt::darkGray;
            else if (m_burning) col = Qt::red;
            else if (m_health == 1) col = Qt::yellow;
            painter->setBrush(col);
            painter->setPen(Qt::black);
            painter->drawRect(boundingRect());
        }
    } else {
        painter->setBrush(Qt::gray);
        painter->setPen(Qt::black);
        painter->drawRect(boundingRect());
    }
    // 状态文字
    if (m_decoding) {
        painter->setPen(Qt::white);
        painter->drawText(boundingRect(), Qt::AlignCenter, "译");
    } else if (m_rescuing) {
        painter->setPen(Qt::white);
        painter->drawText(boundingRect(), Qt::AlignCenter, "救");
    } else if (m_healing) {
        painter->setPen(Qt::green);            // 治疗用绿色
        painter->drawText(boundingRect(), Qt::AlignCenter, "疗");
    } else if (m_burning) {
        painter->setPen(Qt::white);
        painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_burnTimer/60+1) + "s");
    }
    if (m_revealTimer > 0) {
        painter->setPen(QPen(Qt::red, 3));
        painter->drawRect(boundingRect().adjusted(-2,-2,2,2));
    }
}

void Survivor::setInBush(bool inBush) { m_inBush = inBush; update(); }
void Survivor::revealPosition(int durationFrames) { m_revealTimer = durationFrames; }