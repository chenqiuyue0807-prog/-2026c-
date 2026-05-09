#include "MainMenuWidget.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QGraphicsDropShadowEffect>
#include <QSoundEffect>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>

MainMenuWidget::MainMenuWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(1000, 700);
    setWindowTitle("非对称对抗 - 主菜单");
    setupUI();
    setupStyle();
}

void MainMenuWidget::setupUI()
{
    m_titleLabel = new QLabel("第五人格简易版", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setStyleSheet("color: white; font-size: 48px; font-weight: bold; background: transparent;");

    m_startBtn = new QPushButton("开始游戏", this);
    m_settingsBtn = new QPushButton("设置", this);
    m_helpBtn = new QPushButton("帮助", this);
    m_quitBtn = new QPushButton("退出游戏", this);

    QSize btnSize(200, 60);
    for (auto *btn : {m_startBtn, m_settingsBtn, m_helpBtn, m_quitBtn})
        btn->setFixedSize(btnSize);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(20);
    layout->addStretch();
    layout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    layout->addSpacing(40);
    for (auto *btn : {m_startBtn, m_settingsBtn, m_helpBtn, m_quitBtn})
        layout->addWidget(btn, 0, Qt::AlignCenter);
    layout->addStretch();

    connect(m_startBtn, &QPushButton::clicked, this, &MainMenuWidget::startGameClicked);
    connect(m_settingsBtn, &QPushButton::clicked, this, &MainMenuWidget::settingsClicked);
    connect(m_helpBtn, &QPushButton::clicked, this, &MainMenuWidget::helpClicked);
    connect(m_quitBtn, &QPushButton::clicked, qApp, &QApplication::quit);
    connect(m_quitBtn, &QPushButton::clicked, this, &MainMenuWidget::quitClicked);
}

void MainMenuWidget::setupStyle()
{
    QString style = R"(
        QPushButton {
            background-color: rgba(30, 30, 50, 180); color: white;
            border: 2px solid rgba(255, 255, 255, 150); border-radius: 10px;
            font-size: 20px; font-weight: bold;
        }
        QPushButton:hover { background-color: rgba(70, 70, 120, 200); border: 2px solid white; }
    )";
    for (auto *btn : {m_startBtn, m_settingsBtn, m_helpBtn, m_quitBtn})
        btn->setStyleSheet(style);

    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(15); shadow->setColor(QColor(0,0,0,200)); shadow->setOffset(3,3);
    m_titleLabel->setGraphicsEffect(shadow);
}

void MainMenuWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    QPixmap bg(":/new/prefix1/images/background.png");
    if (!bg.isNull()) {
        painter.drawPixmap(rect(), bg.scaled(size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    }
}

void MainMenuWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    // 窗口首次显示时播放主菜单音乐
    AudioManager::instance()->playMusic(AudioManager::MainMenu);
}