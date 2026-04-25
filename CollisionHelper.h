#ifndef COLLISIONHELPER_H
#define COLLISIONHELPER_H

#include <QPointF>
#include <QRectF>
#include <QLineF>
#include <QPolygonF>
#include <QList>

class CollisionHelper
{
public:
    static bool circleIntersectsRect(const QPointF &circleCenter, qreal radius, const QRectF &rect);
    static bool circleIntersectsCircle(const QPointF &c1, qreal r1, const QPointF &c2, qreal r2);
    static bool pointInRect(const QPointF &point, const QRectF &rect);
    static bool pointInCircle(const QPointF &point, const QPointF &center, qreal radius);
    static bool lineIntersectsRect(const QLineF &line, const QRectF &rect);
    static bool lineIntersectsCircle(const QLineF &line, const QPointF &center, qreal radius);
    static bool lineIntersectsAnyPolygon(const QLineF &line, const QList<QPolygonF> &polygons);
    static qreal distance(const QPointF &p1, const QPointF &p2);
    static qreal angleBetween(const QPointF &from, const QPointF &to);
    static qreal angleDifference(qreal angle1, qreal angle2);
    static qreal normalizeAngle(qreal angleDeg);
    static QPointF rectCenter(const QRectF &rect);
};

#endif