#include "Gate.h"
#include "Survivor.h"
#include <QPainter>
#include <QFont>
#include <QDebug>

Gate::Gate(QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsItem(parent)
    , m_unlocked(false)
    , m_progress(0.0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

void Gate::setUnlocked(bool unlocked) {
    if (m_unlocked != unlocked) {
        m_unlocked = unlocked;
        if (!unlocked) {
            m_progress = 0.0;
            m_openers.clear();
        }
        update();
    }
}

bool Gate::hasOpener(Survivor *survivor) const { return m_openers.contains(survivor); }

void Gate::addOpener(Survivor *survivor) {
    if (!survivor || hasOpener(survivor)) return;
    if (!m_unlocked || isFullyOpen() || m_openers.size() >= 2) return;
    m_openers.append(survivor);
}

void Gate::removeOpener(Survivor *survivor) { m_openers.removeOne(survivor); }

void Gate::updateGate() {
    if (!m_unlocked || isFullyOpen() || m_openers.isEmpty()) return;
    updateOpenProgress();
}

void Gate::updateOpenProgress() {
    qreal speed = currentOpenSpeed();
    m_progress += speed;
    int maxProgress = currentStageMaxProgress();
    if (m_progress > maxProgress) m_progress = maxProgress;

    if (m_progress >= 100.0) {
        m_progress = 100.0;
        emit progressChanged(static_cast<int>(m_progress));
        emit gateFullyOpened();
        for (Survivor *s : m_openers) s->stopOpeningGate();
        m_openers.clear();
    } else {
        emit progressChanged(static_cast<int>(m_progress));
    }
}

qreal Gate::currentOpenSpeed() const {
    if (m_openers.isEmpty()) return 0.0;
    int stageFrames = currentStageFrames();
    if (stageFrames <= 0) return 0.0;
    int currentMax = currentStageMaxProgress();
    int startProgress = 0;
    if (m_progress < STAGE1_THRESHOLD) startProgress = 0;
    else if (m_progress < STAGE2_THRESHOLD) startProgress = STAGE1_THRESHOLD;
    else startProgress = STAGE2_THRESHOLD;
    int remainingInStage = currentMax - startProgress;
    qreal baseSpeedPerFrame = static_cast<qreal>(remainingInStage) / stageFrames;
    qreal totalMultiplier = 1.0 + (m_openers.size() - 1) * 0.5;
    return baseSpeedPerFrame * totalMultiplier;
}

int Gate::currentStageMaxProgress() const {
    if (m_progress < STAGE1_THRESHOLD) return STAGE1_THRESHOLD;
    else if (m_progress < STAGE2_THRESHOLD) return STAGE2_THRESHOLD;
    else return STAGE3_THRESHOLD;
}

int Gate::currentStageFrames() const {
    if (m_progress < STAGE1_THRESHOLD) return STAGE1_FRAMES;
    else if (m_progress < STAGE2_THRESHOLD) return STAGE2_FRAMES;
    else return STAGE3_FRAMES;
}

QRectF Gate::boundingRect() const { return QRectF(-50, -80, 100, 160); }

void Gate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    static QPixmap gatePix(":/new/prefix1/images/men.png");   // 只保留这一个，不要重复定义
    QRectF r = boundingRect();
    if (!gatePix.isNull()) {
      painter->drawPixmap(r.toRect(), gatePix.scaled(100, 160, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QColor color;
        if (!m_unlocked) color = QColor(80, 80, 80);
        else if (isFullyOpen()) color = QColor(50, 200, 50);
        else if (!m_openers.isEmpty()) color = QColor(200, 200, 50);
        else color = QColor(150, 150, 150);
        painter->setBrush(color);
        painter->setPen(QPen(Qt::black, 3));
        painter->drawRect(r);
    }

    if (m_unlocked && !isFullyOpen()) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(0, 0, 255, 150));
        QRectF progressRect(r.left() + 5, r.bottom() - 15, r.width() - 10, 10);
        painter->drawRect(progressRect);
        painter->setPen(Qt::white);
        painter->setFont(QFont("Arial", 8));
        painter->drawText(r, Qt::AlignCenter, QString::number(static_cast<int>(m_progress)) + "%");
    }
}