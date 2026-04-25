#ifndef GATE_H
#define GATE_H

#include <QGraphicsItem>
#include <QList>
#include <QObject>

class Survivor;

class Gate : public QObject, public QGraphicsItem
{
    Q_OBJECT
public:
    explicit Gate(QGraphicsItem *parent = nullptr);

    // 解锁/锁定（由 GameEngine 在密码机全部破译后调用）
    void setUnlocked(bool unlocked);
    bool isUnlocked() const { return m_unlocked; }

    // 开启进度 (0~100)
    int progress() const { return m_progress; }
    bool isFullyOpen() const { return m_progress >= 100; }

    // 当前正在开启大门的求生者人数
    int currentOpeners() const { return m_openers.size(); }
    bool hasOpener(Survivor *survivor) const;

    // 添加/移除开启者（由 Survivor 调用）
    void addOpener(Survivor *survivor);
    void removeOpener(Survivor *survivor);

    // 每帧更新（由 GameEngine 驱动）
    void updateGate();

    // 绘制
    QRectF boundingRect() const override;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

signals:
    void progressChanged(int progress);
    void gateFullyOpened();          // 大门完全开启
    void survivorEscaped(Survivor *survivor); // 某求生者逃脱（可选）

private:
    bool m_unlocked;                 // 是否已解锁
    int m_progress;                  // 0~100
    QList<Survivor*> m_openers;      // 当前开启者列表

    // 分段参数（百分比阈值）
    static constexpr int STAGE1_THRESHOLD = 30;
    static constexpr int STAGE2_THRESHOLD = 70;
    static constexpr int STAGE3_THRESHOLD = 100;

    // 每段所需帧数（60FPS）
    static constexpr int STAGE1_FRAMES = 180;   // 3秒 * 60
    static constexpr int STAGE2_FRAMES = 240;   // 4秒 * 60
    static constexpr int STAGE3_FRAMES = 180;   // 3秒 * 60

    // 辅助
    void updateOpenProgress();
    qreal currentOpenSpeed() const;   // 每帧增加的进度
    int currentStageMaxProgress() const; // 当前阶段上限
    int currentStageFrames() const;      // 当前阶段所需总帧数
};

#endif // GATE_H