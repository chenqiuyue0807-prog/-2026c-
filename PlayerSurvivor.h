#ifndef PLAYERSURVIVOR_H
#define PLAYERSURVIVOR_H

#include "Survivor.h"

class GameEngine;
class Hunter;

class PlayerSurvivor : public Survivor
{
    Q_OBJECT

public:
    explicit PlayerSurvivor(SurvivorType type, QGraphicsItem *parent = nullptr);

    void setGameEngine(GameEngine *engine) { m_gameEngine = engine; }

    // 键盘事件
    void handleKeyPress(QKeyEvent *event) override;
    void handleKeyRelease(QKeyEvent *event) override;

    // 每帧更新
    void updateCharacter() override;

    // 技能冷却比例
    qreal skillCooldownRatio() const;
    bool isSkillReady() const { return m_skillCooldownTimer <= 0; }

    // 技能释放
    void useSkill();

signals:
    void skillUsed();
    void interactionPrompt(const QString &text);

private:
    // 输入状态
    bool m_keyW, m_keyS, m_keyA, m_keyD;
    bool m_keySpacePressed;
    bool m_keySpaceWasPressed;
    bool m_keyF;

    // 技能冷却
    int m_skillCooldownTimer;

    // 引擎引用（获取监管者、密码机等）
    GameEngine *m_gameEngine;

    // 移动方向更新
    void updateMoveDirection();

    // 交互检测
    void checkInteraction();
    enum InteractionType { None, Cipher, Rescue, Gate };
    InteractionType getNearestInteraction(qreal &distance, void **targetPtr);

    // 各角色技能实现
    void useDoctorSkill();
    void useMechanicSkill();
    void useAirForceSkill();
};

#endif // PLAYERSURVIVOR_H