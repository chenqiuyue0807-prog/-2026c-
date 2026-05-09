#ifndef MECHANICALPUPPET_H
#define MECHANICALPUPPET_H

#include <QObject>
#include <QGraphicsRectItem>
#include <QTimer>
#include <QPointF>
#include <QPixmap>
#include <QPainter>
#include <QStyleOptionGraphicsItem>

class CipherMachine;

class MechanicalPuppet : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    explicit MechanicalPuppet(const QPointF &startPos, QGraphicsScene *scene, QObject *parent = nullptr);
    ~MechanicalPuppet();

    void moveToTarget(const QPointF &target);
    void destroy();   // 被监管者攻击时调用
    qreal m_decodeSpeed = 0.0;
    bool m_destroyed = false;

protected:
    void advance(int phase) override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

private slots:
    void onTimeout();

private:
    QTimer m_timer;
    QGraphicsScene *m_scene;
    QPointF m_target;
    bool m_moving;
    QPixmap m_pixmap;

    // 破译相关
    CipherMachine *m_targetCipher = nullptr;
    bool m_decoding = false;
};

#endif