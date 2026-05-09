#ifndef GAMEHUDWIDGET_H
#define GAMEHUDWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>
#include <QHBoxLayout>

class GameEngine;
class PlayerSurvivor;
class Survivor;

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

private slots:
    void updateTeammatesStatus();   // 新增：刷新队友状态

private:
    // 原有控件
    QLabel *m_healthLabel1, *m_healthLabel2, *m_cipherLabel, *m_timeLabel, *m_gateLabel, *m_msgLabel;
    QProgressBar *m_skillBar;
    QPushButton *m_pauseBtn;
    QTimer *m_msgTimer;
    GameEngine *m_engine = nullptr;

    // 新增队友状态控件
    QHBoxLayout *m_teammatesRow;
    QLabel *m_teammateLabels[3];   // 医生、机械师、空军

    void setupUI();
    QString getSurvivorStatusText(Survivor *s, bool isPlayer);
};

#endif