#ifndef HUNTERBEHAVIOR_H
#define HUNTERBEHAVIOR_H

#include <QObject>
#include <QPointF>
#include <QList>
#include <QElapsedTimer>

class GameScene;
class Hunter;
class Survivor;
class AISurvivor;
class PlayerSurvivor;
class CipherMachine;
class Gate;
class Obstacle;
class Bush;

class HunterBehavior : public QObject
{
    Q_OBJECT

public:
    explicit HunterBehavior(QObject *parent = nullptr);

    // 依赖注入
    void setScene(GameScene *scene) { m_scene = scene; }
    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setCiphers(const QList<CipherMachine*> &ciphers) { m_ciphers = ciphers; }
    void setSurvivors(const QList<AISurvivor*> &aiSurvivors, PlayerSurvivor *player);
    void setGates(const QList<Gate*> &gates) { m_gates = gates; }
    void setGamePhase(int phase) { m_gamePhase = phase; } // 0准备，1破译，2逃脱，3结算

    // 重置状态（新对局时调用）
    void reset();

    // 每 0.2 秒决策一次
    void updateDecision();

private:
    // 巡逻相关
    void patrolUpdate();
    void chaseUpdate();
    void searchUpdate();

    // 辅助方法
    Survivor* findNearestSurvivorInRange(qreal range);
    bool canSeeSurvivor(Survivor *survivor);
    void moveTo(const QPointF &target);
    bool isPathBlocked(const QPointF &from, const QPointF &to);
    void tryDestroyBlockingObstacle();
    void handleBushes();
    void tryAttack(Survivor *target);

    // 巡逻顺序（密码机索引，按照 0,1,2）
    int m_patrolIndex;
    QList<int> m_patrolOrder;

    // 状态
    enum State { Patrolling, Chasing, Searching };
    State m_state;
    Survivor *m_chaseTarget;
    QPointF m_lastSeenPos;
    int m_lostTimer;      // 丢失目标的决策周期计数（1周期=0.2秒）
    int m_searchTimer;    // 搜索停留的决策周期计数

    static constexpr int LOSE_TIMEOUT_TICKS = 40;   // 8秒 / 0.2秒 = 40周期
    static constexpr int SEARCH_DURATION_TICKS = 20; // 4秒 / 0.2秒 = 20周期

    // 外部对象
    GameScene *m_scene;
    Hunter *m_hunter;
    QList<CipherMachine*> m_ciphers;
    QList<Survivor*> m_allSurvivors; // 玩家 + AI
    QList<Gate*> m_gates;
    int m_gamePhase; // 0准备,1破译,2逃脱,3结算
};

#endif // HUNTERBEHAVIOR_H