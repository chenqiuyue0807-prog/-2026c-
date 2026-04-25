#ifndef SURVIVOR_H
#define SURVIVOR_H

#include "Character.h"
#include "GameConfig.h"

class CipherMachine;
class Gate;
class TimerCounter;

class Survivor : public Character
{
    Q_OBJECT

public:
    explicit Survivor(SurvivorType type, QGraphicsItem *parent = nullptr);

    SurvivorType survivortype() const { return m_type; }

    // 生命值
    int health() const { return m_health; }
    bool isHurt() const { return m_health == 1; }
    bool isBurning() const { return m_burning; }
    bool isEliminated() const { return m_eliminated; }

    // 救助标记
    bool hasBeenRescued() const { return m_hasBeenRescued; }
    void setRescued(bool rescued) { m_hasBeenRescued = rescued; }

    // 状态查询
    bool isDecoding() const { return m_decoding; }
    bool isOpeningGate() const { return m_openingGate; }
    bool isRescuing() const { return m_rescuing; }

    // 交互对象
    CipherMachine* currentCipher() const { return m_currentCipher; }
    Gate* currentGate() const { return m_currentGate; }
    Survivor* rescueTarget() const { return m_rescueTarget; }

    // 破译速度倍率（正常 1.0，受伤 0.85）
    qreal decodeSpeedMultiplier() const;

    // 受到攻击
    virtual void takeDamage();

    // 治疗（医生技能或救助完成）
    virtual void heal();

    // 燃烧
    virtual void startBurning();

    // 救助
    virtual void startRescuing(Survivor *target);
    virtual void stopRescuing();
    virtual void completeRescue();
    virtual void beingRescued();

    // 破译
    virtual void startDecoding(CipherMachine *cipher);
    virtual void stopDecoding();

    // 大门
    virtual void startOpeningGate(Gate *gate);
    virtual void stopOpeningGate();

    // 逃脱
    virtual void escape();

    // 草丛状态
    void setInBush(bool inBush);
    bool isInBush() const { return m_inBush; }

    // 暴露位置（破译失误等）
    void revealPosition(int durationFrames);

    // 每帧更新
    void updateCharacter() override;

    // 绘制
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    // 基类速度
    qreal baseSpeed() const override;

signals:
    void healthChanged(int health);
    void eliminated();
    void escaped();

protected:
    SurvivorType m_type;
    int m_health;               // 2=满血,1=受伤,0=倒地
    bool m_eliminated;
    bool m_burning;
    bool m_decoding;
    bool m_openingGate;
    bool m_rescuing;
    bool m_hasBeenRescued;
    bool m_inBush;

    CipherMachine *m_currentCipher;
    Gate *m_currentGate;
    Survivor *m_rescueTarget;

    // 计时器（帧计数）
   // TimerCounter *timer() const { return m_timerCounter; }
    int m_burnTimer;            // 燃烧剩余帧数
    int m_rescueTimer;          // 救助进度帧数
    int m_revealTimer;          // 暴露位置剩余帧数

    void checkBurnTimeout();
    void updateRescueProgress();
};

#endif // SURVIVOR_H