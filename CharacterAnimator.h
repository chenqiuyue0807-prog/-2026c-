#ifndef CHARACTERANIMATOR_H
#define CHARACTERANIMATOR_H

#include <QObject>
#include <QPixmap>
#include <QTimer>
#include "GameConfig.h"  // 里面定义了 Direction

class CharacterAnimator : public QObject
{
    Q_OBJECT
public:
    explicit CharacterAnimator(QObject *parent = nullptr);

    void setUpPixmap(const QPixmap &pm);
    void setDownPixmap(const QPixmap &pm);
    void setLeftPixmap(const QPixmap &pm);
    void setRightPixmap(const QPixmap &pm);

    void setFrameInterval(int ms);      // 切换帧的时间间隔，默认150ms
    void startAnimation();              // 开始行走动画
    void stopAnimation();               // 停止动画，回到站立
    void setDirection(Direction dir);   // 改变朝向
    QPixmap currentPixmap() const;      // 获取当前应显示的图片

    void reset();

signals:
    void pixmapChanged();   // 图片变化时发出（可用于强制重绘）

private slots:
    void advanceFrame();

private:
    QPixmap m_up, m_down, m_left, m_right;
    Direction m_direction = Direction::Down;
    bool m_moving = false;
    int m_currentFrame = 0;    // 0 或 1，用于两帧循环
    QTimer *m_timer;
};

#endif // CHARACTERANIMATOR_H