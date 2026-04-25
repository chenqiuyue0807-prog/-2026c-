#ifndef CAMERAFOLLOW_H
#define CAMERAFOLLOW_H

#include <QObject>
#include <QGraphicsView>

class QGraphicsItem;

class CameraFollow : public QObject
{
    Q_OBJECT
public:
    explicit CameraFollow(QObject *parent = nullptr);
    void setView(QGraphicsView *view);
    void setTarget(QGraphicsItem *target);
    void setSmooth(bool smooth, qreal factor = 0.1);
    void update();
    void snapToTarget();
private:
    QGraphicsView *m_view = nullptr;
    QGraphicsItem *m_target = nullptr;
    bool m_smooth = false;
    qreal m_factor = 0.1;
    QPointF m_currentCenter;
};

#endif