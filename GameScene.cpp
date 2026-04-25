#include "GameScene.h"
#include "Character.h"
#include <QKeyEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "entities/PlayerSurvivor.h"
#include "entities/Hunter.h"
#include "entities/AISurvivor.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"

// ---------- Obstacle 实现 ----------
Obstacle::Obstacle(const QRectF &rect, Type type, QGraphicsItem *parent)
    : QGraphicsRectItem(rect, parent), m_type(type)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    QColor color;
    switch (type) {
    case Wall:  color = QColor(139, 69, 19); break;
    case Box:   color = QColor(160, 82, 45); break;
    case Board: color = QColor(205, 133, 63); break;
    }
    setBrush(QBrush(color));
    setPen(QPen(Qt::black, 2));
}

// ---------- Bush 实现 ----------
Bush::Bush(const QRectF &rect, QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsRectItem(rect, parent), m_containsSurvivor(false)
{
    setFlag(QGraphicsItem::ItemIsSelectable, false);
    setFlag(QGraphicsItem::ItemIsMovable, false);
    setBrush(QBrush(QColor(34, 139, 34, 180)));
    setPen(Qt::NoPen);
}

void Bush::shake()
{
    // 晃动动画（如需启用请确保 Bush 是 QObject 子类）
    // 当前为空实现以避免编译错误
}

void Bush::setContainsSurvivor(bool contains)
{
    m_containsSurvivor = contains;
}

// ---------- GameScene 实现 ----------
GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), m_playerCharacter(nullptr)
{
    setSceneRect(0, 0, 1200, 800);
    setBackgroundBrush(QColor(50, 80, 50));
}

void GameScene::createObstaclesAndBushes()
{
    // 障碍物
    addObstacle(QPointF(200, 150), QSizeF(80, 20), Obstacle::Wall);
    addObstacle(QPointF(400, 200), QSizeF(20, 120), Obstacle::Wall);
    addObstacle(QPointF(600, 100), QSizeF(100, 20), Obstacle::Wall);
    addObstacle(QPointF(800, 300), QSizeF(20, 150), Obstacle::Wall);
    addObstacle(QPointF(300, 500), QSizeF(150, 20), Obstacle::Wall);
    addObstacle(QPointF(700, 600), QSizeF(80, 20), Obstacle::Wall);
    addObstacle(QPointF(150, 650), QSizeF(20, 100), Obstacle::Wall);
    addObstacle(QPointF(900, 500), QSizeF(20, 150), Obstacle::Wall);
    addObstacle(QPointF(350, 350), QSizeF(40, 40), Obstacle::Box);
    addObstacle(QPointF(750, 400), QSizeF(40, 40), Obstacle::Box);
    addObstacle(QPointF(500, 650), QSizeF(40, 40), Obstacle::Box);
    addObstacle(QPointF(250, 250), QSizeF(60, 15), Obstacle::Board);
    addObstacle(QPointF(650, 250), QSizeF(60, 15), Obstacle::Board);
    addObstacle(QPointF(450, 450), QSizeF(15, 60), Obstacle::Board);

    // 草丛
    addBush(QPointF(100, 100), QSizeF(80, 80));
    addBush(QPointF(500, 150), QSizeF(100, 80));
    addBush(QPointF(850, 200), QSizeF(90, 90));
    addBush(QPointF(200, 450), QSizeF(100, 100));
    addBush(QPointF(600, 500), QSizeF(120, 80));
    addBush(QPointF(900, 650), QSizeF(80, 80));
    addBush(QPointF(300, 700), QSizeF(100, 100));
}

void GameScene::addObstacle(const QPointF &pos, const QSizeF &size, Obstacle::Type type)
{
    QRectF rect(pos, size);
    Obstacle *obs = new Obstacle(rect, type);
    addItem(obs);
    m_obstacles.append(obs);
}

void GameScene::addBush(const QPointF &pos, const QSizeF &size)
{
    QRectF rect(pos, size);
    Bush *bush = new Bush(rect);
    addItem(bush);
    m_bushes.append(bush);
}

bool GameScene::isPointInsideObstacle(const QPointF &point) const
{
    for (QGraphicsItem *item : m_obstacles) {
        if (item->contains(item->mapFromScene(point)))
            return true;
    }
    return false;
}

bool GameScene::lineIntersectsObstacle(const QPointF &p1, const QPointF &p2) const
{
    QLineF line(p1, p2);
    for (QGraphicsItem *item : m_obstacles) {
        QRectF rect = item->sceneBoundingRect();
        QLineF top(rect.topLeft(), rect.topRight());
        QLineF bottom(rect.bottomLeft(), rect.bottomRight());
        QLineF left(rect.topLeft(), rect.bottomLeft());
        QLineF right(rect.topRight(), rect.bottomRight());
        QPointF intersection;
        if (line.intersects(top, &intersection) == QLineF::BoundedIntersection ||
            line.intersects(bottom, &intersection) == QLineF::BoundedIntersection ||
            line.intersects(left, &intersection) == QLineF::BoundedIntersection ||
            line.intersects(right, &intersection) == QLineF::BoundedIntersection) {
            return true;
        }
    }
    return false;
}

Bush* GameScene::getBushAt(const QPointF &point, qreal maxDistance) const
{
    for (Bush *bush : m_bushes) {
        if (bush->contains(bush->mapFromScene(point)))
            return bush;
    }
    Bush *closest = nullptr;
    qreal minDist = maxDistance;
    for (Bush *bush : m_bushes) {
        QPointF center = bush->sceneBoundingRect().center();
        qreal d = QLineF(point, center).length();
        if (d < minDist) {
            minDist = d;
            closest = bush;
        }
    }
    return closest;
}

QList<Bush*> GameScene::getBushesInRadius(const QPointF &center, qreal radius) const
{
    QList<Bush*> result;
    QRectF searchRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    for (Bush *bush : m_bushes) {
        if (bush->sceneBoundingRect().intersects(searchRect))
            result.append(bush);
    }
    return result;
}

QList<QGraphicsItem*> GameScene::getObstaclesInRadius(const QPointF &center, qreal radius) const
{
    QList<QGraphicsItem*> result;
    QRectF searchRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    for (QGraphicsItem *item : m_obstacles) {
        if (item->sceneBoundingRect().intersects(searchRect))
            result.append(item);
    }
    return result;
}

void GameScene::setPlayerCharacter(Character *player)
{
    m_playerCharacter = player;
}

void GameScene::keyPressEvent(QKeyEvent *event)
{
    if (m_playerCharacter)
        m_playerCharacter->handleKeyPress(event);
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (m_playerCharacter)
        m_playerCharacter->handleKeyRelease(event);
    QGraphicsScene::keyReleaseEvent(event);
}