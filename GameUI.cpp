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
#include "entities/PlayerSurvivor.h"

GameUI::GameUI(QWidget *parent) : QWidget(parent)
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
    lay->addWidget(m_view);

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
    if (!m_engine) return;
    m_view->setScene(m_engine->getScene());
    m_camera->setTarget(m_engine->getPlayer());
    m_hud->setPlayer(m_engine->getPlayer());
    m_hud->reset();
    m_minimap->startUpdate();
    m_hud->show();
    m_minimap->show();

    // 启动相机更新定时器
    QTimer *cameraTimer = new QTimer(this);
    connect(cameraTimer, &QTimer::timeout, m_camera, &CameraFollow::update);
    cameraTimer->start(16);  // ~60 FPS

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
}

void GameUI::resizeEvent(QResizeEvent *)
{
    if (m_hud) m_hud->setGeometry(0, 0, width(), height());
    if (m_minimap) m_minimap->move(10, 10);
}

void GameUI::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape && m_engine) {
        m_engine->pauseGame();
        PauseWidget *p = new PauseWidget(this);
        p->show();
        connect(p, &PauseWidget::resumeGame, m_engine, &GameEngine::resumeGame);
        connect(p, &PauseWidget::restartGame, this, [this](){ m_engine->restartGame(); initGame(); });
        connect(p, &PauseWidget::backToMainMenu, this, &GameUI::backToMainMenu);
    }
    QWidget::keyPressEvent(e);
}