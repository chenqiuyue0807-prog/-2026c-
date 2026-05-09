#include "CipherMachine.h"
#include "Survivor.h"
#include "GameConfig.h"
#include <QPainter>
#include <QRandomGenerator>
#include <QDebug>

CipherMachine::CipherMachine(QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsItem(parent)
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
        m_mistakeTimer = 0;
        return;
    }

    updateDecodeProgress();

    if (m_mistakeTimer <= 0) {
        checkMistake();
        m_mistakeTimer = MISTAKE_CHECK_INTERVAL;
    } else {
        m_mistakeTimer--;
    }
}

void CipherMachine::addPuppetProgress(qreal speed)
{
    if (isCompleted()) return;

    m_progress += speed;
    if (m_progress >= 100.0) {
        m_progress = 100.0;
        emit progressChanged(static_cast<int>(m_progress));
        emit decodeCompleted();
        // 傀儡破译完成时，强制所有正在破译的求生者停止（如果有的话）
        for (Survivor *s : m_decoders)
            s->stopDecoding();
        m_decoders.clear();
    } else {
        emit progressChanged(static_cast<int>(m_progress));
    }
}

void CipherMachine::updateDecodeProgress()
{
    qreal speed = currentDecodeSpeed();          // 每帧增加百分比（0.x）
    m_progress += speed;                         // 直接累加
    if (m_progress >= 100.0) {
        m_progress = 100.0;
        emit progressChanged(static_cast<int>(m_progress));
        emit decodeCompleted();
        for (Survivor *s : m_decoders)
            s->stopDecoding();
        m_decoders.clear();
    } else {
        emit progressChanged(static_cast<int>(m_progress));
    }
}

qreal CipherMachine::currentDecodeSpeed() const
{
    if (m_decoders.isEmpty()) return 0.0;
    const qreal baseSpeedPerFrame = 100.0 / (GameConfig::SINGLE_CIPHER_DECODE_TIME * 60.0);
    qreal totalMultiplier = 0.0;
    for (const Survivor *s : m_decoders)
        totalMultiplier += s->decodeSpeedMultiplier();
    if (m_decoders.size() >= 2)
        totalMultiplier += (m_decoders.size() - 1) * 0.3;
    return baseSpeedPerFrame * totalMultiplier;
}

void CipherMachine::checkMistake()
{
    if (QRandomGenerator::global()->bounded(100) < GameConfig::DECODE_MISTAKE_CHANCE) {
        m_progress = qMax(0.0, m_progress - 5.0);
        emit progressChanged(static_cast<int>(m_progress));
        for (Survivor *s : m_decoders) {
            s->revealPosition(GameConfig::FRAMES_MISTAKE_REVEAL);
            emit mistakeTriggered(s);
        }
        qDebug() << "Cipher mistake! Progress reduced to" << m_progress;
    }
}

QRectF CipherMachine::boundingRect() const {
    return QRectF(-50, -50, 100, 100);   // 从70x70扩大到100x100
}

void CipherMachine::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
    static QPixmap cipherPix(":/new/prefix1/images/mimaji.png");   // 只保留这一个
    QRectF r = boundingRect();
    if (!cipherPix.isNull()) {
        painter->drawPixmap(r.toRect(), cipherPix.scaled(r.size().toSize(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        QColor color;
        if (isCompleted()) color = QColor(255, 215, 0);
        else if (!m_decoders.isEmpty()) color = QColor(100, 200, 100);
        else color = QColor(128, 128, 128);
        painter->setBrush(color);
        painter->setPen(QPen(Qt::black, 2));
        painter->drawEllipse(r);
    }

    // 进度条
    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(0, 0, 255, 150));
    QRectF progressRect(r.left() + 5, r.bottom() - 15, r.width() - 10, 10);
    painter->drawRect(progressRect);

    // 百分比文字
    painter->setPen(Qt::black);
    painter->setFont(QFont("Arial", 10));
    painter->drawText(r, Qt::AlignCenter, QString::number(static_cast<int>(m_progress)) + "%");
}