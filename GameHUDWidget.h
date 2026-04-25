#ifndef GAMEHUDWIDGET_H
#define GAMEHUDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

class GameEngine;
class PlayerSurvivor;

class GameHUDWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GameHUDWidget(QWidget *parent = nullptr);
    void setGameEngine(GameEngine *engine);
    void setPlayer(PlayerSurvivor *player);
public slots:
    void updateHealthDisplay(int health);
    void updateSkillCooldown(qreal ratio);
    void updateCipherProgress(int completed, int total);
    void updateTimeDisplay(int seconds);
    void updateGateStatus(bool unlocked, int progress);
    void showMessage(const QString &msg, int durationMs = 2000);
    void reset();
signals:
    void pauseClicked();
private:
    QLabel *m_healthLabel1, *m_healthLabel2, *m_cipherLabel, *m_timeLabel, *m_gateLabel, *m_msgLabel;
    QProgressBar *m_skillBar;
    QPushButton *m_pauseBtn;
    QTimer *m_msgTimer;
    GameEngine *m_engine = nullptr;
    void setupUI();
};

#endif