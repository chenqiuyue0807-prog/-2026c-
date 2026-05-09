#ifndef HUNTERBEHAVIOR_H
#define HUNTERBEHAVIOR_H

#include <QObject>
#include <QPointF>
#include <QList>

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

    void setScene(GameScene *scene) { m_scene = scene; }
    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setCiphers(const QList<CipherMachine*> &ciphers) { m_ciphers = ciphers; }
    void setGates(const QList<Gate*> &gates) { m_gates = gates; }
    void setGamePhase(int phase) { m_gamePhase = phase; }
    void setSurvivors(const QList<AISurvivor*> &aiSurvivors, PlayerSurvivor *player);

    void reset();
    void updateDecision();

private:
    void patrolUpdate();
    void chaseUpdate();
    void searchUpdate();

    Survivor* findNearestSurvivorInRange(qreal range);
    bool canSeeSurvivor(Survivor *survivor);
    void moveTo(const QPointF &target);
    bool isPathBlocked(const QPointF &from, const QPointF &to);
    void tryDestroyBlockingObstacle();
    void handleBushes();
    void tryAttack(Survivor *target);

    int m_patrolIndex;
    QList<int> m_patrolOrder;

    enum State { Patrolling, Chasing, Searching };
    State m_state;
    Survivor *m_chaseTarget;
    QPointF m_lastSeenPos;        // 最后一次看到目标的位置（用于搜索）
    QPointF m_lastSeePos;         // 上一次目标位置（用于预测移动）
    int m_lostTimer;
    int m_searchTimer;

    static constexpr int LOSE_TIMEOUT_TICKS = 30;    // 6秒 / 0.2秒
    static constexpr int SEARCH_DURATION_TICKS = 15;

    GameScene *m_scene;
    Hunter *m_hunter;
    QList<CipherMachine*> m_ciphers;
    QList<Survivor*> m_allSurvivors;
    QList<Gate*> m_gates;
    int m_gamePhase;
};

#endif