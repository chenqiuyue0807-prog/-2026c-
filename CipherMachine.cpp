#include "CipherMachine.h"
#include "Survivor.h"
#include "GameConfig.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QDebug>

CipherMachine::CipherMachine(QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsItem(parent)   // 初始化 QObject 和 QGraphicsItem
    , m_progress(0)
    , m_mistakeTimer(0)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
}

bool CipherMachine::hasDecoder(Survivor *survivor) const
{
    return m_decoders.contains(survivor);
}

void CipherMachine::addDecoder(Survivor *survivor)
{
    if (!survivor || hasDecoder(survivor)) return;
    if (m_decoders.size() >= GameConfig::MAX_SURVIVORS_PER_CIPHER) return;
    if (isCompleted()) return;

    m_decoders.append(survivor);
}

void CipherMachine::removeDecoder(Survivor *survivor)
{
    m_decoders.removeOne(survivor);
}

void CipherMachine::updateCipher()
{
    if (isCompleted()) return;
    if (m_decoders.isEmpty()) {
        // 没有破译者时重置失误计时，避免空闲时也检测失误
        m_mistakeTimer = 0;
        return;
    }

    // 更新破译进度
    updateDecodeProgress();

    // 破译失误检测（每5秒一次，20%概率）
    if (m_mistakeTimer <= 0) {
        checkMistake();
        m_mistakeTimer = MISTAKE_CHECK_INTERVAL;
    } else {
        m_mistakeTimer--;
    }
}

void CipherMachine::updateDecodeProgress()
{
    qreal speed = currentDecodeSpeed();
    m_progress += static_cast<int>(speed * 100); // 百分比累加

    if (m_progress >= 100) {
        m_progress = 100;
        emit progressChanged(m_progress);
        emit decodeCompleted();

        // 破译完成，强制所有破译者停止破译
        for (Survivor *s : m_decoders) {
            s->stopDecoding();
        }
        m_decoders.clear();
    } else {
        emit progressChanged(m_progress);
    }
}

qreal CipherMachine::currentDecodeSpeed() const
{
    if (m_decoders.isEmpty()) return 0.0;

    // 基础破译速度：每帧增加的进度百分比 = 100% / (45秒 * 60帧)
    const qreal baseSpeedPerFrame = 100.0 / (GameConfig::SINGLE_CIPHER_DECODE_TIME * 60.0);

    // 计算总效率倍率（每个求生者按其自身破译倍率）
    qreal totalMultiplier = 0.0;
    for (const Survivor *s : m_decoders) {
        totalMultiplier += s->decodeSpeedMultiplier(); // 受伤时0.85，正常1.0
    }

    // 多人破译加速：第一个人按基础，从第二人开始每人增加30%的基础速度
    if (m_decoders.size() >= 2) {
        totalMultiplier += (m_decoders.size() - 1) * 0.3; // 文档：每增加1人提速30%
    }

    return baseSpeedPerFrame * totalMultiplier;
}

void CipherMachine::checkMistake()
{
    // 20% 概率触发失误
    if (QRandomGenerator::global()->bounded(100) < GameConfig::DECODE_MISTAKE_CHANCE) {
        // 进度倒退5%
        m_progress = qMax(0, m_progress - 5);
        emit progressChanged(m_progress);

        // 暴露所有破译者位置（红色提示持续3秒）
        for (Survivor *s : m_decoders) {
            s->revealPosition(GameConfig::FRAMES_MISTAKE_REVEAL);
            emit mistakeTriggered(s);
        }

        qDebug() << "Cipher mistake! Progress reduced to" << m_progress;
    }
}

QRectF CipherMachine::boundingRect() const
{
    return QRectF(-25, -25, 50, 50);
}

void CipherMachine::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // 未解锁时灰色，破译中绿色，完成后金色
    QColor color;
    if (isCompleted()) {
        color = QColor(255, 215, 0); // 金色
    } else if (!m_decoders.isEmpty()) {
        color = QColor(100, 200, 100); // 破译中绿色
    } else {
        color = QColor(128, 128, 128); // 灰色
    }

    painter->setBrush(color);
    painter->setPen(QPen(Qt::black, 2));
    painter->drawEllipse(boundingRect());

    // 绘制进度条
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 255, 150));
    QRectF progressRect(-25, 15, 50 * m_progress / 100.0, 10);
    painter->drawRect(progressRect);

    // 绘制百分比文字
    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 8));
    painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_progress) + "%");
}