#include "Survivor.h"
#include "CipherMachine.h"
#include "Gate.h"
#include "GameScene.h"
#include "utils/TimerCounter.h"
#include <QPainter>
#include <QDebug>

Survivor::Survivor(SurvivorType type, QGraphicsItem *parent)
    : Character(parent)
    , m_type(type)
    , m_health(GameConfig::SURVIVOR_MAX_HEALTH)
    , m_eliminated(false)
    , m_burning(false)
    , m_decoding(false)
    , m_openingGate(false)
    , m_rescuing(false)
    , m_hasBeenRescued(false)
    , m_inBush(false)
    , m_currentCipher(nullptr)
    , m_currentGate(nullptr)
    , m_rescueTarget(nullptr)
    , m_burnTimer(0)
    , m_rescueTimer(0)
    , m_revealTimer(0)
{
    setSpeedMultiplier(1.0);
}

qreal Survivor::baseSpeed() const
{
    qreal base = GameConfig::SURVIVOR_BASE_SPEED;
    if (isHurt()) base *= GameConfig::HURT_SPEED_RATIO;
    return base;
}

qreal Survivor::decodeSpeedMultiplier() const
{
    return isHurt() ? GameConfig::HURT_DECODE_RATIO : 1.0;
}

void Survivor::takeDamage()
{
    if (m_eliminated) return;
    if (m_health > 0) {
        m_health--;
        emit healthChanged(m_health);
        // 中断当前交互
        if (m_decoding) stopDecoding();
        if (m_openingGate) stopOpeningGate();
        if (m_rescuing) stopRescuing();

        if (m_health == 0) {
            startBurning();
        } else {
            setSpeedMultiplier(GameConfig::HURT_SPEED_RATIO);
        }
    }
}

void Survivor::heal()
{
    if (m_eliminated) return;
    if (m_health == 1) {
        m_health = 2;
        setSpeedMultiplier(1.0);
        emit healthChanged(m_health);
    }
}

void Survivor::startBurning()
{
    if (m_burning || m_eliminated) return;
    m_burning = true;
    m_burnTimer = GameConfig::FRAMES_BURN;
    m_canMove = false;
    m_decoding = false;
    m_openingGate = false;
    m_rescuing = false;
}

void Survivor::checkBurnTimeout()
{
    if (!m_burning) return;
    if (m_burnTimer > 0) {
        m_burnTimer--;
    }
    if (m_burnTimer <= 0 && !m_eliminated) {
        m_eliminated = true;
        m_burning = false;
        m_canMove = false;
        emit eliminated();
    }
}

void Survivor::startRescuing(Survivor *target)
{
    if (!target || !target->isBurning() || m_rescuing) return;
    m_rescueTarget = target;
    m_rescuing = true;
    m_canMove = false;
    m_rescueTimer = GameConfig::FRAMES_RESCUE;
    revealPosition(GameConfig::FRAMES_RESCUE);
}

void Survivor::stopRescuing()
{
    m_rescuing = false;
    m_canMove = true;
    m_rescueTarget = nullptr;
    m_rescueTimer = 0;
}

void Survivor::updateRescueProgress()
{
    if (!m_rescuing || !m_rescueTarget) return;
    if (m_rescueTimer > 0) m_rescueTimer--;
    if (m_rescueTimer <= 0) {
        completeRescue();
    }
}

void Survivor::completeRescue()
{
    if (!m_rescueTarget || m_rescueTarget->isEliminated() || !m_rescueTarget->isBurning()) {
        stopRescuing();
        return;
    }
    m_rescueTarget->beingRescued();
    stopRescuing();
}

void Survivor::beingRescued()
{
    if (!m_burning || m_eliminated) return;
    m_burning = false;
    m_burnTimer = 0;
    m_health = 1;
    m_canMove = true;
    m_hasBeenRescued = true;
    setSpeedMultiplier(GameConfig::HURT_SPEED_RATIO);
    emit healthChanged(m_health);
}

void Survivor::startDecoding(CipherMachine *cipher)
{
    if (!cipher || m_decoding || m_eliminated || m_burning) return;
    m_currentCipher = cipher;
    m_decoding = true;
    m_canMove = false;
    cipher->addDecoder(this);
}

void Survivor::stopDecoding()
{
    if (m_currentCipher) {
        m_currentCipher->removeDecoder(this);
    }
    m_decoding = false;
    m_canMove = true;
    m_currentCipher = nullptr;
}

void Survivor::startOpeningGate(Gate *gate)
{
    if (!gate || m_openingGate || m_eliminated || m_burning) return;
    m_currentGate = gate;
    m_openingGate = true;
    m_canMove = false;
    gate->addOpener(this);
}

void Survivor::stopOpeningGate()
{
    if (m_currentGate) {
        m_currentGate->removeOpener(this);
    }
    m_openingGate = false;
    m_canMove = true;
    m_currentGate = nullptr;
}

void Survivor::escape()
{
    if (m_eliminated) return;
    m_eliminated = true;
    m_canMove = false;
    m_decoding = false;
    m_openingGate = false;
    m_rescuing = false;
    m_burning = false;
    emit escaped();
}

void Survivor::setInBush(bool inBush)
{
    m_inBush = inBush;
    update();
}

void Survivor::revealPosition(int durationFrames)
{
    m_revealTimer = durationFrames;
}

void Survivor::updateCharacter()
{
    if (m_burning) checkBurnTimeout();
    if (m_rescuing) updateRescueProgress();
    if (m_revealTimer > 0) m_revealTimer--;
    if (!m_enabled) return;

    Character::updateCharacter();

    // 检查交互距离
    if (m_decoding && m_currentCipher) {
        qreal dist = QLineF(pos(), m_currentCipher->pos()).length();
        if (dist > GameConfig::INTERACT_CIPHER_DIST) stopDecoding();
    }
    if (m_openingGate && m_currentGate) {
        qreal dist = QLineF(pos(), m_currentGate->pos()).length();
        if (dist > GameConfig::INTERACT_GATE_DIST) stopOpeningGate();
    }
}

void Survivor::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor color;
    if (m_eliminated) {
        color = Qt::darkGray;
    } else if (m_burning) {
        color = Qt::red;
    } else if (m_health == 1) {
        color = Qt::yellow;
    } else {
        switch (m_type) {
        case SurvivorType::Doctor:   color = Qt::white; break;
        case SurvivorType::Mechanic: color = QColor(200, 150, 50); break;
        case SurvivorType::AirForce: color = QColor(100, 150, 200); break;
        default: color = Qt::gray;
        }
    }
    if (m_inBush && !m_eliminated) color.setAlpha(128);

    painter->setBrush(color);
    painter->setPen(Qt::black);
    painter->drawRect(boundingRect());

    if (m_decoding) painter->drawText(boundingRect(), Qt::AlignCenter, "译");
    else if (m_rescuing) painter->drawText(boundingRect(), Qt::AlignCenter, "救");
    else if (m_burning) painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_burnTimer / 60 + 1) + "s");

    if (m_revealTimer > 0) {
        painter->setPen(QPen(Qt::red, 3));
        painter->drawRect(boundingRect().adjusted(-2, -2, 2, 2));
    }
}