#include "GameScene.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>

// ========== 构造函数与初始化 ==========
GameScene::GameScene(QObject *parent)
    : QGraphicsScene(parent)
    , m_keyUp(false), m_keyDown(false), m_keyLeft(false), m_keyRight(false)
    , m_keySpace(false), m_keyF(false)
    , m_player(nullptr), m_hunter(nullptr)
    , m_gamePhase(GamePhase::Preparation)
    , m_phaseTimer(0)
    , m_completedCiphers(0)
    , m_interactingSurvivor(nullptr)
    , m_currentCipher(nullptr)
    , m_currentGate(nullptr)
    , m_interactProgress(0)
    , m_interactSpeed(0)
    , m_phaseLabel(nullptr)
    , m_cipherLabel(nullptr)
    , m_timerLabel(nullptr)
    , m_interactProgressBar(nullptr)
{
    setSceneRect(0, 0, 1200, 800);
    setBackgroundBrush(Qt::gray);

    // 帧循环 (60 FPS)
    m_gameTimer = new QTimer(this);
    connect(m_gameTimer, &QTimer::timeout, this, &GameScene::updateGame);
    m_gameTimer->start(16);

    // AI 决策循环 (200ms)
    m_aiTimer = new QTimer(this);
    connect(m_aiTimer, &QTimer::timeout, this, &GameScene::updateAI);
    m_aiTimer->start(200);

    initScene();
}

GameScene::~GameScene()
{
    // 清理所有角色（QGraphicsScene 会自动删除 item，但 QObject 子类需手动 delete）
    qDeleteAll(m_allRoles);
    m_allRoles.clear();
}

void GameScene::initScene()
{
    // ----- 创建玩家（医生）-----
    m_player = new RoleItem(RoleType::Doctor);
    m_player->setPos(100, 700);
    addItem(m_player);
    m_allRoles.append(m_player);

    // ----- 创建监管者（厂长）-----
    m_hunter = new RoleItem(RoleType::Hunter);
    m_hunter->setPos(600, 400);
    addItem(m_hunter);
    m_allRoles.append(m_hunter);

    // ----- 创建 AI 求生者（机械师、空军）-----
    RoleItem *ai1 = new RoleItem(RoleType::Mechanic);
    ai1->setPos(1100, 700);
    addItem(ai1);
    m_allRoles.append(ai1);

    RoleItem *ai2 = new RoleItem(RoleType::AirForce);
    ai2->setPos(100, 100);
    addItem(ai2);
    m_allRoles.append(ai2);

    // ----- 第3步：初始化密码机和大门 -----
    initCipherMachines();
    initGates();

    // 为了方便测试，直接进入破译阶段（跳过准备阶段）
    m_gamePhase = GamePhase::Decoding;
}

// ========== 第3步：密码机与大门初始化 ==========
void GameScene::initCipherMachines()
{
    QList<QPointF> positions = {
        QPointF(200, 200),
        QPointF(600, 400),
        QPointF(1000, 600)
    };

    for (const QPointF &pos : positions) {
        CipherMachineItem *cipher = new CipherMachineItem(pos);
        addItem(cipher);
        m_cipherMachines.append(cipher);
        connect(cipher, &CipherMachineItem::completed, this, &GameScene::onCipherCompleted);
    }
}

void GameScene::initGates()
{
    QList<QPointF> positions = {
        QPointF(50, 400),
        QPointF(1100, 400)
    };

    for (const QPointF &pos : positions) {
        GateItem *gate = new GateItem(pos);
        gate->setUnlocked(false);
        addItem(gate);
        m_gates.append(gate);
        // 大门完全开启 -> 求生者胜利
        connect(gate, &GateItem::opened, this, [this]() {
            emit gameOver(true);
        });
    }
}

// ========== 第3步：UI 控件设置 ==========
void GameScene::setUIWidgets(QLabel *phaseLabel, QLabel *cipherLabel,
                             QLabel *timerLabel, QProgressBar *interactProgress)
{
    m_phaseLabel = phaseLabel;
    m_cipherLabel = cipherLabel;
    m_timerLabel = timerLabel;
    m_interactProgressBar = interactProgress;

    if (m_cipherLabel) {
        m_cipherLabel->setText(QString("密码机: %1/3").arg(m_completedCiphers));
    }
    if (m_phaseLabel) {
        m_phaseLabel->setText("破译阶段");
    }
}

// ========== 键盘/鼠标输入转发 ==========
void GameScene::handleKeyPress(int key)
{
    switch (key) {
    case Qt::Key_W:     m_keyUp = true; break;
    case Qt::Key_S:     m_keyDown = true; break;
    case Qt::Key_A:     m_keyLeft = true; break;
    case Qt::Key_D:     m_keyRight = true; break;
    case Qt::Key_Space: m_keySpace = true; break;
    case Qt::Key_F:     m_keyF = true; break;
    default: break;
    }
}

void GameScene::handleKeyRelease(int key)
{
    switch (key) {
    case Qt::Key_W:     m_keyUp = false; break;
    case Qt::Key_S:     m_keyDown = false; break;
    case Qt::Key_A:     m_keyLeft = false; break;
    case Qt::Key_D:     m_keyRight = false; break;
    case Qt::Key_Space: m_keySpace = false; break;
    case Qt::Key_F:     m_keyF = false; break;
    default: break;
    }
}

void GameScene::handleMousePress(Qt::MouseButton button, const QPointF &pos)
{
    // 后续步骤实现监管者攻击/破坏障碍物
    Q_UNUSED(button);
    Q_UNUSED(pos);
}

// ========== 主更新循环（每帧） ==========
void GameScene::updateGame()
{
    // 1. 玩家移动输入（WASD）
    if (m_player && !m_player->isDead() && !m_player->isInteracting()) {
        qreal dx = 0, dy = 0;
        if (m_keyUp)    dy -= 1.0;
        if (m_keyDown)  dy += 1.0;
        if (m_keyLeft)  dx -= 1.0;
        if (m_keyRight) dx += 1.0;

        m_player->setMoveDirection(dx, dy);
    } else if (m_player) {
        m_player->stopMoving();
    }

    // 2. 更新所有角色位置（含 AI 暂时不会移动，后续实现）
    for (RoleItem *role : m_allRoles) {
        if (role) {
            role->updatePosition(sceneBounds());
        }
    }

    // 3. 第3步：处理玩家交互（空格键破译/开门）
    handleInteraction();

    // 后续步骤可在此添加攻击冷却更新、技能冷却等
}

// ========== AI 决策循环（每 200ms） ==========
void GameScene::updateAI()
{
    // 后续步骤实现监管者 AI 和 AI 求生者
}

// ========== 第3步：交互逻辑 ==========
void GameScene::handleInteraction()
{
    if (!m_player || m_player->isDead()) return;

    // 如果正在交互中，持续更新进度
    if (m_interactingSurvivor == m_player) {
        updateInteractProgress();
        return;
    }

    if (!m_keySpace) return;
    if (m_player->isInteracting()) return;

    QPointF playerPos = m_player->pos();

    // 1. 优先检查大门（已解锁）
    for (GateItem *gate : m_gates) {
        if (gate->isUnlocked() && !gate->isOpened()) {
            QPointF diff = gate->pos() - playerPos;
            qreal distSq = diff.x() * diff.x() + diff.y() * diff.y();
            if (distSq <= 50 * 50) {
                startOpeningGate(gate);
                return;
            }
        }
    }

    // 2. 检查密码机（未完成）
    for (CipherMachineItem *cipher : m_cipherMachines) {
        if (!cipher->isCompleted()) {
            QPointF diff = cipher->pos() - playerPos;
            qreal distSq = diff.x() * diff.x() + diff.y() * diff.y();
            if (distSq <= 50 * 50) {
                startDecoding(cipher);
                return;
            }
        }
    }
}

void GameScene::startDecoding(CipherMachineItem *cipher)
{
    if (!m_player) return;

    m_player->setState(RoleState::Interacting);
    m_interactingSurvivor = m_player;
    m_currentCipher = cipher;
    m_currentGate = nullptr;
    m_interactProgress = cipher->progress();
    m_interactSpeed = 0.2;   // 测试用较快速度

    cipher->setBeingDecoded(true);

    if (m_interactProgressBar) {
        m_interactProgressBar->setVisible(true);
        m_interactProgressBar->setValue(static_cast<int>(m_interactProgress));
    }
}

void GameScene::startOpeningGate(GateItem *gate)
{
    if (!m_player) return;

    m_player->setState(RoleState::Interacting);
    m_interactingSurvivor = m_player;
    m_currentGate = gate;
    m_currentCipher = nullptr;
    m_interactProgress = gate->openProgress();
    m_interactSpeed = 0.2;

    gate->setBeingOpened(true);

    if (m_interactProgressBar) {
        m_interactProgressBar->setVisible(true);
        m_interactProgressBar->setValue(static_cast<int>(m_interactProgress));
    }
}

void GameScene::stopInteracting()
{
    if (m_interactingSurvivor) {
        m_interactingSurvivor->setState(RoleState::Idle);
        m_interactingSurvivor = nullptr;
    }
    if (m_currentCipher) {
        m_currentCipher->setBeingDecoded(false);
        m_currentCipher = nullptr;
    }
    if (m_currentGate) {
        m_currentGate->setBeingOpened(false);
        m_currentGate = nullptr;
    }
    if (m_interactProgressBar) {
        m_interactProgressBar->setVisible(false);
    }
}

void GameScene::updateInteractProgress()
{
    // 如果松开空格或交互者不存在，停止交互
    if (!m_interactingSurvivor || !m_keySpace) {
        stopInteracting();
        return;
    }

    m_interactProgress += m_interactSpeed;
    if (m_interactProgress > 100) m_interactProgress = 100;

    if (m_currentCipher) {
        m_currentCipher->setProgress(static_cast<int>(m_interactProgress));
        if (m_interactProgressBar) {
            m_interactProgressBar->setValue(static_cast<int>(m_interactProgress));
        }
        if (m_currentCipher->isCompleted()) {
            stopInteracting();
        }
    }
    else if (m_currentGate) {
        m_currentGate->setOpenProgress(static_cast<int>(m_interactProgress));
        if (m_interactProgressBar) {
            m_interactProgressBar->setValue(static_cast<int>(m_interactProgress));
        }
        if (m_currentGate->isOpened()) {
            stopInteracting();
        }
    }
}

// ========== 第3步：密码机完成槽 ==========
void GameScene::onCipherCompleted()
{
    m_completedCiphers++;

    if (m_cipherLabel) {
        m_cipherLabel->setText(QString("密码机: %1/3").arg(m_completedCiphers));
    }

    // 完成三台密码机，解锁所有大门
    if (m_completedCiphers >= 3) {
        for (GateItem *gate : m_gates) {
            gate->setUnlocked(true);
        }
        m_gamePhase = GamePhase::Escape;
        if (m_phaseLabel) {
            m_phaseLabel->setText("逃脱阶段！快开门！");
        }
    }
}