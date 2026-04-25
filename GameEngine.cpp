#include "GameEngine.h"
#include "GameScene.h"
#include "PlayerSurvivor.h"
#include "AISurvivor.h"
#include "Hunter.h"
#include "GameConfig.h"
#include <QGraphicsRectItem>
#include <QDebug>
#include "GameEngine.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include "entities/PlayerSurvivor.h"
#include "entities/Hunter.h"
#include "entities/AISurvivor.h"

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_player(nullptr)
    , m_hunter(nullptr)
    , m_paused(false)
    , m_state(Preparation)
    , m_escapedCount(0)
    , m_rescueCount(0)
    , m_destroyCount(0)
{
    m_scene = new GameScene(this);
    m_scene->setSceneRect(0, 0, GameConfig::MAP_WIDTH, GameConfig::MAP_HEIGHT);
    connect(&m_frameTimer, &QTimer::timeout, this, &GameEngine::gameLoop);
}
QList<CipherMachine*> GameEngine::getCiphers() const
{
    QList<CipherMachine*> list;
    // 如果你的 m_ciphers 中存储的是 CipherMachine*，直接转换返回
    for (const auto& data : m_ciphers) {
        if (auto* cm = dynamic_cast<CipherMachine*>(data.item)) {
            list.append(cm);
        }
    }
    // 如果密码机是直接使用 CipherMachine 对象存储，请相应调整
    return list;
}

QList<Gate*> GameEngine::getGates() const
{
    QList<Gate*> list;
    for (const auto& data : m_gates) {
        if (auto* gate = dynamic_cast<Gate*>(data.item)) {
            list.append(gate);
        }
    }
    return list;
}

GameEngine::~GameEngine()
{
    cleanupGameWorld();
}

void GameEngine::startGame(int survivorType)
{
    cleanupGameWorld();
    createGameWorld(survivorType);

    // 进入准备阶段
    setGameState(Preparation);

    // 启动主循环
    m_frameTimer.start(FRAME_INTERVAL);
    m_paused = false;
}

void GameEngine::gameLoop()
{
    if (m_paused) return;

    // 更新倒计时
    updateCountdown();

    // 更新密码机进度（如果处于破译阶段）
    if (m_state == Decoding) {
        for (auto &cipher : m_ciphers) {
            if (cipher.completed) continue;

            // 检查是否有求生者在附近
            for (auto *survivor : m_aiSurvivors) {
                if (survivor->isEnabled() && !survivor->isEliminated()) {
                    QLineF line(survivor->pos(), cipher.item->pos());
                    if (line.length() <= GameConfig::INTERACT_CIPHER_DIST) {
                        cipher.progress += 1; // 每帧增加 1% （约 45 秒达到 100%）
                        if (cipher.progress >= 100) {
                            cipher.progress = 100;
                            cipher.completed = true;
                            emit cipherProgressChanged(completedCipherCount(), GameConfig::CIPHER_COUNT);
                        }
                    }
                }
            }
        }
        // 检查是否所有密码机完成
        if (completedCipherCount() >= GameConfig::CIPHER_COUNT) {
            // 解锁大门
            for (auto &gate : m_gates) {
                gate.unlocked = true;
            }
            setGameState(Escape);
        }
    }

    // 更新大门开启进度（逃脱阶段）
    if (m_state == Escape) {
        for (auto &gate : m_gates) {
            if (!gate.unlocked || gate.progress >= 100) continue;

            // 检查是否有求生者在附近
            for (auto *survivor : m_aiSurvivors) {
                if (survivor->isEnabled() && !survivor->isEliminated()) {
                    QLineF line(survivor->pos(), gate.item->pos());
                    if (line.length() <= GameConfig::INTERACT_GATE_DIST) {
                        gate.progress += 1; // 每帧增加 1%，约 10 秒到 100%
                        if (gate.progress >= 100) {
                            gate.progress = 100;
                            // 有求生者逃脱
                            survivor->escape();
                            m_escapedCount++;
                            checkVictoryCondition();
                        }
                    }
                }
            }
        }
    }

    // 更新 AI
    updateAI();

    // 更新所有角色移动
    m_scene->advance();
    if (m_player && m_player->isEnabled())
        m_player->updateCharacter();
    if (m_hunter && m_hunter->isEnabled())
        m_hunter->updateCharacter();
    for (auto *ai : m_aiSurvivors) {
        if (ai->isEnabled())
            ai->updateCharacter();
    }

    // 发送时间信号
    emit timeUpdated(remainingTime());
}

void GameEngine::setGameState(GameState state)
{
    if (m_state == state) return;
    m_state = state;
    m_stageTimer.restart();
    emit gameStateChanged(state);

    // 状态切换时的处理
    switch (state) {
    case Preparation:
        // 禁用所有角色
        if (m_player) m_player->setEnabled(false);
        if (m_hunter) m_hunter->setEnabled(false);
        for (auto *ai : m_aiSurvivors) ai->setEnabled(false);
        break;
    case Decoding:
        // 启用所有角色
        if (m_player) m_player->setEnabled(true);
        if (m_hunter) m_hunter->setEnabled(true);
        for (auto *ai : m_aiSurvivors) ai->setEnabled(true);
        break;
    case Escape:
        // 大门已解锁，监管者加速
        if (m_hunter) m_hunter->setSpeedMultiplier(GameConfig::ESCAPE_HUNTER_SPEED_RATIO);
        break;
    case Result:
        m_frameTimer.stop();
        // 判断胜负
        break;
    }
}

void GameEngine::updateCountdown()
{
    qint64 elapsed = m_stageTimer.elapsed();
    bool timeout = false;

    switch (m_state) {
    case Preparation:
        if (elapsed >= PREPARATION_DURATION) timeout = true;
        break;
    case Decoding:
        if (elapsed >= DECODING_DURATION) timeout = true;
        break;
    case Escape:
        if (elapsed >= ESCAPE_DURATION) timeout = true;
        break;
    default:
        return;
    }

    if (timeout) {
        if (m_state == Preparation) {
            setGameState(Decoding);
        } else if (m_state == Decoding) {
            hunterWin(); // 破译超时，监管者胜
        } else if (m_state == Escape) {
            hunterWin(); // 逃脱超时，监管者胜
        }
    }
}

void GameEngine::checkVictoryCondition()
{
    // 至少一人逃脱，求生者胜
    if (m_escapedCount > 0) {
        survivorWin();
        return;
    }
    // 所有求生者被淘汰，监管者胜
    if (aliveSurvivorCount() == 0) {
        hunterWin();
        return;
    }
}

void GameEngine::survivorWin()
{
    if (m_state != Result) {
        setGameState(Result);
        emit gameEnded(true);
    }
}

void GameEngine::hunterWin()
{
    if (m_state != Result) {
        setGameState(Result);
        emit gameEnded(false);
    }
}

int GameEngine::remainingTime() const
{
    qint64 elapsed = m_stageTimer.elapsed();
    switch (m_state) {
    case Preparation: return qMax(0, (int)((PREPARATION_DURATION - elapsed) / 1000));
    case Decoding:    return qMax(0, (int)((DECODING_DURATION - elapsed) / 1000));
    case Escape:      return qMax(0, (int)((ESCAPE_DURATION - elapsed) / 1000));
    default:          return 0;
    }
}

int GameEngine::completedCipherCount() const
{
    int count = 0;
    for (const auto &c : m_ciphers) {
        if (c.completed) count++;
    }
    return count;
}

int GameEngine::aliveSurvivorCount() const
{
    int count = 0;
    if (m_player && !m_player->isEliminated()) count++;
    for (auto *ai : m_aiSurvivors) {
        if (!ai->isEliminated()) count++;
    }
    return count;
}

void GameEngine::pauseGame()
{
    m_paused = true;
    m_frameTimer.stop();
}

void GameEngine::resumeGame()
{
    m_paused = false;
    m_frameTimer.start(FRAME_INTERVAL);
}

void GameEngine::restartGame()
{
    // 需要重新选择角色，这里简单重新开始同样的角色
    // 实际使用时通过信号让外部重启
    cleanupGameWorld();
    // 此处需要外部传入角色类型，简化处理
}

void GameEngine::exitToMainMenu()
{
    m_frameTimer.stop();
    cleanupGameWorld();
    // 发出信号让外部切换界面
}

// ---------- 游戏世界创建 ----------
void GameEngine::createGameWorld(int survivorType)
{
    // 创建玩家
    m_player = new PlayerSurvivor(static_cast<SurvivorType>(survivorType));
    m_player->setPos(GameConfig::getSurvivorSpawnPoint(0));
    m_scene->addItem(m_player);
    m_scene->setPlayerCharacter(m_player);

    // 创建监管者
    m_hunter = new Hunter();
    m_hunter->setPos(GameConfig::getHunterSpawnPoint());
    m_scene->addItem(m_hunter);

    // 创建 AI 求生者（剩下的两种类型）
    QList<SurvivorType> aiTypes;
    if (survivorType != 0) aiTypes.append(SurvivorType::Doctor);
    if (survivorType != 1) aiTypes.append(SurvivorType::Mechanic);
    if (survivorType != 2) aiTypes.append(SurvivorType::AirForce);
    for (int i = 0; i < aiTypes.size(); ++i) {
        AISurvivor *ai = new AISurvivor(aiTypes[i]);
        ai->setPos(GameConfig::getSurvivorSpawnPoint(i + 1));
        m_scene->addItem(ai);
        m_aiSurvivors.append(ai);
    }

    // 创建密码机（3个）
    QList<QPointF> cipherPos = GameConfig::getCipherPositions();
    for (const QPointF &pos : cipherPos) {
        auto *item = m_scene->addRect(QRectF(-15, -15, 30, 30), QPen(Qt::black), QBrush(Qt::gray));
        item->setPos(pos);
        CipherData cd;
        cd.item = item;
        cd.progress = 0;
        cd.completed = false;
        m_ciphers.append(cd);
    }

    // 创建大门（2个，初始锁定）
    QList<QPointF> gatePos = GameConfig::getGatePositions();
    for (const QPointF &pos : gatePos) {
        auto *item = m_scene->addRect(QRectF(-20, -30, 40, 60), QPen(Qt::black), QBrush(Qt::darkRed));
        item->setPos(pos);
        GateData gd;
        gd.item = item;
        gd.progress = 0;
        gd.unlocked = false;
        m_gates.append(gd);
    }

    // 重置统计
    m_escapedCount = 0;
    m_rescueCount = 0;
    m_destroyCount = 0;
}

void GameEngine::cleanupGameWorld()
{
    m_frameTimer.stop();
    if (m_scene) {
        m_scene->clear();
    }
    m_player = nullptr;
    m_hunter = nullptr;
    m_aiSurvivors.clear();
    m_ciphers.clear();
    m_gates.clear();
}

// ---------- 简易 AI ----------
void GameEngine::updateAI()
{
    // 监管者巡逻逻辑（简化：随机移动）
    if (m_hunter && m_hunter->isEnabled() && !m_hunter->isStunned()) {
        // 这里可以后续替换为完整的 HunterBehavior
        // 暂时让监管者不动或简单随机移动
    }

    // AI 求生者简单行为：朝着最近的密码机移动
    for (auto *ai : m_aiSurvivors) {
        if (!ai->isEnabled() || ai->isEliminated()) continue;

        // 找到最近未完成的密码机
        CipherData *target = nullptr;
        qreal minDist = 1e9;
        for (auto &cipher : m_ciphers) {
            if (!cipher.completed) {
                qreal d = QLineF(ai->pos(), cipher.item->pos()).length();
                if (d < minDist) {
                    minDist = d;
                    target = &cipher;
                }
            }
        }
        if (target) {
            QPointF dir = target->item->pos() - ai->pos();
            if (dir.x() > 0) ai->setMoveDirection(Direction::Right);
            else if (dir.x() < 0) ai->setMoveDirection(Direction::Left);
            else if (dir.y() > 0) ai->setMoveDirection(Direction::Down);
            else if (dir.y() < 0) ai->setMoveDirection(Direction::Up);
            else ai->setMoveDirection(Direction::None);
        }
    }
}