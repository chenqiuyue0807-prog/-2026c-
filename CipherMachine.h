#ifndef CIPHERMACHINE_H
#define CIPHERMACHINE_H

#include <QObject>
#include <QGraphicsItem>
#include <QList>

class Survivor;

class CipherMachine : public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    explicit CipherMachine(QGraphicsItem *parent = nullptr);

    int progress() const { return m_progress; }
    bool isCompleted() const { return m_progress >= 100; }

    int currentDecoders() const { return m_decoders.size(); }
    bool hasDecoder(Survivor *survivor) const;

    void addDecoder(Survivor *survivor);
    void removeDecoder(Survivor *survivor);

    void updateCipher();
    void addPuppetProgress(qreal speed);

    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void progressChanged(int progress);
    void decodeCompleted();
    void mistakeTriggered(Survivor *survivor);

private:
    qreal m_progress;
    QList<Survivor*> m_decoders;

    int m_mistakeTimer;
    static constexpr int MISTAKE_CHECK_INTERVAL = 300;

    void updateDecodeProgress();
    qreal currentDecodeSpeed() const;
    void checkMistake();
};

#endif