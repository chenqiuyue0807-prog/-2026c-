#ifndef MATHHELPER_H
#define MATHHELPER_H

#include <QPointF>
#include <QLineF>
#include "GameConfig.h" // for Direction

class MathHelper
{
public:
    static constexpr qreal PI = 3.14159265358979323846;
    static constexpr qreal RAD_TO_DEG = 180.0 / PI;
    static constexpr qreal DEG_TO_RAD = PI / 180.0;

    static qreal distance(const QPointF &p1, const QPointF &p2);
    static qreal distanceSquared(const QPointF &p1, const QPointF &p2);
    static QPointF vector(const QPointF &from, const QPointF &to);
    static qreal dotProduct(const QPointF &v1, const QPointF &v2);
    static qreal crossProduct(const QPointF &v1, const QPointF &v2);
    static QPointF normalize(const QPointF &vector);
    static qreal length(const QPointF &vector);
    static qreal lengthSquared(const QPointF &vector);

    static qreal angleBetweenRad(const QPointF &from, const QPointF &to);
    static qreal angleBetweenDeg(const QPointF &from, const QPointF &to);
    static qreal angleToDirectionRad(Direction dir);
    static qreal angleToDirectionDeg(Direction dir);

    static qreal normalizeAngleRad(qreal rad);
    static qreal normalizeAngleDeg(qreal deg);
    static qreal angleDifferenceRad(qreal a1, qreal a2);
    static qreal angleDifferenceDeg(qreal a1, qreal a2);

    static QPointF rotatePoint(const QPointF &point, const QPointF &center, qreal angleDeg);
    static qreal lerp(qreal a, qreal b, qreal t);
    static QPointF lerpPoint(const QPointF &p1, const QPointF &p2, qreal t);
    static bool inRange(qreal value, qreal min, qreal max);
    static QPointF angleDegToDirection(qreal angleDeg);
};

#endif