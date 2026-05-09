#ifndef SURVIVOR_H
#define SURVIVOR_H

#include "Character.h"
#include "GameConfig.h"

class CharacterAnimator; // 前向声明
class CipherMachine;
class Gate;

class Survivor : public Character
{
    Q_OBJECT
public:
    explicit Survivor(SurvivorType type, QGraphicsItem *parent = nullptr);

    SurvivorType survivortype() const { return m_type; }
    int health() const { return m_health; }
    bool isHurt() const { return m_health == 1; }
    bool isBurning() const { return m_burning; }
    bool isEliminated() const { return m_eliminated; }

    bool hasBeenRescued() const { return m_hasBeenRescued; }
    void setRescued(bool rescued) { m_hasBeenRescued = rescued; }

    bool isDecoding() const { return m_decoding; }
    bool isOpeningGate() const { return m_openingGate; }
    bool isRescuing() const { return m_rescuing; }
    bool isHealing() const { return m_healing; }          // 是否正在治疗队友

    CipherMachine* currentCipher() const { return m_currentCipher; }
    Gate* currentGate() const { return m_currentGate; }
    Survivor* rescueTarget() const { return m_rescueTarget; }
    Survivor* healTarget() const { return m_healTarget; } // 治疗目标

    qreal decodeSpeedMultiplier() const;

    virtual void takeDamage();
    virtual void heal();
    virtual void startBurning();
    virtual void startRescuing(Survivor *target);
    virtual void stopRescuing();
    virtual void completeRescue();
    virtual void beingRescued();
    virtual void startDecoding(CipherMachine *cipher);
    virtual void stopDecoding();
    virtual void startOpeningGate(Gate *gate);
    virtual void stopOpeningGate();
    virtual void escape();

    // 治疗交互（求生者之间治疗）
    virtual void startHealing(Survivor *target);
    virtual void stopHealing();
    virtual void completeHeal();

    void setInBush(bool inBush);
    bool isInBush() const { return m_inBush; }
    void revealPosition(int durationFrames);

    void updateCharacter() override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    qreal baseSpeed() const override;

signals:
    void healthChanged(int health);
    void eliminated();
    void escaped();

protected:
    bool m_escaped = false;
    int m_showEscapedText = 0;
    int m_showEliminatedText = 0;

    CharacterAnimator* m_animator = nullptr;
    SurvivorType m_type;
    int m_health;
    bool m_eliminated;
    bool m_burning;
    bool m_decoding;
    bool m_openingGate;
    bool m_rescuing;
    bool m_hasBeenRescued;
    bool m_inBush;
    bool m_healing = false;                // 正在治疗队友

    CipherMachine *m_currentCipher = nullptr;
    Gate *m_currentGate = nullptr;
    Survivor *m_rescueTarget = nullptr;
    Survivor *m_healTarget = nullptr;       // 治疗的目标

    int m_burnTimer;
    int m_rescueTimer;
    int m_revealTimer;
    int m_healTimer = 0;                   // 治疗进度计时器（帧）
    int m_downCount = 0;
    int m_selfReviveTimer = 0;
    bool m_canSelfRevive = true;

    void checkBurnTimeout();
    void updateRescueProgress();
    void updateHealProgress();             // 更新治疗进度
    void selfRevive();
};

#endif // SURVIVOR_H