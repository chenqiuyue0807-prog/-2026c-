#include "CameraFollow.h"
#include <QGraphicsItem>

CameraFollow::CameraFollow(QObject *parent) : QObject(parent) {}
void CameraFollow::setView(QGraphicsView *v) { m_view = v; if(m_view) m_currentCenter = m_view->mapToScene(m_view->viewport()->rect().center()); }
void CameraFollow::setTarget(QGraphicsItem *t) { m_target = t; if(m_target && m_view) snapToTarget(); }
void CameraFollow::setSmooth(bool s, qreal f) { m_smooth = s; m_factor = qBound(0.01, f, 1.0); }
void CameraFollow::update() {
    if (!m_view || !m_target) return;
    QPointF targetPos = m_target->pos();
    if (!m_smooth) { m_view->centerOn(targetPos); m_currentCenter = targetPos; }
    else { m_currentCenter += (targetPos - m_currentCenter) * m_factor; m_view->centerOn(m_currentCenter); }
}
void CameraFollow::snapToTarget() { if(m_target && m_view) { m_view->centerOn(m_target->pos()); m_currentCenter = m_target->pos(); } }