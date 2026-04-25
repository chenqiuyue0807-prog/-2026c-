#ifndef UIMANAGER_H
#define UIMANAGER_H

#include <QObject>

class GameEngine;
class GameHUDWidget;
class PlayerSurvivor;

class UIManager : public QObject
{
    Q_OBJECT
public:
    explicit UIManager(QObject *parent = nullptr);
    void setEngine(GameEngine *engine);
    void setHUD(GameHUDWidget *hud);
    void setPlayer(PlayerSurvivor *player);
    void connectSignals();
    void refresh();

private slots:
    void onTimeUpdated(int sec);
    void onCipherChanged(int completed, int total);
    void onGameStateChanged(int state);
    void onHealthChanged(int health);
    void onSkillUsed();
    void onPrompt(const QString &text);
    void updateGateStatus();

private:
    GameEngine *m_engine = nullptr;
    GameHUDWidget *m_hud = nullptr;
    PlayerSurvivor *m_player = nullptr;
};

#endif