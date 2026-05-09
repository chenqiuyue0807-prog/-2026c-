#include "GameScene.h"
#include "entities/Character.h"
#include <QKeyEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include "entities/PlayerSurvivor.h"
#include "entities/Hunter.h"
#include "entities/AISurvivor.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"

void Bush::shake() { return; }
void Bush::setContainsSurvivor(bool contains) { m_containsSurvivor = contains; }

GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent), m_playerCharacter(nullptr)
{
    setSceneRect(0, 0, 2400, 1800);
    QPixmap bgPixmap(":/new/prefix1/images/lumain.png");
    if (!bgPixmap.isNull()) {
        setBackgroundBrush(QBrush(bgPixmap.scaled(2400, 1800, Qt::IgnoreAspectRatio, Qt::SmoothTransformation)));
    } else {
        setBackgroundBrush(QColor(50, 80, 50));
    }
}

// GameScene.cpp 的 createObstaclesAndBushes() 完整替换
void GameScene::createObstaclesAndBushes()
{
    // ================= 1. 墙壁：构建地图分区和走廊 =================
    // 左上角“L”型房间（保留）
    addObstacle(QPointF(150, 150), QSizeF(200, 20), Obstacle::Wall);  // 上横
    addObstacle(QPointF(150, 150), QSizeF(20, 150), Obstacle::Wall);  // 左竖
    addObstacle(QPointF(330, 200), QSizeF(20, 150), Obstacle::Wall);  // 右竖

    // 左侧新增竖墙，形成走廊
    addObstacle(QPointF(500, 300), QSizeF(20, 250), Obstacle::Wall);
    addObstacle(QPointF(500, 600), QSizeF(180, 20), Obstacle::Wall);

    // 中央区域（原回字形已删除）改为交错横墙
    addObstacle(QPointF(750, 400), QSizeF(300, 20), Obstacle::Wall);
    addObstacle(QPointF(800, 600), QSizeF(20, 300), Obstacle::Wall);
    addObstacle(QPointF(1000, 700), QSizeF(300, 20), Obstacle::Wall);
    addObstacle(QPointF(1200, 500), QSizeF(20, 250), Obstacle::Wall);

    // 右侧“双横走廊”保留，但改成更多分段
    addObstacle(QPointF(1500, 400), QSizeF(400, 20), Obstacle::Wall);
    addObstacle(QPointF(1600, 600), QSizeF(20, 200), Obstacle::Wall);
    addObstacle(QPointF(1800, 800), QSizeF(400, 20), Obstacle::Wall);

    // 右侧下方新增长墙和隔断
    addObstacle(QPointF(1500, 1100), QSizeF(300, 20), Obstacle::Wall);
    addObstacle(QPointF(1700, 1000), QSizeF(20, 200), Obstacle::Wall);

    // 左下方“U”型通道（保留）
    addObstacle(QPointF(200, 1200), QSizeF(150, 20), Obstacle::Wall);
    addObstacle(QPointF(200, 1200), QSizeF(20, 200), Obstacle::Wall);
    addObstacle(QPointF(350, 1400), QSizeF(20, 200), Obstacle::Wall);

    // 左下方新增L墙
    addObstacle(QPointF(500, 1300), QSizeF(100, 20), Obstacle::Wall);
    addObstacle(QPointF(600, 1200), QSizeF(20, 100), Obstacle::Wall);

    // 右下角小隔间（保留）
    addObstacle(QPointF(2000, 1400), QSizeF(200, 20), Obstacle::Wall);
    addObstacle(QPointF(2000, 1400), QSizeF(20, 200), Obstacle::Wall);
    addObstacle(QPointF(2180, 1500), QSizeF(20, 150), Obstacle::Wall);

    // 顶部中央独立短墙（保留）
    addObstacle(QPointF(1100, 150), QSizeF(200, 20), Obstacle::Wall);

    // 底部中央长墙改为两条错开
    addObstacle(QPointF(800, 1600), QSizeF(300, 20), Obstacle::Wall);
    addObstacle(QPointF(1300, 1600), QSizeF(300, 20), Obstacle::Wall);

    // 增加新墙：右上角隔断
    addObstacle(QPointF(1900, 200), QSizeF(20, 150), Obstacle::Wall);
    addObstacle(QPointF(1800, 200), QSizeF(150, 20), Obstacle::Wall);

    // 增加新墙：左下角额外
    addObstacle(QPointF(100, 1000), QSizeF(20, 200), Obstacle::Wall);
    addObstacle(QPointF(150, 1000), QSizeF(100, 20), Obstacle::Wall);

    // ================= 2. 箱子：散布在各处作为临时掩体 =================
    addObstacle(QPointF(300, 400), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(650, 200), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(900, 300), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(1300, 350), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(1800, 500), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(2200, 700), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(1900, 1100), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(1400, 1300), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(800, 1400), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(400, 900), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(700, 1000), QSizeF(30, 30), Obstacle::Box);
    addObstacle(QPointF(1600, 1400), QSizeF(30, 30), Obstacle::Box);

    // ================= 3. 板区：半掩体 =================
    addObstacle(QPointF(600, 500), QSizeF(50, 15), Obstacle::Board);
    addObstacle(QPointF(1200, 900), QSizeF(15, 50), Obstacle::Board);
    addObstacle(QPointF(2000, 600), QSizeF(50, 15), Obstacle::Board);
    addObstacle(QPointF(1500, 1500), QSizeF(15, 50), Obstacle::Board);
    addObstacle(QPointF(350, 1200), QSizeF(15, 50), Obstacle::Board);
    addObstacle(QPointF(1800, 1300), QSizeF(50, 15), Obstacle::Board);

    // ================= 4. 草丛：安全躲藏点（适当增加） =================
    addBush(QPointF(700, 150), QSizeF(120, 120));
    addBush(QPointF(1600, 700), QSizeF(130, 130));
    addBush(QPointF(200, 900), QSizeF(100, 100));
    addBush(QPointF(1200, 1200), QSizeF(140, 140));
    addBush(QPointF(2200, 1300), QSizeF(130, 130));
    addBush(QPointF(800, 800), QSizeF(120, 120));
    addBush(QPointF(1800, 1600), QSizeF(110, 110));
    addBush(QPointF(300, 1600), QSizeF(120, 120));
    addBush(QPointF(1900, 200), QSizeF(130, 130));
    addBush(QPointF(1400, 300), QSizeF(100, 100));
    addBush(QPointF(500, 1300), QSizeF(120, 120));
    addBush(QPointF(1000, 1600), QSizeF(110, 110));
    addBush(QPointF(2100, 500), QSizeF(100, 100));
    addBush(QPointF(1600, 1100), QSizeF(130, 130));
    // 新增草丛
    addBush(QPointF(50, 700), QSizeF(110, 110));
    addBush(QPointF(1300, 700), QSizeF(100, 100));
    addBush(QPointF(2200, 100), QSizeF(120, 120));
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
        if (d < minDist) { minDist = d; closest = bush; }
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
    qDebug() << "KEY:" << event->key();   // 临时诊断
    if (m_playerCharacter) {
        m_playerCharacter->handleKeyPress(event);
        event->accept();     // 关键：阻止事件继续传播
        return;
    }
    QGraphicsScene::keyPressEvent(event);
}

void GameScene::keyReleaseEvent(QKeyEvent *event)
{
    if (m_playerCharacter)
        m_playerCharacter->handleKeyRelease(event);
}