#include "GameUI.h"
#include "GameEngine.h"
#include "GameScene.h"
#include "GameHUDWidget.h"
#include "MiniMapWidget.h"
#include "utils/CameraFollow.h"
#include "ui/ResultWidget.h"
#include "ui/PauseWidget.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QTimer>
#include <QPainter>
#include "entities/PlayerSurvivor.h"

GameUI::GameUI(QWidget *parent) : QWidget(parent), m_cameraTimer(nullptr)
{
    setWindowTitle("Qt2D 非对称对抗");
    resize(1280, 900);
    setupUI();
}

void GameUI::setupUI()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->setContentsMargins(0,0,0,0);

    m_view = new QGraphicsView(this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 焦点策略将在 initGame 中设置为 StrongFocus 并获取焦点
    m_view->setFocusPolicy(Qt::NoFocus);  // 临时设为 NoFocus，initGame 时会改为 StrongFocus
    m_view->setFixedSize(1000, 700);
    lay->addWidget(m_view, 0, Qt::AlignCenter);

    m_hud = new GameHUDWidget(this);
    m_minimap = new MiniMapWidget(this);
    m_camera = new CameraFollow(this);
}

void GameUI::setGameEngine(GameEngine *engine)
{
    m_engine = engine;
    m_camera->setView(m_view);
    m_minimap->setGameEngine(engine);
    m_hud->setGameEngine(engine);
}

void GameUI::initGame()
{
    m_view->setRenderHint(QPainter::Antialiasing, false);
    m_view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    if (!m_engine) return;

    // ========= 焦点设置：让游戏窗口本身和视图层正确接收键盘事件 =========
    setFocusPolicy(Qt::StrongFocus);    // GameUI 窗口可以获取焦点
    setFocus();                         // 窗口获取焦点（需在 show 之后调用，通常外部会 show）
    activateWindow();                   // 确保窗口为活动窗口

    // 关键修正：QGraphicsView 必须拥有键盘焦点才能将按键传递给场景
    // 之前错误地设为 NoFocus 会导致场景永远收不到键盘事件
    m_view->setFocusPolicy(Qt::StrongFocus);
    m_view->setFocus();                 // 让视图获取键盘焦点

    // ========= 场景绑定 =========
    m_view->setScene(m_engine->getScene());

    // ========= HUD 穿透：保持对鼠标透明（点击可穿透到视图），但不获取键盘焦点 =========
    m_hud->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    m_hud->setFocusPolicy(Qt::NoFocus); // HUD 不干扰键盘输入

    // ========= 相机：立即跳转到玩家，再开启平滑跟随 =========
    m_camera->setTarget(m_engine->getPlayer());
    m_camera->snapToTarget();            // 首次立即居中
    m_camera->setSmooth(true, 0.2);

    // ========= 玩家关联 =========
    m_hud->setPlayer(m_engine->getPlayer());
    if (auto* player = m_engine->getPlayer())
        player->setGameEngine(m_engine);

    m_hud->reset();
    m_minimap->startUpdate();
    m_hud->show();
    m_minimap->show();

    // ========= 相机定时更新 =========
    if (m_cameraTimer) {
        m_cameraTimer->stop();
        delete m_cameraTimer;
    }
    m_cameraTimer = new QTimer(this);
    connect(m_cameraTimer, &QTimer::timeout, m_camera, &CameraFollow::update);
    m_cameraTimer->start(20);

    // ========= 游戏结束信号 =========
    connect(m_engine, &GameEngine::gameEnded, this, [this](bool w){
        m_minimap->stopUpdate();
        ResultWidget *r = new ResultWidget(this);
        r->setResult(w, m_engine->escapedSurvivorCount(),
                     3 - m_engine->aliveSurvivorCount(),
                     m_engine->completedCipherCount(),
                     m_engine->rescueCount(),
                     m_engine->destroyCount());
        r->show();
        connect(r, &ResultWidget::playAgain, this, [this](){ m_engine->restartGame(); initGame(); });
        connect(r, &ResultWidget::backToMenu, this, &GameUI::backToMainMenu);
        connect(r, &ResultWidget::quitGame, qApp, &QApplication::quit);
    });

    this->activateWindow();
}

void GameUI::resizeEvent(QResizeEvent *)
{
    if (m_hud) m_hud->setGeometry(0, 0, width(), height());
    if (m_minimap) m_minimap->move(10, 10);
}

// 键盘按下事件：GameUI 窗口捕获后转发给 QGraphicsView
void GameUI::keyPressEvent(QKeyEvent *e)
{
    // ---------- 暂停功能独立处理（不转发给游戏逻辑）----------
    if (e->key() == Qt::Key_Escape && m_engine) {
        m_engine->pauseGame();
        PauseWidget *p = new PauseWidget(this);
        p->setWindowModality(Qt::ApplicationModal);
        p->show();
        connect(p, &PauseWidget::resumeGame, m_engine, &GameEngine::resumeGame);
        connect(p, &PauseWidget::restartGame, this, [this](){ m_engine->restartGame(); initGame(); });
        connect(p, &PauseWidget::backToMainMenu, this, &GameUI::backToMainMenu);
        return; // 暂停事件已处理，不再向下转发
    }

    // ---------- 将其他按键（WASD/空格/F）转发给 QGraphicsView ----------
    // 视图会自动调用其绑定的 QGraphicsScene 的 keyPressEvent，
    // 从而到达 PlayerSurvivor 等角色的键盘处理函数。
    if (m_view) {
        QApplication::sendEvent(m_view, e);
    }
}

// 键盘释放事件：必须同样转发给视图，否则角色无法停止移动
void GameUI::keyReleaseEvent(QKeyEvent *e)
{
    if (m_view) {
        QApplication::sendEvent(m_view, e);
    }
}

void GameUI::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    QPixmap bg(":/new/prefix1/images/lumain.png");
    if (!bg.isNull()) {
        painter.drawPixmap(rect(), bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
    QWidget::paintEvent(event);
}