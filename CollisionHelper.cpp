#include "CollisionHelper.h"
#include <QtMath>

bool CollisionHelper::circleIntersectsRect(const QPointF &c, qreal r, const QRectF &rect) {
    qreal cx = qMax(rect.left(), qMin(c.x(), rect.right()));
    qreal cy = qMax(rect.top(), qMin(c.y(), rect.bottom()));
    qreal dx = c.x()-cx, dy = c.y()-cy;
    return dx*dx+dy*dy <= r*r;
}
bool CollisionHelper::circleIntersectsCircle(const QPointF &c1, qreal r1, const QPointF &c2, qreal r2) {
    qreal dx = c1.x()-c2.x(), dy = c1.y()-c2.y();
    return dx*dx+dy*dy <= (r1+r2)*(r1+r2);
}
bool CollisionHelper::pointInRect(const QPointF &p, const QRectF &r) { return r.contains(p); }
bool CollisionHelper::pointInCircle(const QPointF &p, const QPointF &c, qreal r) {
    qreal dx=p.x()-c.x(), dy=p.y()-c.y(); return dx*dx+dy*dy <= r*r;
}
bool CollisionHelper::lineIntersectsRect(const QLineF &line, const QRectF &rect) {
    if (rect.contains(line.p1()) || rect.contains(line.p2())) return true;
    QLineF top(rect.topLeft(), rect.topRight()), bottom(rect.bottomLeft(), rect.bottomRight());
    QLineF left(rect.topLeft(), rect.bottomLeft()), right(rect.topRight(), rect.bottomRight());
    QPointF p;
    return (line.intersects(top,&p)==QLineF::BoundedIntersection) ||
           (line.intersects(bottom,&p)==QLineF::BoundedIntersection) ||
           (line.intersects(left,&p)==QLineF::BoundedIntersection) ||
           (line.intersects(right,&p)==QLineF::BoundedIntersection);
}
bool CollisionHelper::lineIntersectsCircle(const QLineF &line, const QPointF &center, qreal radius) {
    QPointF p1 = line.p1(), p2 = line.p2(), d = p2-p1, f = p1-center;
    qreal a = d.x()*d.x()+d.y()*d.y(), b = 2*(f.x()*d.x()+f.y()*d.y()), c = f.x()*f.x()+f.y()*f.y()-radius*radius;
    qreal disc = b*b-4*a*c;
    if (disc < 0) return false;
    disc = sqrt(disc);
    qreal t1 = (-b - disc)/(2*a), t2 = (-b + disc)/(2*a);
    return (t1>=0 && t1<=1) || (t2>=0 && t2<=1) || pointInCircle(p1,center,radius) || pointInCircle(p2,center,radius);
}
bool CollisionHelper::lineIntersectsAnyPolygon(const QLineF &line, const QList<QPolygonF> &polygons) {
    for (const QPolygonF &poly : polygons)
        if (lineIntersectsRect(line, poly.boundingRect())) return true;
    return false;
}
qreal CollisionHelper::distance(const QPointF &p1, const QPointF &p2) { return QLineF(p1,p2).length(); }
qreal CollisionHelper::angleBetween(const QPointF &from, const QPointF &to) { return atan2(to.y()-from.y(), to.x()-from.x()); }
qreal CollisionHelper::angleDifference(qreal a1, qreal a2) { qreal d = fabs(a1-a2); return d > 180.0 ? 360.0 - d : d; }
qreal CollisionHelper::normalizeAngle(qreal deg) { while (deg<0) deg+=360; while (deg>=360) deg-=360; return deg; }
QPointF CollisionHelper::rectCenter(const QRectF &r) { return r.center(); }