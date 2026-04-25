#include "GameHUDWidget.h"
#include "GameEngine.h"
#include "entities/PlayerSurvivor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>

GameHUDWidget::GameHUDWidget(QWidget *parent) : QWidget(parent), m_msgTimer(new QTimer(this))
{
    setAttribute(Qt::WA_TransparentForMouseEvents, false);
    setAttribute(Qt::WA_TranslucentBackground);
    m_msgTimer->setSingleShot(true);
    setupUI();   // 先创建所有控件
    connect(m_msgTimer, &QTimer::timeout, m_msgLabel, &QLabel::clear); // 此时 m_msgLabel 已存在
}

void GameHUDWidget::setupUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);
    QHBoxLayout *top = new QHBoxLayout;
    m_healthLabel1 = new QLabel("❤️", this); m_healthLabel1->setStyleSheet("font-size:28px;color:red;");
    m_healthLabel2 = new QLabel("❤️", this); m_healthLabel2->setStyleSheet("font-size:28px;color:red;");
    top->addWidget(m_healthLabel1); top->addWidget(m_healthLabel2);
    top->addStretch();
    m_skillBar = new QProgressBar(this); m_skillBar->setFixedSize(100,18); m_skillBar->setRange(0,100); m_skillBar->setValue(100); m_skillBar->setTextVisible(false);
    top->addWidget(new QLabel("技能", this)); top->addWidget(m_skillBar);
    top->addStretch();
    m_cipherLabel = new QLabel("密码机:0/3", this); m_cipherLabel->setStyleSheet("color:white;background:rgba(0,0,0,120);padding:4px 8px;border-radius:5px;");
    top->addWidget(m_cipherLabel);
    top->addStretch();
    m_timeLabel = new QLabel("00:00", this); m_timeLabel->setStyleSheet("color:white;background:rgba(0,0,0,120);padding:4px 10px;border-radius:5px;font-size:16px;");
    top->addWidget(m_timeLabel);
    m_pauseBtn = new QPushButton("⏸", this); m_pauseBtn->setFixedSize(40,40); m_pauseBtn->setStyleSheet("background:rgba(0,0,0,150);color:white;border-radius:8px;");
    top->addWidget(m_pauseBtn);
    main->addLayout(top);

    QHBoxLayout *mid = new QHBoxLayout; mid->addStretch();
    m_gateLabel = new QLabel("大门未解锁", this); m_gateLabel->setStyleSheet("color:white;background:rgba(0,0,0,150);padding:6px 20px;border-radius:8px;font-size:16px;");
    mid->addWidget(m_gateLabel); mid->addStretch();
    main->addLayout(mid);
    main->addStretch();

    m_msgLabel = new QLabel("", this); m_msgLabel->setAlignment(Qt::AlignCenter); m_msgLabel->setStyleSheet("color:yellow;background:rgba(0,0,0,180);padding:10px;border-radius:8px;font-size:16px;"); m_msgLabel->hide();
    main->addWidget(m_msgLabel);

    connect(m_pauseBtn, &QPushButton::clicked, this, &GameHUDWidget::pauseClicked);
}

// 其余函数与原来相同，无需修改

void GameHUDWidget::setGameEngine(GameEngine *engine)
{
    m_engine = engine;
    if (m_engine) {
        connect(m_engine, &GameEngine::timeUpdated, this, &GameHUDWidget::updateTimeDisplay);
        connect(m_engine, &GameEngine::cipherProgressChanged, this, &GameHUDWidget::updateCipherProgress);
        connect(m_engine, &GameEngine::gameStateChanged, this, [this](int s){ if(s==2) updateGateStatus(true,0); });
    }
}

void GameHUDWidget::setPlayer(PlayerSurvivor *player)
{
    if (player) {
        connect(player, &Survivor::healthChanged, this, &GameHUDWidget::updateHealthDisplay);
        connect(player, &PlayerSurvivor::skillUsed, this, [this](){ updateSkillCooldown(1.0); });
        connect(player, &PlayerSurvivor::interactionPrompt, this, [this](const QString &t){ showMessage(t,1500); });
    }
}

void GameHUDWidget::updateHealthDisplay(int h)
{
    m_healthLabel1->setText(h>=1?"❤️":"🖤"); m_healthLabel2->setText(h>=2?"❤️":"🖤");
    m_healthLabel1->setStyleSheet(QString("font-size:28px;color:%1;").arg(h>=1?"red":"gray"));
    m_healthLabel2->setStyleSheet(QString("font-size:28px;color:%1;").arg(h>=2?"red":"gray"));
}
void GameHUDWidget::updateSkillCooldown(qreal r) { m_skillBar->setValue(r*100); }
void GameHUDWidget::updateCipherProgress(int c, int t) { m_cipherLabel->setText(QString("密码机:%1/%2").arg(c).arg(t)); }
void GameHUDWidget::updateTimeDisplay(int s) { m_timeLabel->setText(QString("%1:%2").arg(s/60,2,10,QChar('0')).arg(s%60,2,10,QChar('0'))); }
void GameHUDWidget::updateGateStatus(bool u, int p) { m_gateLabel->setText(u?(p>=100?"大门已开启！":QString("大门开启中:%1%").arg(p)):"大门未解锁"); }
void GameHUDWidget::showMessage(const QString &m, int d) { m_msgLabel->setText(m); m_msgLabel->show(); m_msgTimer->start(d); }
void GameHUDWidget::reset() { updateHealthDisplay(2); updateSkillCooldown(0); updateCipherProgress(0,3); updateTimeDisplay(0); updateGateStatus(false,0); }