#ifndef SURVIVORBEHAVIOR_H
#define SURVIVORBEHAVIOR_H

#include <QObject>
#include <QList>
#include <QPointF>

class GameScene;
class AISurvivor;
class PlayerSurvivor;
class CipherMachine;
class Gate;
class Hunter;
class Survivor;
class Bush;

class SurvivorBehavior : public QObject
{
    Q_OBJECT

public:
    explicit SurvivorBehavior(QObject *parent = nullptr);

    // 设置依赖对象
    void setScene(GameScene *scene) { m_scene = scene; }
    void setCiphers(const QList<CipherMachine*> &ciphers) { m_ciphers = ciphers; }
    void setGates(const QList<Gate*> &gates) { m_gates = gates; }
    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setPlayerSurvivor(PlayerSurvivor *player) { m_player = player; }

    // 重置状态（新对局时调用）
    void reset();

    // 对单个 AI 求生者进行决策更新（由 AIDecisionMaker 每 0.2 秒调用）
    void updateDecision(AISurvivor *ai);

    // 设置当前游戏阶段（0准备，1破译，2逃脱，3结算）
    void setGamePhase(int phase) { m_gamePhase = phase; }

private:
    // 决策辅助
    bool isHunterNearby(AISurvivor *ai, qreal threshold = 150.0);
    bool isBeingChased(AISurvivor *ai);
    CipherMachine* findBestCipher(AISurvivor *ai);
    Gate* findNearestUnlockedGate(AISurvivor *ai);
    Survivor* findBurningTeammate(AISurvivor *ai, bool prioritizePlayer = true);
    QPointF findHidingSpot(AISurvivor *ai);
    Bush* findNearestBush(AISurvivor *ai);
    bool shouldUseSkill(AISurvivor *ai);
    void useSkillIfPossible(AISurvivor *ai);

    // 行为执行
    void performDecodeBehavior(AISurvivor *ai);
    void performEscapeBehavior(AISurvivor *ai);
    void performHideBehavior(AISurvivor *ai);
    void performRescueBehavior(AISurvivor *ai, Survivor *target);

    // 外部对象
    GameScene *m_scene;
    QList<CipherMachine*> m_ciphers;
    QList<Gate*> m_gates;
    Hunter *m_hunter;
    PlayerSurvivor *m_player;
    int m_gamePhase; // 0准备,1破译,2逃脱,3结算
};

#endif // SURVIVORBEHAVIOR_H