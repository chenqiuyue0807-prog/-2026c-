#include "MiniMapWidget.h"
#include "GameEngine.h"
#include "entities/PlayerSurvivor.h"
#include "entities/AISurvivor.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include <QPainter>

MiniMapWidget::MiniMapWidget(QWidget *parent) : QWidget(parent), m_timer(new QTimer(this))
{
    setFixedSize(MW+10, MH+30);
    setStyleSheet("background:rgba(0,0,0,120);border:1px solid gray;border-radius:5px;");
    connect(m_timer, &QTimer::timeout, this, &MiniMapWidget::refresh);
    m_timer->setInterval(100);
}

void MiniMapWidget::setGameEngine(GameEngine *e) { m_engine = e; }
void MiniMapWidget::startUpdate() { m_timer->start(); }
void MiniMapWidget::stopUpdate() { m_timer->stop(); }
void MiniMapWidget::refresh() { update(); }

void MiniMapWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(Qt::white); p.setFont(QFont("Arial",8));
    p.drawText(QRect(0,0,width(),15), Qt::AlignCenter, "小地图");
    p.save(); p.translate(5,20);
    p.fillRect(0,0,MW,MH, QColor(30,30,30,200));

    if (!m_engine) { p.restore(); return; }

    // 密码机
    for (auto *c : m_engine->getCiphers()) {
        QPointF mp = worldToMini(c->pos());
        p.setBrush(c->isCompleted() ? Qt::yellow : Qt::gray);
        p.drawEllipse(mp, 2, 2);
    }
    // 大门
    for (auto *g : m_engine->getGates()) {
        QPointF mp = worldToMini(g->pos());
        p.setBrush(g->isUnlocked() ? (g->isFullyOpen() ? Qt::green : Qt::yellow) : Qt::darkGray);
        p.drawRect(mp.x()-2, mp.y()-3, 4, 6);
    }
    // AI队友
    for (auto *ai : m_engine->getAISurvivors()) {
        if (ai->isEliminated()) continue;
        QPointF mp = worldToMini(ai->pos());
        QColor col = ai->isBurning() ? Qt::red : (ai->isHurt() ? Qt::yellow : Qt::green);
        p.setBrush(col); p.drawEllipse(mp, 2, 2);
    }
    // 玩家
    if (auto *pl = m_engine->getPlayer()) {
        if (!pl->isEliminated()) {
            QPointF mp = worldToMini(pl->pos());
            p.setBrush(pl->isBurning() ? Qt::red : (pl->isHurt() ? Qt::yellow : Qt::green));
            p.setPen(QPen(Qt::white, 1));
            p.drawEllipse(mp, 3, 3);
        }
    }
    p.restore();
}

QPointF MiniMapWidget::worldToMini(const QPointF &wp) const { return QPointF(wp.x()*SCALE, wp.y()*SCALE); }