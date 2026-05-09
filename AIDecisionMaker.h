#ifndef AIDECISIONMAKER_H
#define AIDECISIONMAKER_H

#include <QObject>
#include <QTimer>
#include <QList>

class GameScene;
class Hunter;
class AISurvivor;
class PlayerSurvivor;
class HunterBehavior;      // 新增前置声明
class SurvivorBehavior;    // 新增前置声明

class AIDecisionMaker : public QObject
{
    Q_OBJECT

public:
    explicit AIDecisionMaker(QObject *parent = nullptr);
    ~AIDecisionMaker();

    void setScene(GameScene *scene) { m_scene = scene; }
    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setSurvivors(const QList<AISurvivor*> &survivors) { m_aiSurvivors = survivors; }
    void setPlayer(PlayerSurvivor *player) { m_player = player; }

    // 新增：注入行为模块
    void setBehaviors(HunterBehavior *hb, SurvivorBehavior *sb) {
        m_hunterBehavior = hb;
        m_survivorBehavior = sb;
    }

    void start();
    void stop();
    void reset();

private slots:
    void makeDecision();

private:
    GameScene *m_scene;
    Hunter *m_hunter;
    QList<AISurvivor*> m_aiSurvivors;
    PlayerSurvivor *m_player;
    HunterBehavior *m_hunterBehavior = nullptr;     // 新增
    SurvivorBehavior *m_survivorBehavior = nullptr; // 新增
    QTimer m_timer;
};

#endif