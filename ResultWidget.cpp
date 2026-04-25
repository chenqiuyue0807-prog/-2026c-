#include "ResultWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>

ResultWidget::ResultWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("结算"); setFixedSize(500, 450);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background:rgba(20,20,40,220);border-radius:20px;border:2px solid rgba(255,255,255,100);");
    setupUI();
}

void ResultWidget::setupUI()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    m_title = new QLabel("游戏结束", this); m_title->setAlignment(Qt::AlignCenter); m_title->setStyleSheet("font-size:32px;font-weight:bold;color:white;");
    m_subtitle = new QLabel("", this); m_subtitle->setAlignment(Qt::AlignCenter); m_subtitle->setStyleSheet("font-size:18px;font-weight:bold;color:gold;");
    m_stats = new QLabel(this); m_stats->setAlignment(Qt::AlignCenter); m_stats->setStyleSheet("color:#ccc;font-size:16px;");
    lay->addWidget(m_title); lay->addWidget(m_subtitle); lay->addSpacing(10); lay->addWidget(m_stats); lay->addStretch();

    QHBoxLayout *btnRow = new QHBoxLayout;
    m_againBtn = new QPushButton("再来一局", this); m_menuBtn = new QPushButton("返回主菜单", this); m_quitBtn = new QPushButton("退出游戏", this);
    QString bs = "QPushButton{background:rgba(60,60,100,200);color:white;border:2px solid rgba(255,255,255,150);border-radius:8px;padding:10px 20px;font-size:16px;font-weight:bold;}QPushButton:hover{background:rgba(80,80,140,220);}";
    for (auto *b : {m_againBtn, m_menuBtn, m_quitBtn}) b->setStyleSheet(bs);
    btnRow->addWidget(m_againBtn); btnRow->addWidget(m_menuBtn); btnRow->addWidget(m_quitBtn);
    lay->addLayout(btnRow);

    connect(m_againBtn, &QPushButton::clicked, this, [this](){ emit playAgain(); close(); });
    connect(m_menuBtn, &QPushButton::clicked, this, [this](){ emit backToMenu(); close(); });
    connect(m_quitBtn, &QPushButton::clicked, this, [this](){ emit quitGame(); close(); });
}

void ResultWidget::setResult(bool w, int esc, int elim, int ciph, int resc, int dest)
{
    m_title->setText(w?"求生者获胜！":"监管者获胜！");
    m_title->setStyleSheet(QString("font-size:32px;font-weight:bold;color:%1;").arg(w?"#4CAF50":"#F44336"));
    m_subtitle->setText(bestTitle(w, esc, resc, ciph, elim, dest));
    m_stats->setText(QString("逃脱:%1  淘汰:%2\n密码机:%3/3\n救助:%4  破坏:%5").arg(esc).arg(elim).arg(ciph).arg(resc).arg(dest));
}

QString ResultWidget::bestTitle(bool w, int esc, int resc, int ciph, int elim, int dest)
{
    if (w) {
        if (esc >= 2) return "「团队之星」";
        if (resc >= 2) return "「救援大师」";
        if (ciph >= 3) return "「破译专家」";
        return "「幸运逃生」";
    } else {
        if (elim >= 3) return "「完美猎杀」";
        if (dest >= 3) return "「拆迁队长」";
        return "「巡逻者」";
    }
}