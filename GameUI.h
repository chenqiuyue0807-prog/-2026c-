#ifndef GAMEUI_H
#define GAMEUI_H

#include <QWidget>
#include <QGraphicsView>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

class GameEngine;
class GameHUDWidget;
class MiniMapWidget;
class CameraFollow;

class GameUI : public QWidget
{
    Q_OBJECT
public:
    explicit GameUI(QWidget *parent = nullptr);
    void setGameEngine(GameEngine *engine);
    void initGame();
    void showResult(bool survivorWin, int escaped, int eliminated, int cipher, int rescue, int destroy);

signals:
    void backToMainMenu();

protected:
    void resizeEvent(QResizeEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;    // 处理键盘按下（暂停 + 转发给视图）
    void keyReleaseEvent(QKeyEvent *event) override;  // 处理键盘释放（转发给视图，角色需要松开事件）
    void paintEvent(QPaintEvent *event) override;     // 绘制背景

private:
    GameEngine *m_engine = nullptr;
    QGraphicsView *m_view;
    GameHUDWidget *m_hud;
    MiniMapWidget *m_minimap;
    CameraFollow *m_camera;
    void setupUI();
    QTimer *m_cameraTimer = nullptr;
};

#endif // GAMEUI_H