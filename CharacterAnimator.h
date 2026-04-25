#ifndef CHARACTERANIMATOR_H
#define CHARACTERANIMATOR_H

#include <QObject>
#include <QPixmap>
#include <QTimer>
#include "GameConfig.h"

class CharacterAnimator : public QObject
{
    Q_OBJECT
public:
    explicit CharacterAnimator(QObject *parent = nullptr);
    void setUpPixmap(const QPixmap &pm);
    void setDownPixmap(const QPixmap &pm);
    void setLeftPixmap(const QPixmap &pm);
    void setRightPixmap(const QPixmap &pm);
    void setFrameInterval(int ms);
    void startAnimation();
    void stopAnimation();
    void setDirection(Direction dir);
    QPixmap currentPixmap() const;
    void reset();
signals:
    void pixmapChanged();
private slots:
    void advanceFrame();
private:
    QPixmap m_up, m_down, m_left, m_right;
    Direction m_dir = Direction::Down;
    bool m_moving = false;
    int m_frame = 0;
    QTimer *m_timer;
};

#endif