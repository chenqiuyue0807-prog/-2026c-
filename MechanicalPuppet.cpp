#include "MechanicalPuppet.h"
#include "GameConfig.h"
#include "CipherMachine.h"
#include <QGraphicsScene>
#include <QDebug>
#include <cmath>
#include <QLineF>

MechanicalPuppet::MechanicalPuppet(const QPointF &startPos, QGraphicsScene *scene, QObject *parent)
    : QObject(parent), QGraphicsRectItem(), m_scene(scene), m_moving(false)
{
    setRect(-15, -15, 30, 30);          // 固定小尺寸，保持不变
    setPos(startPos);

    m_pixmap.load(":/new/prefix2/images/xz.png");
    // 删除下面这两行，它们会把碰撞体积恢复成图片原始大小
    // if (!m_pixmap.isNull()) {
    //     setRect(-m_pixmap.width()/2, -m_pixmap.height()/2, m_pixmap.width(), m_pixmap.height());
    // }

    if (m_scene) m_scene->addItem(this);

    // 计算破译速度...
    qreal baseSpeed = 100.0 / (GameConfig::SINGLE_CIPHER_DECODE_TIME * 60.0);
    m_decodeSpeed = baseSpeed * GameConfig::PUPPET_DECODE_SPEED_RATIO;

    connect(&m_timer, &QTimer::timeout, this, &MechanicalPuppet::onTimeout);
    m_timer.start(15000);
}

MechanicalPuppet::~MechanicalPuppet()
{
    if (m_scene) m_scene->removeItem(this);
}

void MechanicalPuppet::moveToTarget(const QPointF &target)
{
    m_target = target;
    m_moving = true;
}

void MechanicalPuppet::destroy()
{
    if (!m_destroyed) {
        m_destroyed = true;
        if (m_scene) m_scene->removeItem(this);
        deleteLater();
    }
}

void MechanicalPuppet::advance(int phase)
{
    if (!phase) return;

    // 如果正在破译
    if (m_decoding) {
        if (m_targetCipher && !m_targetCipher->isCompleted()) {
            m_targetCipher->addPuppetProgress(m_decodeSpeed);
        } else {
            // 密码机已完成或无效，销毁自己
            destroy();
        }
        return;
    }

    // 移动逻辑
    if (m_moving) {
        QPointF dir = m_target - pos();
        qreal len = std::hypot(dir.x(), dir.y());
        if (len < 5.0) {
            m_moving = false;
            // 到达目标，寻找最近密码机并开始破译
            if (m_scene) {
                CipherMachine *best = nullptr;
                qreal minDist = 1e9;
                QList<QGraphicsItem*> items = m_scene->items();
                for (QGraphicsItem *item : items) {
                    CipherMachine *cipher = dynamic_cast<CipherMachine*>(item);
                    if (cipher && !cipher->isCompleted()) {
                        qreal d = QLineF(pos(), cipher->pos()).length();
                        if (d < minDist) {
                            minDist = d;
                            best = cipher;
                        }
                    }
                }
                if (best) {
                    m_targetCipher = best;
                    m_decoding = true;
                }
            }
        } else {
            QPointF step = dir / len * 2.0;
            setPos(pos() + step);
        }
    }
}

void MechanicalPuppet::onTimeout()
{
    destroy();
}

void MechanicalPuppet::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    if (!m_pixmap.isNull()) {
        // 缩放图片至碰撞矩形大小，保持比例
        QPixmap scaled = m_pixmap.scaled(boundingRect().size().toSize(),
                                         Qt::KeepAspectRatio,
                                         Qt::SmoothTransformation);
        // 居中绘制缩放后的图片
        QRectF targetRect = boundingRect();
        QSizeF imgSize = scaled.size();
        qreal x = targetRect.center().x() - imgSize.width() / 2;
        qreal y = targetRect.center().y() - imgSize.height() / 2;
        painter->drawPixmap(QPointF(x, y), scaled);
    } else {
        // 降级：绘制青色矩形（同样使用 boundingRect）
        painter->setBrush(Qt::cyan);
        painter->setPen(QPen(Qt::blue));
        painter->drawRect(boundingRect());
    }
}