#include "mainwindow.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Escape Game - Step 3");
    resize(1200, 850);

    m_scene = new GameScene(this);
    m_view = new QGraphicsView(m_scene, this);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setRenderHint(QPainter::Antialiasing);
    m_view->setFocusPolicy(Qt::StrongFocus);

    setupUI();

    // 将 UI 控件传递给场景
    m_scene->setUIWidgets(m_phaseLabel, m_cipherLabel, m_timerLabel, m_interactProgressBar);

    // 连接游戏结束信号
    connect(m_scene, &GameScene::gameOver, this, [this](bool survivorWin) {
        QString msg = survivorWin ? "求生者获胜！" : "监管者获胜！";
        statusBar()->showMessage(msg, 5000);
    });
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // 顶部信息栏
    QWidget *infoBar = new QWidget();
    infoBar->setFixedHeight(40);
    QHBoxLayout *infoLayout = new QHBoxLayout(infoBar);
    infoLayout->setContentsMargins(10, 5, 10, 5);

    m_phaseLabel = new QLabel("准备阶段");
    m_phaseLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    m_cipherLabel = new QLabel("密码机: 0/3");
    m_cipherLabel->setStyleSheet("font-size: 14px;");
    m_timerLabel = new QLabel("00:00");
    m_timerLabel->setStyleSheet("font-size: 14px;");
    m_interactProgressBar = new QProgressBar();
    m_interactProgressBar->setMaximum(100);
    m_interactProgressBar->setValue(0);
    m_interactProgressBar->setVisible(false);
    m_interactProgressBar->setMaximumWidth(200);

    infoLayout->addWidget(m_phaseLabel);
    infoLayout->addWidget(m_cipherLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_timerLabel);
    infoLayout->addWidget(m_interactProgressBar);

    mainLayout->addWidget(infoBar);
    mainLayout->addWidget(m_view);

    setCentralWidget(m_centralWidget);
    statusBar()->showMessage("WASD移动，靠近密码机/大门按空格交互");
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_scene) {
        m_scene->handleKeyPress(event->key());
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    if (m_scene) {
        m_scene->handleKeyRelease(event->key());
    }
    QMainWindow::keyReleaseEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (m_scene) {
        QPointF scenePos = m_view->mapToScene(event->pos());
        m_scene->handleMousePress(event->button(), scenePos);
    }
    QMainWindow::mousePressEvent(event);
}