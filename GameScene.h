#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QTimer>
#include <QLabel>
#include <QProgressBar>
#include "RoleItem.h"
#include "CipherMachineItem.h"
#include "GateItem.h"

enum class GamePhase {
    Preparation,
    Decoding,
    Escape,
    Settlement
};

class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);
    ~GameScene();

    // 键盘/鼠标事件入口（由 MainWindow 调用）
    void handleKeyPress(int key);
    void handleKeyRelease(int key);
    void handleMousePress(Qt::MouseButton button, const QPointF &pos);

    // 玩家角色
    RoleItem *player() const { return m_player; }

    // 设置 UI 控件（由 MainWindow 传入）
    void setUIWidgets(QLabel *phaseLabel, QLabel *cipherLabel,
                      QLabel *timerLabel, QProgressBar *interactProgress);

signals:
    void gameMessage(const QString &msg);
    void phaseChanged(GamePhase phase);
    void gameOver(bool survivorWin);   // true=求生者胜，false=监管者胜

public slots:
    void updateGame();     // 每帧调用
    void updateAI();       // 每 200ms 调用

private slots:
    void onCipherCompleted();   // 密码机完成时触发

private:
    // --- 键盘状态 ---
    bool m_keyUp, m_keyDown, m_keyLeft, m_keyRight;
    bool m_keySpace;
    bool m_keyF;

    // --- 定时器 ---
    QTimer *m_gameTimer;   // 16ms 帧循环
    QTimer *m_aiTimer;     // 200ms AI 决策

    // --- 角色 ---
    QList<RoleItem*> m_allRoles;
    RoleItem *m_player;    // 玩家控制的求生者
    RoleItem *m_hunter;    // 监管者（AI）

    // --- 游戏阶段 ---
    GamePhase m_gamePhase;
    int m_phaseTimer;      // 阶段倒计时（帧计数，后续完善）

    // --- 第3步：密码机与大门 ---
    QList<CipherMachineItem*> m_cipherMachines;
    QList<GateItem*> m_gates;
    int m_completedCiphers;

    // --- 第3步：交互系统 ---
    RoleItem *m_interactingSurvivor;   // 当前正在交互的求生者（目前仅玩家）
    CipherMachineItem *m_currentCipher;
    GateItem *m_currentGate;
    qreal m_interactProgress;          // 0~100
    qreal m_interactSpeed;             // 每帧增加量

    // --- 第3步：UI 控件指针 ---
    QLabel *m_phaseLabel;
    QLabel *m_cipherLabel;
    QLabel *m_timerLabel;
    QProgressBar *m_interactProgressBar;

    // --- 初始化 ---
    void initScene();
    void initCipherMachines();
    void initGates();

    // --- 第3步：交互处理函数 ---
    void handleInteraction();
    void startDecoding(CipherMachineItem *cipher);
    void startOpeningGate(GateItem *gate);
    void stopInteracting();
    void updateInteractProgress();

    // 场景边界
    QRectF sceneBounds() const { return sceneRect(); }
};

#endif // GAMESCENE_H