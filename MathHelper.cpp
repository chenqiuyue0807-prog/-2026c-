#include "MathHelper.h"
#include <QtMath>

qreal MathHelper::distance(const QPointF &p1, const QPointF &p2) { return QLineF(p1, p2).length(); }
qreal MathHelper::distanceSquared(const QPointF &p1, const QPointF &p2) { qreal dx=p1.x()-p2.x(), dy=p1.y()-p2.y(); return dx*dx+dy*dy; }
QPointF MathHelper::vector(const QPointF &from, const QPointF &to) { return to - from; }
qreal MathHelper::dotProduct(const QPointF &v1, const QPointF &v2) { return v1.x()*v2.x() + v1.y()*v2.y(); }
qreal MathHelper::crossProduct(const QPointF &v1, const QPointF &v2) { return v1.x()*v2.y() - v1.y()*v2.x(); }
QPointF MathHelper::normalize(const QPointF &v) { qreal len = length(v); return len < 1e-6 ? QPointF(0,0) : v/len; }
qreal MathHelper::length(const QPointF &v) { return sqrt(v.x()*v.x()+v.y()*v.y()); }
qreal MathHelper::lengthSquared(const QPointF &v) { return v.x()*v.x()+v.y()*v.y(); }

qreal MathHelper::angleBetweenRad(const QPointF &from, const QPointF &to) { return atan2(to.y()-from.y(), to.x()-from.x()); }
qreal MathHelper::angleBetweenDeg(const QPointF &from, const QPointF &to) { return normalizeAngleDeg(angleBetweenRad(from, to) * RAD_TO_DEG); }
qreal MathHelper::angleToDirectionRad(Direction dir) {
    switch (dir) {
    case Direction::Right: return 0.0;
    case Direction::Down:  return PI / 2.0;
    case Direction::Left:  return PI;
    case Direction::Up:    return PI * 1.5;
    default: return 0.0;
    }
}
qreal MathHelper::angleToDirectionDeg(Direction dir) {
    switch (dir) {
    case Direction::Right: return 0.0;
    case Direction::Down:  return 90.0;
    case Direction::Left:  return 180.0;
    case Direction::Up:    return 270.0;
    default: return 0.0;
    }
}
qreal MathHelper::normalizeAngleRad(qreal rad) { while (rad > PI) rad -= 2*PI; while (rad < -PI) rad += 2*PI; return rad; }
qreal MathHelper::normalizeAngleDeg(qreal deg) { while (deg < 0) deg += 360; while (deg >= 360) deg -= 360; return deg; }
qreal MathHelper::angleDifferenceRad(qreal a1, qreal a2) { qreal d = fabs(a1-a2); return d > PI ? 2*PI - d : d; }
qreal MathHelper::angleDifferenceDeg(qreal a1, qreal a2) { qreal d = fabs(a1-a2); return d > 180 ? 360 - d : d; }

QPointF MathHelper::rotatePoint(const QPointF &p, const QPointF &c, qreal angDeg) {
    qreal rad = angDeg * DEG_TO_RAD;
    qreal s = sin(rad), cs = cos(rad);
    qreal dx = p.x()-c.x(), dy = p.y()-c.y();
    return QPointF(c.x() + dx*cs - dy*s, c.y() + dx*s + dy*cs);
}
qreal MathHelper::lerp(qreal a, qreal b, qreal t) { return a + (b-a)*t; }
QPointF MathHelper::lerpPoint(const QPointF &p1, const QPointF &p2, qreal t) { return QPointF(lerp(p1.x(),p2.x(),t), lerp(p1.y(),p2.y(),t)); }
bool MathHelper::inRange(qreal v, qreal min, qreal max) { return v >= min && v <= max; }
QPointF MathHelper::angleDegToDirection(qreal deg) { qreal rad = deg * DEG_TO_RAD; return QPointF(cos(rad), sin(rad)); }