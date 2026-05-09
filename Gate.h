#ifndef GATE_H
#define GATE_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <QWidget>
#include <QList>

class Survivor;

class Gate : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit Gate(QGraphicsItem *parent = nullptr);

    void setUnlocked(bool unlocked);
    bool isUnlocked() const { return m_unlocked; }

    int progress() const { return static_cast<int>(m_progress); }
    bool isFullyOpen() const { return m_progress >= 100.0; }

    int currentOpeners() const { return m_openers.size(); }
    bool hasOpener(Survivor *survivor) const;

    void addOpener(Survivor *survivor);
    void removeOpener(Survivor *survivor);

    void updateGate();

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void progressChanged(int progress);
    void gateFullyOpened();
    void survivorEscaped(Survivor *survivor);

private:
    bool m_unlocked;
    qreal m_progress = 0.0;
    QList<Survivor*> m_openers;

    static constexpr int STAGE1_THRESHOLD = 30;
    static constexpr int STAGE2_THRESHOLD = 70;
    static constexpr int STAGE3_THRESHOLD = 100;

    // 8秒总时间分段：2.4s、2.4s、3.2s → 144帧、144帧、192帧
    static constexpr int STAGE1_FRAMES = 144;
    static constexpr int STAGE2_FRAMES = 144;
    static constexpr int STAGE3_FRAMES = 192;

    void updateOpenProgress();
    qreal currentOpenSpeed() const;
    int currentStageMaxProgress() const;
    int currentStageFrames() const;
};

#endif