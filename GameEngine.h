#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QElapsedTimer>
#include "GameConfig.h"
class CipherMachine;
class Gate;
class GameScene;
class PlayerSurvivor;
class Hunter;
class AISurvivor;
class QGraphicsRectItem;   // 代表密码机/大门/草丛
class QGraphicsItem;       // 新增：因为 CipherData 和 GateData 中使用了 QGraphicsItem*

class GameEngine : public QObject
{
    Q_OBJECT

public:
    enum GameState {
        Preparation,   // 准备阶段
        Decoding,      // 破译阶段
        Escape,        // 逃脱阶段
        Result         // 结算阶段
    };

    explicit GameEngine(QObject *parent = nullptr);
    ~GameEngine();

    // 开始新游戏（传入玩家角色类型）
    void startGame(int survivorType);

    // 获取场景
    GameScene* getScene() const { return m_scene; }

    // 当前阶段
    GameState currentState() const { return m_state; }

    // 剩余时间（秒）
    int remainingTime() const;

    // 密码机进度
    int completedCipherCount() const;

    // 存活/逃脱人数
    int aliveSurvivorCount() const;
    int escapedSurvivorCount() const { return m_escapedCount; }

    // 统计（可在结算时用）
    int rescueCount() const { return m_rescueCount; }
    int destroyCount() const { return m_destroyCount; }

    PlayerSurvivor* getPlayer() const { return m_player; }
    Hunter* getHunter() const { return m_hunter; }
    QList<AISurvivor*> getAISurvivors() const { return m_aiSurvivors; }
    QList<CipherMachine*> getCiphers() const;   // 实现见 .cpp
    QList<Gate*> getGates() const;              // 实现见 .cpp
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
    // 阶段切换
    void setGameState(GameState state);

    // 创建/清理游戏世界
    void createGameWorld(int survivorType);
    void cleanupGameWorld();

    // 每帧更新各阶段逻辑
    void updateCountdown();

    // 胜负检查
    void checkVictoryCondition();
    void survivorWin();
    void hunterWin();

    // AI 控制（简易）
    void updateAI();

    // 角色指针
    GameScene *m_scene;
    PlayerSurvivor *m_player;
    Hunter *m_hunter;
    QList<AISurvivor*> m_aiSurvivors;

    // 图形项（密码机、大门、草丛）——这里用 QGraphicsRectItem 表示
    struct CipherData {
        QGraphicsItem *item;
        int progress;
        bool completed;
    };
    QList<CipherData> m_ciphers;

    struct GateData {
        QGraphicsItem *item;   // 改为 QGraphicsItem*
        int progress;
        bool unlocked;
    };
    QList<GateData> m_gates;

    // 计时
    QTimer m_frameTimer;
    QElapsedTimer m_stageTimer;
    bool m_paused;
    GameState m_state;

    // 统计数据
    int m_escapedCount;
    int m_rescueCount;
    int m_destroyCount;

    // 常量
    static constexpr int PREPARATION_DURATION = 10000;   // 10 秒
    static constexpr int DECODING_DURATION = 240000;    // 4 分钟
    static constexpr int ESCAPE_DURATION = 60000;       // 1 分钟
    static constexpr int FRAME_INTERVAL = 16;           // ~60 FPS
};

#endif // GAMEENGINE_H