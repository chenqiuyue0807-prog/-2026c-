#include "CipherMachineItem.h"

CipherMachineItem::CipherMachineItem(const QPointF &pos, QObject *parent)
    : QObject(parent)
    , m_progress(0)
    , m_completed(false)
    , m_beingDecoded(false)
{
    // 绘制一个简单的齿轮图标
    QPixmap pixmap(40, 40);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);
    painter.setBrush(Qt::darkGray);
    painter.drawEllipse(5, 5, 30, 30);
    painter.setBrush(Qt::yellow);
    painter.drawRect(18, 0, 4, 10);
    painter.drawRect(18, 30, 4, 10);
    painter.drawRect(0, 18, 10, 4);
    painter.drawRect(30, 18, 10, 4);

    setPixmap(pixmap);
    setPos(pos);
    setFlag(QGraphicsItem::ItemIsSelectable, false);
}

void CipherMachineItem::setProgress(int prog)
{
    m_progress = qBound(0, prog, 100);
    emit progressChanged(m_progress);

    if (m_progress >= 100 && !m_completed) {
        m_completed = true;
        m_beingDecoded = false;
        updateAppearance();
        emit completed();
    }
}

void CipherMachineItem::setCompleted(bool completed)
{
    m_completed = completed;
    if (completed) {
        m_progress = 100;
        m_beingDecoded = false;
    }
    updateAppearance();
}

void CipherMachineItem::setBeingDecoded(bool decoding)
{
    m_beingDecoded = decoding;
    updateAppearance();
}

void CipherMachineItem::updateAppearance()
{
    QPixmap pixmap(40, 40);
    pixmap.fill(Qt::transparent);
    QPainter painter(&pixmap);

    if (m_completed) {
        painter.setBrush(Qt::green);
    } else {
        painter.setBrush(Qt::darkGray);
    }
    painter.drawEllipse(5, 5, 30, 30);

    // 正在破译时添加白色边框
    if (m_beingDecoded && !m_completed) {
        painter.setPen(QPen(Qt::white, 2));
        painter.drawRect(0, 0, 39, 39);
    }

    painter.setBrush(Qt::yellow);
    painter.drawRect(18, 0, 4, 10);
    painter.drawRect(18, 30, 4, 10);
    painter.drawRect(0, 18, 10, 4);
    painter.drawRect(30, 18, 10, 4);

    setPixmap(pixmap);
}