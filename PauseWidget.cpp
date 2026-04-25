#include "PauseWidget.h"
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QLabel>

PauseWidget::PauseWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(350, 250);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background:rgba(20,20,40,220);border-radius:15px;border:2px solid rgba(255,255,255,100);");
    setupUI();
}

void PauseWidget::setupUI()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QLabel *t = new QLabel("游戏暂停", this); t->setAlignment(Qt::AlignCenter); t->setStyleSheet("color:white;font-size:22px;font-weight:bold;");
    lay->addWidget(t); lay->addStretch();
    QString bs = "QPushButton{background:rgba(60,60,100,200);color:white;border:2px solid rgba(255,255,255,150);border-radius:8px;padding:10px;font-size:16px;font-weight:bold;}QPushButton:hover{background:rgba(80,80,140,220);}";
    QPushButton *resume = new QPushButton("继续游戏", this); resume->setStyleSheet(bs);
    QPushButton *restart = new QPushButton("重新开始", this); restart->setStyleSheet(bs);
    QPushButton *menu = new QPushButton("返回主菜单", this); menu->setStyleSheet(bs);
    lay->addWidget(resume); lay->addWidget(restart); lay->addWidget(menu); lay->addStretch();

    connect(resume, &QPushButton::clicked, this, [this](){ emit resumeGame(); close(); });
    connect(restart, &QPushButton::clicked, this, [this](){ emit restartGame(); close(); });
    connect(menu, &QPushButton::clicked, this, [this](){ emit backToMainMenu(); close(); });
}

void PauseWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) { emit resumeGame(); close(); }
    else QWidget::keyPressEvent(event);
}