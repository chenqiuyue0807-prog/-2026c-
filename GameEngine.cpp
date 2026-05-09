#include "GameEngine.h"
#include "GameScene.h"
#include "entities/PlayerSurvivor.h"
#include "entities/AISurvivor.h"
#include "entities/Hunter.h"
#include "entities/CipherMachine.h"
#include "entities/Gate.h"
#include "GameConfig.h"
#include "ai/AIDecisionMaker.h"
#include "ai/HunterBehavior.h"
#include "ai/SurvivorBehavior.h"
#include "utils/AudioManager.h"
#include <QDebug>
#include <QLineF>

GameEngine::GameEngine(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_player(nullptr)
    , m_hunter(nullptr)
    , m_aiDecisionMaker(nullptr)
    , m_hunterBehavior(nullptr)
    , m_survivorBehavior(nullptr)
    , m_paused(false)
    , m_state(Preparation)
    , m_escapedCount(0)
    , m_rescueCount(0)
    , m_destroyCount(0)
    , m_lastSurvivorType(0)
{
    m_scene = new GameScene(this);
    m_scene->setSceneRect(0, 0, GameConfig::MAP_WIDTH, GameConfig::MAP_HEIGHT);
    connect(&m_frameTimer, &QTimer::timeout, this, &GameEngine::gameLoop);
}

GameEngine::~GameEngine()
{
    cleanupGameWorld();
}

void GameEngine::startGame(int survivorType)
{
    m_lastSurvivorType = survivorType;
    cleanupGameWorld();
    createGameWorld(survivorType);

    // ---- 创建行为模块并注入依赖 ----
    m_hunterBehavior = new HunterBehavior(this);
    m_hunterBehavior->setHunter(m_hunter);
    m_hunterBehavior->setScene(m_scene);
    m_hunterBehavior->setCiphers(m_cachedCiphers);
    m_hunterBehavior->setGates(m_cachedGates);
    m_hunterBehavior->setSurvivors(m_aiSurvivors, m_player);
    m_hunterBehavior->setGamePhase(1);   // 破译阶段

    m_survivorBehavior = new SurvivorBehavior(this);
    m_survivorBehavior->setScene(m_scene);
    m_survivorBehavior->setCiphers(m_cachedCiphers);
    m_survivorBehavior->setGates(m_cachedGates);
    m_survivorBehavior->setHunter(m_hunter);
    m_survivorBehavior->setPlayerSurvivor(m_player);
    m_survivorBehavior->setGamePhase(1);

    m_aiDecisionMaker = new AIDecisionMaker(this);
    m_aiDecisionMaker->setScene(m_scene);
    m_aiDecisionMaker->setHunter(m_hunter);
    m_aiDecisionMaker->setSurvivors(m_aiSurvivors);
    m_aiDecisionMaker->setPlayer(m_player);
    m_aiDecisionMaker->setBehaviors(m_hunterBehavior, m_survivorBehavior);
    m_aiDecisionMaker->start();
    // ----------------------------------

    setGameState(Decoding);
    m_frameTimer.start(FRAME_INTERVAL);
    m_paused = false;

    m_survivorBehavior->setSurvivors(m_aiSurvivors, m_player);
    AudioManager::instance()->playMusic(AudioManager::GameBGM);   // 开始游戏背景
}

void GameEngine::restartGame()
{
    m_frameTimer.stop();
    startGame(m_lastSurvivorType);
}

void GameEngine::gameLoop()
{
    static int frameCount = 0;
    static QElapsedTimer fpsTimer;
    if (!fpsTimer.isValid()) fpsTimer.start();
    frameCount++;
    if (fpsTimer.elapsed() >= 1000) {
        qDebug() << "FPS:" << frameCount;
        frameCount = 0;
        fpsTimer.restart();
    }

    if (m_paused) return;

    // ========== 每帧动态切换背景音乐 ==========
    updateDynamicMusic();

    updateCountdown();

    static int cleanupCounter = 0;
    if (++cleanupCounter >= 60) {
        cleanupCounter = 0;
        for (int i = m_aiSurvivors.size() - 1; i >= 0; --i) {
            if (m_aiSurvivors[i]->isEliminated()) {
                m_aiSurvivors.removeAt(i);
            }
        }
    }

    for (auto* cipher : m_cachedCiphers)
        cipher->updateCipher();

    if (completedCipherCount() >= GameConfig::CIPHER_COUNT && m_state == Decoding) {
        for (auto* gate : m_cachedGates)
            gate->setUnlocked(true);
        setGameState(Escape);
        if (m_hunterBehavior) m_hunterBehavior->setGamePhase(2);
        if (m_survivorBehavior) m_survivorBehavior->setGamePhase(2);
    }

    if (m_state == Escape) {
        for (auto* gate : m_cachedGates)
            gate->updateGate();
    }

    m_scene->advance();
    if (m_player && m_player->isEnabled()) m_player->updateCharacter();
    if (m_hunter && m_hunter->isEnabled()) m_hunter->updateCharacter();
    for (auto* ai : m_aiSurvivors) if (ai->isEnabled()) ai->updateCharacter();

    emit cipherProgressChanged(completedCipherCount(), GameConfig::CIPHER_COUNT);
    emit timeUpdated(remainingTime());
}

void GameEngine::setGameState(GameState state)
{
    if (m_state == state) return;
    m_state = state;
    m_stageTimer.restart();
    emit gameStateChanged(state);

    switch (state) {
    case Preparation:
        if (m_player) m_player->setEnabled(false);
        if (m_hunter) m_hunter->setEnabled(false);
        for (auto *ai : m_aiSurvivors) ai->setEnabled(false);
        break;
    case Decoding:
        if (m_player) m_player->setEnabled(true);
        if (m_hunter) m_hunter->setEnabled(true);
        for (auto *ai : m_aiSurvivors) ai->setEnabled(true);
        break;
    case Escape:
        if (m_hunter) m_hunter->setSpeedMultiplier(GameConfig::ESCAPE_HUNTER_SPEED_RATIO);
        break;
    case Result:
        m_frameTimer.stop();
        break;
    }
}

void GameEngine::updateCountdown()
{
    qint64 elapsed = m_stageTimer.elapsed();
    bool timeout = false;
    switch (m_state) {
    case Preparation: if (elapsed >= PREPARATION_DURATION) timeout = true; break;
    case Decoding:    if (elapsed >= DECODING_DURATION) timeout = true; break;
    case Escape:      if (elapsed >= ESCAPE_DURATION) timeout = true; break;
    default: return;
    }
    if (timeout) {
        if (m_state == Preparation) setGameState(Decoding);
        else if (m_state == Decoding) hunterWin();
        else if (m_state == Escape) hunterWin();
    }
}

void GameEngine::checkVictoryCondition()
{
    if (m_escapedCount > 1) { survivorWin(); return; }
    if (aliveSurvivorCount() == 0) { hunterWin(); return; }
}

void GameEngine::survivorWin()
{
    if (m_state != Result) {
        AudioManager::instance()->playMusic(AudioManager::GameOver);
        setGameState(Result);
        emit gameEnded(true);
    }
}

void GameEngine::hunterWin()
{
    if (m_state != Result) {
        AudioManager::instance()->playMusic(AudioManager::GameOver);
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
    default: return 0;
    }
}

int GameEngine::completedCipherCount() const
{
    int count = 0;
    for (auto* c : m_cachedCiphers) if (c->isCompleted()) count++;
    return count;
}

int GameEngine::aliveSurvivorCount() const
{
    int count = 0;
    if (m_player && !m_player->isEliminated()) count++;
    for (auto* ai : m_aiSurvivors) if (!ai->isEliminated()) count++;
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

void GameEngine::exitToMainMenu()
{
    m_frameTimer.stop();
    cleanupGameWorld();
}

void GameEngine::createGameWorld(int survivorType)
{
    m_scene->createObstaclesAndBushes();

    m_player = new PlayerSurvivor(static_cast<SurvivorType>(survivorType));
    m_player->setPos(GameConfig::getSurvivorSpawnPoint(0));
    m_player->setGameScene(m_scene);
    m_player->setGameEngine(this);
    m_scene->addItem(m_player);
    m_scene->setPlayerCharacter(m_player);

    m_hunter = new Hunter();
    m_hunter->setPos(GameConfig::getHunterSpawnPoint());
    m_hunter->setGameScene(m_scene);
    m_scene->addItem(m_hunter);

    QList<SurvivorType> aiTypes;
    if (survivorType != 0) aiTypes.append(SurvivorType::Doctor);
    if (survivorType != 1) aiTypes.append(SurvivorType::Mechanic);
    if (survivorType != 2) aiTypes.append(SurvivorType::AirForce);
    for (int i = 0; i < aiTypes.size(); ++i) {
        AISurvivor *ai = new AISurvivor(aiTypes[i]);
        ai->setPos(GameConfig::getSurvivorSpawnPoint(i + 1));
        ai->setGameScene(m_scene);
        m_scene->addItem(ai);
        m_aiSurvivors.append(ai);
    }

    QList<QPointF> cipherPos = GameConfig::getCipherPositions();
    for (const QPointF &pos : cipherPos) {
        CipherMachine *c = new CipherMachine();
        c->setPos(pos);
        m_scene->addItem(c);
        m_cachedCiphers.append(c);
    }

    QList<QPointF> gatePos = GameConfig::getGatePositions();
    for (const QPointF &pos : gatePos) {
        Gate *g = new Gate();
        g->setPos(pos);
        m_scene->addItem(g);
        m_cachedGates.append(g);
    }

    m_escapedCount = 0;
    m_rescueCount = 0;
    m_destroyCount = 0;

    auto onEscape = [this]() {
        m_escapedCount++;
        checkVictoryCondition();
    };
    if (m_player) {
        connect(m_player, &Survivor::escaped, this, onEscape);
    }
    for (auto *ai : m_aiSurvivors) {
        connect(ai, &Survivor::escaped, this, onEscape);
    }
}

void GameEngine::cleanupGameWorld()
{
    if (m_aiDecisionMaker) {
        m_aiDecisionMaker->stop();
        delete m_aiDecisionMaker;
        m_aiDecisionMaker = nullptr;
    }
    delete m_hunterBehavior;
    m_hunterBehavior = nullptr;
    delete m_survivorBehavior;
    m_survivorBehavior = nullptr;

    m_frameTimer.stop();
    if (m_scene) m_scene->clear();
    m_player = nullptr;
    m_hunter = nullptr;
    m_aiSurvivors.clear();
    m_cachedCiphers.clear();
    m_cachedGates.clear();
}

// ==================== 动态音乐切换实现 ====================
void GameEngine::updateDynamicMusic()
{
    // 只在解码或逃脱阶段切换动态音乐
    if (m_state != Decoding && m_state != Escape) return;
    Hunter *hunter = m_hunter;
    if (!hunter || hunter->isStunned()) {
        // 监管者眩晕时强制回到普通背景音乐
        AudioManager::instance()->playMusic(AudioManager::GameBGM);
        return;
    }

    bool near = false;
    bool chase = false;

    // 检查玩家
    if (m_player && !m_player->isEliminated()) {
        qreal dist = QLineF(m_player->pos(), hunter->pos()).length();
        if (dist < 200.0) near = true;
        // 玩家没有 isBeingChased 标志，依赖距离判断即可
    }

    // 检查所有 AI 求生者
    for (AISurvivor *ai : m_aiSurvivors) {
        if (ai->isEliminated()) continue;
        qreal dist = QLineF(ai->pos(), hunter->pos()).length();
        if (dist < 200.0) near = true;
        if (ai->isBeingChased()) chase = true;
    }

    // 优先级：追击 > 靠近 > 正常
    if (chase) {
        AudioManager::instance()->playMusic(AudioManager::HunterChase);
    } else if (near) {
        AudioManager::instance()->playMusic(AudioManager::HunterNearby);
    } else {
        AudioManager::instance()->playMusic(AudioManager::GameBGM);
    }
}