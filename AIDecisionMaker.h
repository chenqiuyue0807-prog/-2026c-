#ifndef AIDECISIONMAKER_H
#define AIDECISIONMAKER_H

#include <QObject>
#include <QTimer>
#include <QList>

class GameScene;
class Hunter;
class AISurvivor;
class PlayerSurvivor;

class AIDecisionMaker : public QObject
{
    Q_OBJECT

public:
    explicit AIDecisionMaker(QObject *parent = nullptr);
    ~AIDecisionMaker();

    // 设置依赖对象（由 GameEngine 在 startNewGame 后调用）
    void setScene(GameScene *scene) { m_scene = scene; }
    void setHunter(Hunter *hunter) { m_hunter = hunter; }
    void setSurvivors(const QList<AISurvivor*> &survivors) { m_aiSurvivors = survivors; }
    void setPlayer(PlayerSurvivor *player) { m_player = player; }

    // 启动/停止决策
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
    QTimer m_timer;

    // 简易 AI 决策
    void updateHunterAI();
    void updateSurvivorAI(AISurvivor *survivor);
};

#endif // AIDECISIONMAKER_H