#ifndef PLAYERSURVIVOR_H
#define PLAYERSURVIVOR_H

#include "Survivor.h"
#include <QKeyEvent>

class GameEngine;
class Hunter;

class PlayerSurvivor : public Survivor
{
    Q_OBJECT
public:
    explicit PlayerSurvivor(SurvivorType type, QGraphicsItem *parent = nullptr);

    void setGameEngine(GameEngine *engine) { m_gameEngine = engine; }

    // 键盘事件处理（重写自 Character）
    void handleKeyPress(QKeyEvent *event) override;
    void handleKeyRelease(QKeyEvent *event) override;

    // 每帧更新（重写自 Survivor）
    void updateCharacter() override;

    // 技能冷却比例（用于 HUD 显示）
    qreal skillCooldownRatio() const;
    bool isSkillReady() const { return m_skillCooldownTimer <= 0; }
    void useSkill(); // 释放技能


signals:
    void skillUsed();                              // 技能已使用（通知 UI）
    void interactionPrompt(const QString &text);   // 给玩家的操作提示（HUD）

private:
    // 移动键状态
    bool m_keyW = false, m_keyS = false, m_keyA = false, m_keyD = false;

    int m_skillCooldownTimer = 0;   // 技能冷却剩余帧数
    GameEngine *m_gameEngine = nullptr;

    // 交互提示去重：避免每帧重复发送相同信号，减少 UI 刷新
    bool m_lastPromptActive = false;  // 上一次是否显示了提示

    void updateMoveDirection();
    void checkInteraction();

    enum InteractionType { None, Cipher, Rescue, EscapeGate };
    InteractionType getNearestInteraction(qreal &distance, void **targetPtr);

    // 各角色技能实现
    void useDoctorSkill();
    void useMechanicSkill();
    void useAirForceSkill();
};

#endif // PLAYERSURVIVOR_H