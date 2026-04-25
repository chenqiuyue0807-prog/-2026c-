#include "CharacterSelectWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

CharacterSelectWidget::CharacterSelectWidget(QWidget *parent) : QWidget(parent), m_cardGroup(new QButtonGroup(this))
{
    setWindowTitle("选择角色"); setFixedSize(900, 600);
    setStyleSheet("background-color: rgb(20,20,40);");
    m_cardNormal = "QPushButton{background:rgba(40,40,70,200);border:2px solid gray;border-radius:15px;color:white;}QPushButton:hover{background:rgba(70,70,120,220);}";
    m_cardSelected = "QPushButton{background:rgba(100,100,180,220);border:3px solid yellow;border-radius:15px;color:white;}";
    setupUI();
    m_cardGroup->button(0)->setChecked(true);
}

void CharacterSelectWidget::setupUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);
    QLabel *title = new QLabel("选择你的求生者", this);
    title->setAlignment(Qt::AlignCenter); title->setStyleSheet("font-size:28px;font-weight:bold;color:white;");
    main->addWidget(title);

    QHBoxLayout *cardRow = new QHBoxLayout;
    createCard(0, "医生", "治疗自己或队友，恢复1点生命值\n冷却:20秒", SurvivorType::Doctor);
    createCard(1, "机械师", "释放傀儡自动破译密码机\n破译速度70%，持续15秒\n冷却:25秒", SurvivorType::Mechanic);
    createCard(2, "空军", "发射信号枪眩晕监管者3秒\n射程120像素，需无遮挡\n冷却:35秒", SurvivorType::AirForce);
    for (int i = 0; i < 3; ++i) cardRow->addWidget(m_cardGroup->button(i));
    main->addLayout(cardRow);

    QFrame *hunterFrame = new QFrame(this);
    hunterFrame->setFixedSize(200, 200);
    hunterFrame->setStyleSheet("background:rgba(60,60,60,150);border:2px solid gray;border-radius:15px;");
    QVBoxLayout *hunterLayout = new QVBoxLayout(hunterFrame);
    QLabel *hName = new QLabel("厂长 (监管者)", this); hName->setAlignment(Qt::AlignCenter); hName->setStyleSheet("color:white;font-size:16px;");
    QLabel *hAI = new QLabel("AI 控制", this); hAI->setAlignment(Qt::AlignCenter); hAI->setStyleSheet("color:orange;");
    hunterLayout->addWidget(hName); hunterLayout->addWidget(hAI);
    QHBoxLayout *hRow = new QHBoxLayout; hRow->addStretch(); hRow->addWidget(hunterFrame); hRow->addStretch();
    main->addLayout(hRow);

    QHBoxLayout *btnRow = new QHBoxLayout;
    QPushButton *okBtn = new QPushButton("确认选择", this);
    QPushButton *backBtn = new QPushButton("返回", this);
    btnRow->addStretch(); btnRow->addWidget(backBtn); btnRow->addWidget(okBtn); btnRow->addStretch();
    main->addLayout(btnRow);

    connect(m_cardGroup, QOverload<int>::of(&QButtonGroup::idClicked), this, [this](int id){
        m_selectedId = id;
        m_selectedType = static_cast<SurvivorType>(id);
        for (int i = 0; i < 3; ++i) {
            auto *btn = qobject_cast<QPushButton*>(m_cardGroup->button(i));
            if (btn) btn->setStyleSheet(i == id ? m_cardSelected : m_cardNormal);
        }
    });
    connect(okBtn, &QPushButton::clicked, this, [this](){ emit characterSelected(m_selectedType); close(); });
    connect(backBtn, &QPushButton::clicked, this, [this](){ emit backClicked(); close(); });
}

void CharacterSelectWidget::createCard(int id, const QString &name, const QString &desc, SurvivorType /*type*/)
{
    QPushButton *card = new QPushButton(this);
    card->setFixedSize(200, 300);
    card->setCheckable(true);
    card->setStyleSheet(m_cardNormal);
    m_cardGroup->addButton(card, id);
    QVBoxLayout *lay = new QVBoxLayout(card);
    QLabel *nLabel = new QLabel(name, card); nLabel->setAlignment(Qt::AlignCenter); nLabel->setStyleSheet("font-size:18px;font-weight:bold;");
    QLabel *dLabel = new QLabel(desc, card); dLabel->setWordWrap(true); dLabel->setStyleSheet("color:#ccc;");
    lay->addWidget(nLabel); lay->addWidget(dLabel); lay->addStretch();
}