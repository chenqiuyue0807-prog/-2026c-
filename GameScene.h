#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QObject>
#include <QList>
#include <QPointF>
#include <QPen>
#include <QBrush>
#include <QColor>
#include <QPixmap>
#include "GameConfig.h"

class Character;

// ---------- Obstacle 内联实现 ----------
class Obstacle : public QGraphicsRectItem
{
public:
    enum Type { Wall, Box, Board };
    inline explicit Obstacle(const QRectF &rect, Type type, QGraphicsItem *parent = nullptr)
        : QGraphicsRectItem(rect, parent), m_type(type)
    {
        setFlag(QGraphicsItem::ItemIsSelectable, false);
        setFlag(QGraphicsItem::ItemIsMovable, false);
        QPixmap wallTexture(":/new/prefix1/images/qiang.png");
        if (!wallTexture.isNull()) {
            setBrush(QBrush(wallTexture));
        } else {
            QColor color;
            switch (type) {
            case Wall:  color = QColor(139, 69, 19); break;
            case Box:   color = QColor(160, 82, 45); break;
            case Board: color = QColor(205, 133, 63); break;
            }
            setBrush(QBrush(color));
        }
        setPen(QPen(Qt::black, 2));
    }
    Type obstacleType() const { return m_type; }
private:
    Type m_type;
};

// ---------- Bush 内联实现 ----------
class Bush : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    inline explicit Bush(const QRectF &rect, QGraphicsItem *parent = nullptr)
        : QObject(nullptr), QGraphicsRectItem(rect, parent), m_containsSurvivor(false)
    {
        setFlag(QGraphicsItem::ItemIsSelectable, false);
        setFlag(QGraphicsItem::ItemIsMovable, false);
        QPixmap bushTexture(":/new/prefix1/images/cao.png");
        if (!bushTexture.isNull()) {
            setBrush(QBrush(bushTexture));
        } else {
            setBrush(QBrush(QColor(34, 139, 34, 180)));
        }
        setPen(Qt::NoPen);
    }
    void shake();
    void setContainsSurvivor(bool contains);
    bool containsSurvivor() const { return m_containsSurvivor; }
private:
    bool m_containsSurvivor = false;
};

// ---------- GameScene ----------
class GameScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit GameScene(QObject *parent = nullptr);

    void createObstaclesAndBushes();
    const QList<QGraphicsItem*>& getObstacles() const { return m_obstacles; }
    const QList<Bush*>& getBushes() const { return m_bushes; }

    bool isPointInsideObstacle(const QPointF &point) const;
    bool lineIntersectsObstacle(const QPointF &p1, const QPointF &p2) const;
    Bush* getBushAt(const QPointF &point, qreal maxDistance = 10.0) const;
    QList<Bush*> getBushesInRadius(const QPointF &center, qreal radius) const;
    QList<QGraphicsItem*> getObstaclesInRadius(const QPointF &center, qreal radius) const;

    void setPlayerCharacter(Character *player);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void addObstacle(const QPointF &pos, const QSizeF &size, Obstacle::Type type);
    void addBush(const QPointF &pos, const QSizeF &size);

    QList<QGraphicsItem*> m_obstacles;
    QList<Bush*> m_bushes;
    Character *m_playerCharacter = nullptr;
};

#endif // GAMESCENE_H