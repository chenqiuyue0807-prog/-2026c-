#include "GateItem.h"

GateItem::GateItem(const QPointF &pos, QObject *parent)
    : QObject(parent)
    , m_unlocked(false)
    , m_openProgress(0)
    , m_opened(false)
    , m_beingOpened(false)
{
    // 绘制大门图标
    QPixmap pixmap(50, 60);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setBrush(Qt::darkGray);
    painter.drawRect(0, 0, 50, 60);
    painter.setBrush(Qt::gray);
    painter.drawRect(10, 10, 10, 40);
    painter.drawRect(30, 10, 10, 40);

    setPixmap(pixmap);
    setPos(pos);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void GateItem::setUnlocked(bool unlock)
{
    m_unlocked = unlock;
    updateAppearance();
    if (unlock) {
        emit unlocked();   // 发射解锁信号，无歧义
    }
}

void GateItem::setOpenProgress(int prog)
{
    m_openProgress = qBound(0, prog, 100);
    emit progressChanged(m_openProgress);

    if (m_openProgress >= 100 && !m_opened) {
        m_opened = true;
        m_beingOpened = false;
        updateAppearance();
        emit opened();     // 发射完全开启信号
    }
}

void GateItem::setBeingOpened(bool opening)
{
    m_beingOpened = opening;
    updateAppearance();
}

void GateItem::updateAppearance()
{
    QPixmap pixmap(50, 60);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    // 根据状态设置颜色
    if (!m_unlocked) {
        painter.setBrush(Qt::darkGray);   // 未解锁：灰色
    } else if (m_opened) {
        painter.setBrush(Qt::green);      // 已开启：绿色
    } else {
        painter.setBrush(Qt::darkRed);    // 已解锁未开启：暗红色
    }
    painter.drawRect(0, 0, 50, 60);

    // 正在被交互时加白色边框
    if (m_beingOpened && m_unlocked && !m_opened) {
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(1, 1, 48, 58);
    }

    // 绘制门板细节
    painter.setBrush(Qt::gray);
    painter.drawRect(10, 10, 10, 40);
    painter.drawRect(30, 10, 10, 40);

    setPixmap(pixmap);
}