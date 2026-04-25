#ifndef GAMESCENE_H
#define GAMESCENE_H

#include <QGraphicsScene>
#include <QList>
#include <QPointF>
#include <QGraphicsItem>
#include "GameConfig.h"

// 前向声明
class Character;
class Obstacle;
class Bush;

// 简单的障碍物类

class Obstacle : public QGraphicsRectItem
{
public:
    enum Type { Wall, Box, Board };
    explicit Obstacle(const QRectF &rect, Type type, QGraphicsItem *parent = nullptr);
    Type obstacleType() const { return m_type; }
private:
    Type m_type;
};

// 草丛类
class Bush : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    explicit Bush(const QRectF &rect, QGraphicsItem *parent = nullptr);
    void shake();
    void setContainsSurvivor(bool contains);
    bool containsSurvivor() const { return m_containsSurvivor; }
private:
    bool m_containsSurvivor;
};

class GameScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GameScene(QObject *parent = nullptr);

    // 创建地图固定元素（障碍物、草丛）——由 GameEngine 初始化时调用
    void createObstaclesAndBushes();

    // 获取所有障碍物（用于碰撞检测）
    const QList<QGraphicsItem*>& getObstacles() const { return m_obstacles; }

    // 获取所有草丛（用于晃动、隐身等逻辑）
    const QList<Bush*>& getBushes() const { return m_bushes; }

    // 检查点是否在障碍物内
    bool isPointInsideObstacle(const QPointF &point) const;

    // 线段与障碍物是否相交（用于攻击射线检测）
    bool lineIntersectsObstacle(const QPointF &p1, const QPointF &p2) const;

    // 获取指定点附近的草丛
    Bush* getBushAt(const QPointF &point, qreal maxDistance = 10.0) const;

    // 获取指定范围内的所有草丛
    QList<Bush*> getBushesInRadius(const QPointF &center, qreal radius) const;

    // 设置玩家角色（用于键盘事件转发）
    void setPlayerCharacter(Character *player);

    // 获取指定圆心、半径范围内的所有障碍物
    QList<QGraphicsItem*> getObstaclesInRadius(const QPointF &center, qreal radius) const;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:
    void addObstacle(const QPointF &pos, const QSizeF &size, Obstacle::Type type);
    void addBush(const QPointF &pos, const QSizeF &size);

    QList<QGraphicsItem*> m_obstacles;   // 障碍物列表（墙壁、箱子、板区）
    QList<Bush*> m_bushes;               // 草丛列表

    Character *m_playerCharacter;        // 玩家角色指针（用于事件转发）
};


#endif // GAMESCENE_H