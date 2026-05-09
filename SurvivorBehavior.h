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

    void setScene(GameScene *scene) { m_scene = scene; }
    void setCiphers(const QList<CipherMachine*> &ciphers) { m_ciphers = ciphers; }
    void setGates(const QList<Gate*> &gates) { m_gates = gates; }
    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setPlayerSurvivor(PlayerSurvivor *player) { m_player = player; }
    void setSurvivors(const QList<AISurvivor*> &ai, PlayerSurvivor *player);

    void reset();
    void updateDecision(AISurvivor *ai);
    void setGamePhase(int phase) { m_gamePhase = phase; }
    Survivor* findInjuredPlayerNearby(AISurvivor *ai);

private:
    bool isHunterNearby(AISurvivor *ai, qreal threshold = 150.0);
    bool isBeingChased(AISurvivor *ai);
    CipherMachine* findBestCipher(AISurvivor *ai);
    Gate* findNearestUnlockedGate(AISurvivor *ai);
    Survivor* findBurningTeammate(AISurvivor *ai, bool prioritizePlayer = true);
    Survivor* findInjuredTeammate(AISurvivor *ai, bool prioritizePlayer = true);
    QPointF findHidingSpot(AISurvivor *ai);
    Bush* findNearestBush(AISurvivor *ai);
    bool shouldUseSkill(AISurvivor *ai);
    void useSkillIfPossible(AISurvivor *ai);

    void performDecodeBehavior(AISurvivor *ai);
    void performEscapeBehavior(AISurvivor *ai);
    void performHideBehavior(AISurvivor *ai);
    void performRescueBehavior(AISurvivor *ai, Survivor *target);
    void performHealBehavior(AISurvivor *ai, Survivor *target);

    GameScene *m_scene;
    QList<CipherMachine*> m_ciphers;
    QList<Gate*> m_gates;
    Hunter *m_hunter;
    int m_gamePhase;
    QList<AISurvivor*> m_aiSurvivors;
    PlayerSurvivor *m_player = nullptr;
};

#endif // SURVIVORBEHAVIOR_H