#ifndef CIPHERMACHINEITEM_H
#define CIPHERMACHINEITEM_H

#include <QObject>
#include <QGraphicsPixmapItem>
#include <QPainter>

class CipherMachineItem : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit CipherMachineItem(const QPointF &pos, QObject *parent = nullptr);

    // 破译进度 (0-100)
    int progress() const { return m_progress; }
    void setProgress(int prog);

    // 是否已完成
    bool isCompleted() const { return m_completed; }
    void setCompleted(bool completed);

    // 是否正被破译
    bool isBeingDecoded() const { return m_beingDecoded; }
    void setBeingDecoded(bool decoding);

signals:
    void progressChanged(int progress);
    void completed();

private:
    int m_progress;
    bool m_completed;
    bool m_beingDecoded;

    void updateAppearance();
};

#endif // CIPHERMACHINEITEM_H