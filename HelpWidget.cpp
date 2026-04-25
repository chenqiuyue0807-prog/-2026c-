#include "HelpWidget.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>

HelpWidget::HelpWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("帮助"); setFixedSize(700, 500);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    setStyleSheet("background-color:rgb(30,30,50);color:white;");
    setupUI();
}

void HelpWidget::setupUI()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QLabel *t = new QLabel("游戏帮助", this); t->setAlignment(Qt::AlignCenter); t->setStyleSheet("font-size:24px;font-weight:bold;");
    lay->addWidget(t);

    QTabWidget *tab = new QTabWidget(this);
    auto makeTab = [](const QString &html) {
        QWidget *w = new QWidget; QVBoxLayout *l = new QVBoxLayout(w);
        QLabel *lb = new QLabel(html); lb->setWordWrap(true); lb->setStyleSheet("color:#ccc;font-size:14px;");
        l->addWidget(lb); return w;
    };
    tab->addTab(makeTab("<h3>规则</h3>1监管者vs3求生者。破译3台密码机→开门逃脱。求生者至少1人逃脱即胜；监管者淘汰全部或超时即胜。"), "规则");
    tab->addTab(makeTab("<h3>按键</h3>WASD移动，空格交互，F技能，Esc暂停。<br>监管者：鼠标左键攻击，右键破坏障碍物。"), "按键");
    tab->addTab(makeTab("<h3>求生者</h3><b>医生</b>：治疗(20s)<br><b>机械师</b>：傀儡破译(25s)<br><b>空军</b>：信号枪眩晕3s(35s)"), "求生者");
    tab->addTab(makeTab("<h3>监管者</h3>厂长：扇形攻击(8s冷却)，破坏障碍物(墙壁3s/箱子2s/板区1s)。"), "监管者");
    lay->addWidget(tab);

    QPushButton *back = new QPushButton("返回", this);
    connect(back, &QPushButton::clicked, this, [this](){ emit backClicked(); close(); });
    lay->addWidget(back, 0, Qt::AlignCenter);
}