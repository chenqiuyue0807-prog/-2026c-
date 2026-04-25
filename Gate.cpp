#include "Gate.h"
#include "Survivor.h"
#include <QPainter>
#include <QFont>
#include <QDebug>

Gate::Gate(QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsItem(parent)
    , m_unlocked(false)
    , m_progress(0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

void Gate::setUnlocked(bool unlocked)
{
    if (m_unlocked != unlocked) {
        m_unlocked = unlocked;
        if (!unlocked) {
            // 重新锁定时重置进度（通常不会发生，但以防万一）
            m_progress = 0;
            m_openers.clear();
        }
        update(); // 重绘
    }
}

bool Gate::hasOpener(Survivor *survivor) const
{
    return m_openers.contains(survivor);
}

void Gate::addOpener(Survivor *survivor)
{
    if (!survivor || hasOpener(survivor)) return;
    if (!m_unlocked) return;
    if (isFullyOpen()) return;
    if (m_openers.size() >= 2) return; // 最多2人同时开启
    m_openers.append(survivor);
}

void Gate::removeOpener(Survivor *survivor)
{
    m_openers.removeOne(survivor);
}

void Gate::updateGate()
{
    if (!m_unlocked || isFullyOpen() || m_openers.isEmpty())
        return;

    updateOpenProgress();
}

void Gate::updateOpenProgress()
{
    qreal speed = currentOpenSpeed();
    m_progress += static_cast<int>(speed * 100); // 百分比累加

    // 不能超过当前阶段上限
    int maxProgress = currentStageMaxProgress();
    if (m_progress > maxProgress) {
        m_progress = maxProgress;
    }

    if (m_progress >= 100) {
        m_progress = 100;
        emit progressChanged(m_progress);
        emit gateFullyOpened();

        // 完全开启后，强制所有开启者停止交互
        for (Survivor *s : m_openers) {
            s->stopOpeningGate();
        }
        m_openers.clear();
    } else {
        emit progressChanged(m_progress);
    }
}

qreal Gate::currentOpenSpeed() const
{
    if (m_openers.isEmpty()) return 0.0;

    // 计算当前阶段剩余的进度和所需帧数
    int stageFrames = currentStageFrames();
    if (stageFrames <= 0) return 0.0;

    int currentMax = currentStageMaxProgress();
    // 当前阶段起始进度
    int startProgress = 0;
    if (m_progress < STAGE1_THRESHOLD) {
        startProgress = 0;
    } else if (m_progress < STAGE2_THRESHOLD) {
        startProgress = STAGE1_THRESHOLD;
    } else {
        startProgress = STAGE2_THRESHOLD;
    }
    int remainingInStage = currentMax - startProgress;
    // 基础速度：剩余进度 / 总帧数
    qreal baseSpeedPerFrame = static_cast<qreal>(remainingInStage) / stageFrames;

    // 人数速度倍率：第一人1.0，每增加一人+0.5（文档：每增加1名求生者，开启速度提升50%）
    qreal totalMultiplier = 1.0 + (m_openers.size() - 1) * 0.5;

    return baseSpeedPerFrame * totalMultiplier;
}

int Gate::currentStageMaxProgress() const
{
    if (m_progress < STAGE1_THRESHOLD) {
        return STAGE1_THRESHOLD;
    } else if (m_progress < STAGE2_THRESHOLD) {
        return STAGE2_THRESHOLD;
    } else {
        return STAGE3_THRESHOLD;
    }
}

int Gate::currentStageFrames() const
{
    if (m_progress < STAGE1_THRESHOLD) {
        return STAGE1_FRAMES;
    } else if (m_progress < STAGE2_THRESHOLD) {
        return STAGE2_FRAMES;
    } else {
        return STAGE3_FRAMES;
    }
}

QRectF Gate::boundingRect() const
{
    return QRectF(-40, -60, 80, 120);
}

void Gate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    QColor color;
    if (!m_unlocked) {
        color = QColor(80, 80, 80);        // 未解锁：深灰色
    } else if (isFullyOpen()) {
        color = QColor(50, 200, 50);       // 已完全开启：绿色
    } else if (!m_openers.isEmpty()) {
        color = QColor(200, 200, 50);      // 开启中：黄色
    } else {
        color = QColor(150, 150, 150);     // 已解锁但无人开启：浅灰色
    }

    painter->setBrush(color);
    painter->setPen(QPen(Qt::black, 3));
    painter->drawRect(boundingRect());

    // 门把手
    painter->setBrush(QColor(100, 100, 100));
    painter->drawEllipse(QRectF(20, -10, 10, 20));

    // 进度条（解锁且未完全开启时显示）
    if (m_unlocked && !isFullyOpen()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 255, 150));
        QRectF progressRect(-35, 30, 70 * m_progress / 100.0, 10);
        painter->drawRect(progressRect);

        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 8));
        painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_progress) + "%");
    } else if (isFullyOpen()) {
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 10, QFont::Bold));
        painter->drawText(boundingRect(), Qt::AlignCenter, "OPEN");
    } else if (!m_unlocked) {
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 8));
        painter->drawText(boundingRect(), Qt::AlignCenter, "LOCKED");
    }
}