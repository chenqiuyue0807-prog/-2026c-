#include "MiniMapWidget.h"
#include "GameEngine.h"
#include "entities/PlayerSurvivor.h"
#include "entities/AISurvivor.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include <QPainter>
#include <QDebug>

MiniMapWidget::MiniMapWidget(QWidget *parent) : QWidget(parent), m_timer(new QTimer(this))
{
    setFixedSize(MW + 10, MH + 30);
    setStyleSheet("background:rgba(0,0,0,120);border:1px solid gray;border-radius:5px;");

    connect(m_timer, &QTimer::timeout, this, &MiniMapWidget::refresh);
    m_timer->setInterval(250);   // 每250ms刷新，减少性能消耗
}

void MiniMapWidget::setGameEngine(GameEngine *e) { m_engine = e; }
void MiniMapWidget::startUpdate() { m_timer->start(); }
void MiniMapWidget::stopUpdate() { m_timer->stop(); }
void MiniMapWidget::refresh() { update(); }

void MiniMapWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, false);

    // 绘制标题
    p.setPen(Qt::white);
    p.setFont(QFont("Arial", 8));
    p.drawText(QRect(0, 0, width(), 15), Qt::AlignCenter, "小地图");

    // 平移坐标系到绘制区域（标题下方）
    p.save();
    p.translate(5, 20);
    p.setClipRect(0, 0, MW, MH);

    // 背景
    p.fillRect(0, 0, MW, MH, QColor(30, 30, 30, 200));

    if (!m_engine) {
        p.restore();
        return;
    }

    // 绘制网格（可选，便于观察比例）
    p.setPen(QPen(QColor(80, 80, 80), 1, Qt::DotLine));
    for (int i = 0; i <= 4; ++i) {
        int x = i * MW / 4;
        p.drawLine(x, 0, x, MH);
    }
    for (int i = 0; i <= 4; ++i) {
        int y = i * MH / 4;
        p.drawLine(0, y, MW, y);
    }

    // 密码机
    for (auto *c : m_engine->getCiphers()) {
        QPointF mp = worldToMini(c->pos());
        QColor col = c->isCompleted() ? QColor(255, 215, 0) : QColor(128, 128, 128);
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawEllipse(mp, 3, 3);   // 稍微放大，更显眼
    }

    // 大门
    for (auto *g : m_engine->getGates()) {
        QPointF mp = worldToMini(g->pos());
        QColor col;
        if (g->isFullyOpen()) col = Qt::green;
        else if (g->isUnlocked()) col = Qt::yellow;
        else col = Qt::darkGray;
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawRect(mp.x() - 3, mp.y() - 5, 6, 10);
    }

    // 队友（AI 幸存者）
    for (auto *ai : m_engine->getAISurvivors()) {
        if (ai->isEliminated()) continue;
        QPointF mp = worldToMini(ai->pos());
        QColor col = ai->isBurning() ? Qt::red : (ai->isHurt() ? QColor(255, 165, 0) : Qt::green);
        p.setBrush(col);
        p.setPen(Qt::NoPen);
        p.drawEllipse(mp, 3, 3);
    }

    // 玩家
    if (auto *pl = m_engine->getPlayer()) {
        if (!pl->isEliminated()) {
            QPointF mp = worldToMini(pl->pos());
            QColor col = pl->isBurning() ? Qt::red : (pl->isHurt() ? QColor(255, 165, 0) : Qt::green);
            p.setBrush(col);
            p.setPen(QPen(Qt::white, 2));   // 白边突出玩家
            p.drawEllipse(mp, 4, 4);
        }
    }

    p.restore();
}

QPointF MiniMapWidget::worldToMini(const QPointF &wp) const
{
    return QPointF(wp.x() * SCALE, wp.y() * SCALE);
}