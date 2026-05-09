#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include <QList>
#include "GameConfig.h"

class CipherMachine;
class Gate;
class GameScene;
class PlayerSurvivor;
class Hunter;
class AISurvivor;
class AIDecisionMaker;
class HunterBehavior;
class SurvivorBehavior;

class GameEngine : public QObject
{
    Q_OBJECT

public:
    enum GameState {
        Preparation,
        Decoding,
        Escape,
        Result
    };

    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    void startGame(int survivorType);
    GameScene* getScene() const { return m_scene; }
    GameState currentState() const { return m_state; }
    int remainingTime() const;
    int completedCipherCount() const;
    int aliveSurvivorCount() const;
    int escapedSurvivorCount() const { return m_escapedCount; }
    int rescueCount() const { return m_rescueCount; }
    int destroyCount() const { return m_destroyCount; }

    PlayerSurvivor* getPlayer() const { return m_player; }
    Hunter* getHunter() const { return m_hunter; }
    QList<AISurvivor*> getAISurvivors() const { return m_aiSurvivors; }
    QList<CipherMachine*> getCiphers() const { return m_cachedCiphers; }
    QList<Gate*> getGates() const { return m_cachedGates; }

signals:
    void timeUpdated(int seconds);
    void cipherProgressChanged(int completed, int total);
    void gameStateChanged(GameState newState);
    void gameEnded(bool survivorWin);

public slots:
    void pauseGame();
    void resumeGame();
    void restartGame();
    void exitToMainMenu();

private slots:
    void gameLoop();

private:
    void setGameState(GameState state);
    void createGameWorld(int survivorType);
    void cleanupGameWorld();
    void updateCountdown();
    void updateDynamicMusic();       // 新增：动态切换背景音乐
    void checkVictoryCondition();
    void survivorWin();
    void hunterWin();

    GameScene *m_scene;
    PlayerSurvivor *m_player;
    Hunter *m_hunter;
    QList<AISurvivor*> m_aiSurvivors;
    AIDecisionMaker *m_aiDecisionMaker;

    HunterBehavior *m_hunterBehavior;
    SurvivorBehavior *m_survivorBehavior;

    QList<CipherMachine*> m_cachedCiphers;
    QList<Gate*> m_cachedGates;

    QTimer m_frameTimer;
    QElapsedTimer m_stageTimer;
    bool m_paused;
    GameState m_state;

    int m_escapedCount;
    int m_rescueCount;
    int m_destroyCount;
    int m_lastSurvivorType;

    static constexpr int PREPARATION_DURATION = 10000;
    static constexpr int DECODING_DURATION = 240000;
    static constexpr int ESCAPE_DURATION = 60000;
    static constexpr int FRAME_INTERVAL = 17;
};

#endif // GAMEENGINE_H