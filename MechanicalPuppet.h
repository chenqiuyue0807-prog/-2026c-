#ifndef MECHANICALPUPPET_H
#define MECHANICALPUPPET_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPointF>

class MechanicalPuppet : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit MechanicalPuppet(const QPointF &startPos, QGraphicsScene *scene, QObject *parent = nullptr);
    ~MechanicalPuppet();

    // 自动移动向目标点（最近的未完成密码机）
    void moveToTarget(const QPointF &target);

protected:
    void advance(int phase) override;  // 用于每帧移动

private slots:
    void onTimeout();  // 15秒后自动删除

private:
    QTimer m_timer;
    QGraphicsScene *m_scene;
    QPointF m_target;
    bool m_moving;
};

#endif