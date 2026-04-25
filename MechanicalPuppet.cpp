#include "MechanicalPuppet.h"
#include <QPropertyAnimation>
#include <QGraphicsScene>
#include <QDebug>
#include <cmath>

MechanicalPuppet::MechanicalPuppet(const QPointF &startPos, QGraphicsScene *scene, QObject *parent)
    : QObject(parent), QGraphicsRectItem(), m_scene(scene), m_moving(false)
{
    setRect(-10, -10, 20, 20);
    setBrush(QBrush(Qt::cyan));
    setPen(QPen(Qt::blue));
    setPos(startPos);
    if (m_scene) m_scene->addItem(this);

    // 自动销毁定时器
    connect(&m_timer, &QTimer::timeout, this, &MechanicalPuppet::onTimeout);
    m_timer.start(15000);
}

MechanicalPuppet::~MechanicalPuppet()
{
    // 从场景移除
    if (m_scene) m_scene->removeItem(this);
}

void MechanicalPuppet::moveToTarget(const QPointF &target)
{
    m_target = target;
    m_moving = true;
}

void MechanicalPuppet::advance(int phase)
{
    if (!phase) return; // 只处理逻辑阶段
    if (!m_moving) return;

    QPointF dir = m_target - pos();
    qreal len = std::hypot(dir.x(), dir.y());
    if (len < 5.0) {
        m_moving = false;
        // 到达目标，可以开始模拟破译（可选，目前只移动）
        return;
    }
    QPointF step = dir / len * 2.0; // 移动速度
    setPos(pos() + step);
}

void MechanicalPuppet::onTimeout()
{
    deleteLater();
}