#include "LoadingWidget.h"
#include <QVBoxLayout>
#include <QRandomGenerator>

LoadingWidget::LoadingWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(400, 250);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setStyleSheet("background-color: rgba(20,20,40,220); border-radius:15px;");
    setupUI();
    m_timer = new QTimer(this);
    m_timer->setInterval(30);
    connect(m_timer, &QTimer::timeout, this, [this](){
        m_progressValue += 2;
        if (m_progressValue >= 100) { m_progressValue = 100; m_timer->stop(); }
        m_progressBar->setValue(m_progressValue);
    });
    QTimer::singleShot(2000, this, [this](){ emit loadingFinished(); close(); });
}

void LoadingWidget::setupUI()
{
    QVBoxLayout *lay = new QVBoxLayout(this);
    QLabel *title = new QLabel("游戏加载中", this); title->setAlignment(Qt::AlignCenter); title->setStyleSheet("color:white;font-size:20px;font-weight:bold;");
    m_progressBar = new QProgressBar(this); m_progressBar->setRange(0, 100); m_progressBar->setTextVisible(false);
    m_progressBar->setStyleSheet("QProgressBar{border:2px solid white;border-radius:10px;background:rgba(30,30,50,200);}QProgressBar::chunk{background:#4CAF50;border-radius:8px;}");
    m_tipLabel = new QLabel(getRandomTip(), this); m_tipLabel->setWordWrap(true); m_tipLabel->setStyleSheet("color:#ccc;font-size:14px;");
    lay->addWidget(title); lay->addWidget(m_progressBar); lay->addWidget(m_tipLabel);
}

void LoadingWidget::startLoading()
{
    m_progressValue = 0; m_progressBar->setValue(0);
    m_tipLabel->setText(getRandomTip());
    m_timer->start();
}

QString LoadingWidget::getRandomTip() const
{
    QStringList tips = {"破译密码机时小心监管者靠近！", "医生可以治疗自己和队友。", "机械师的傀儡能吸引注意力。", "空军信号枪可眩晕监管者。", "躲在草丛中可暂时隐身。"};
    return tips[QRandomGenerator::global()->bounded(tips.size())];
}