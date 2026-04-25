#ifndef CIPHERMACHINE_H
#define CIPHERMACHINE_H

#include <QObject>          // 新增
#include <QGraphicsItem>
#include <QList>

class Survivor;

class CipherMachine : public QObject, public QGraphicsItem
{
    Q_OBJECT               // 关键宏
public:
    explicit CipherMachine(QGraphicsItem *parent = nullptr);

    // 破译进度 (0~100)
    int progress() const { return m_progress; }
    bool isCompleted() const { return m_progress >= 100; }

    // 当前正在破译的求生者人数
    int currentDecoders() const { return m_decoders.size(); }
    bool hasDecoder(Survivor *survivor) const;

    // 添加/移除破译者（由 Survivor 调用）
    void addDecoder(Survivor *survivor);
    void removeDecoder(Survivor *survivor);

    // 每帧更新破译进度（由 GameEngine 驱动）
    void updateCipher();

    // 绘制
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void progressChanged(int progress);
    void decodeCompleted();                 // 破译完成
    void mistakeTriggered(Survivor *survivor); // 破译失误，通知暴露位置

private:
    int m_progress;                         // 0~100
    QList<Survivor*> m_decoders;            // 当前破译者列表

    // 破译失误计时（帧计数）
    int m_mistakeTimer;                     // 用于每5秒检测一次
    static constexpr int MISTAKE_CHECK_INTERVAL = 300; // 5秒 * 60FPS

    // 辅助
    void updateDecodeProgress();
    qreal currentDecodeSpeed() const;       // 返回每帧增加的进度值
    void checkMistake();
};

#endif // CIPHERMACHINE_H