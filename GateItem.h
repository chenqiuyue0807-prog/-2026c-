#ifndef GATEITEM_H
#define GATEITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPainter>

class GateItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit GateItem(const QPointF &pos, QObject *parent = nullptr);

    bool isUnlocked() const { return m_unlocked; }
    void setUnlocked(bool unlock);          // 参数名改为 unlock，避免与信号 unlocked 冲突

    int openProgress() const { return m_openProgress; }
    void setOpenProgress(int prog);

    bool isOpened() const { return m_opened; }
    bool isBeingOpened() const { return m_beingOpened; }
    void setBeingOpened(bool opening);

signals:
    void unlocked();                        // 解锁信号
    void opened();                          // 完全开启信号
    void progressChanged(int progress);     // 开启进度变化信号

private:
    bool m_unlocked;
    int m_openProgress;
    bool m_opened;
    bool m_beingOpened;

    void updateAppearance();
};

#endif // GATEITEM_H