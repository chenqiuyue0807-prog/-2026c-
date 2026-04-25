#include "AIDecisionMaker.h"
#include "GameScene.h"
#include "entities/Hunter.h"
#include "entities/AISurvivor.h"
#include "entities/PlayerSurvivor.h"
#include "GameConfig.h"
#include <QLineF>
#include <QRandomGenerator>
#include <QDebug>

AIDecisionMaker::AIDecisionMaker(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_hunter(nullptr)
    , m_player(nullptr)
{
    m_timer.setInterval(200); // 0.2 秒
    connect(&m_timer, &QTimer::timeout, this, &AIDecisionMaker::makeDecision);
}

AIDecisionMaker::~AIDecisionMaker()
{
    stop();
}

void AIDecisionMaker::start()
{
    m_timer.start();
}

void AIDecisionMaker::stop()
{
    m_timer.stop();
}

void AIDecisionMaker::reset()
{
    stop();
    // 无额外状态需重置
}

void AIDecisionMaker::makeDecision()
{
    // 1. 监管者行为
    updateHunterAI();

    // 2. AI 求生者行为
    for (AISurvivor *survivor : m_aiSurvivors) {
        if (survivor && survivor->isEnabled() && !survivor->isEliminated()) {
            updateSurvivorAI(survivor);
        }
    }
}

// ---------- 简易监管者 AI ----------
void AIDecisionMaker::updateHunterAI()
{
    if (!m_hunter || !m_hunter->isEnabled() || m_hunter->isStunned())
        return;

    // 1. 检查附近是否有求生者（120 像素内且无遮挡）
    Survivor *target = nullptr;
    qreal minDist = GameConfig::AI_HUNTER_CHASE_DIST;
    QList<Survivor*> allSurvivors;
    if (m_player && !m_player->isEliminated())
        allSurvivors.append(m_player);
    for (AISurvivor *ai : m_aiSurvivors) {
        if (!ai->isEliminated())
            allSurvivors.append(ai);
    }

    for (Survivor *s : allSurvivors) {
        if (s->isEliminated()) continue;
        qreal d = QLineF(m_hunter->pos(), s->pos()).length();
        // 简化：无射线检测，直接距离判断
        if (d < minDist) {
            minDist = d;
            target = s;
        }
    }

    // 2. 有目标则追击，否则巡逻最近的密码机
    if (target) {
        QPointF dir = target->pos() - m_hunter->pos();
        if (qAbs(dir.x()) > qAbs(dir.y())) {
            m_hunter->setMoveDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
        } else {
            m_hunter->setMoveDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
        }
    } else {
        // 巡逻到地图中心（简单处理）
        QPointF center(600, 400);
        QPointF dir = center - m_hunter->pos();
        if (dir.manhattanLength() < 30) {
            m_hunter->setMoveDirection(Direction::None);
        } else {
            if (qAbs(dir.x()) > qAbs(dir.y()))
                m_hunter->setMoveDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
            else
                m_hunter->setMoveDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
        }
    }
}

// ---------- 简易求生者 AI ----------
void AIDecisionMaker::updateSurvivorAI(AISurvivor *survivor)
{
    if (!survivor || survivor->isEliminated())
        return;

    // 1. 如果被监管者追击，尝试逃跑（向远离监管者方向移动）
    if (m_hunter) {
        qreal distToHunter = QLineF(survivor->pos(), m_hunter->pos()).length();
        if (distToHunter < 150) {
            // 远离监管者
            QPointF dir = survivor->pos() - m_hunter->pos();
            if (dir.manhattanLength() > 0) {
                if (qAbs(dir.x()) > qAbs(dir.y()))
                    survivor->setMoveDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
                else
                    survivor->setMoveDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
                return;
            }
        }
    }

    // 2. 否则寻找最近未破译的密码机并前往
    // 这里需要访问密码机列表，但 AIDecisionMaker 没有直接持有。
    // 可以通过 GameScene 遍历 item 寻找密码机（由 GameEngine 创建的矩形）。
    // 简易方案：让 AI 向固定坐标移动（地图上的三个密码机位置）
    static const QList<QPointF> cipherPoints = GameConfig::getCipherPositions();
    QPointF target = cipherPoints[0];
    qreal minDist = QLineF(survivor->pos(), target).length();
    for (int i = 1; i < cipherPoints.size(); ++i) {
        qreal d = QLineF(survivor->pos(), cipherPoints[i]).length();
        if (d < minDist) {
            minDist = d;
            target = cipherPoints[i];
        }
    }

    QPointF dir = target - survivor->pos();
    if (dir.manhattanLength() < 20) {
        survivor->setMoveDirection(Direction::None);
    } else {
        if (qAbs(dir.x()) > qAbs(dir.y()))
            survivor->setMoveDirection(dir.x() > 0 ? Direction::Right : Direction::Left);
        else
            survivor->setMoveDirection(dir.y() > 0 ? Direction::Down : Direction::Up);
    }
}