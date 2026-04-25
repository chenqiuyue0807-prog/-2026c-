#include "SettingsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QApplication>

SettingsWidget::SettingsWidget(QWidget *parent) : QWidget(parent), m_settings(new QSettings("MyGame", "Settings", this))
{
    setWindowTitle("设置"); setFixedSize(500, 500);
    setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint);
    setStyleSheet("background-color: rgb(30,30,50); color: white;");
    setupUI();
    loadSettings();
}

void SettingsWidget::setupUI()
{
    QVBoxLayout *main = new QVBoxLayout(this);
    QLabel *title = new QLabel("游戏设置", this); title->setAlignment(Qt::AlignCenter); title->setStyleSheet("font-size:24px; font-weight:bold;");
    main->addWidget(title);

    QGroupBox *volGroup = new QGroupBox("音量", this);
    QVBoxLayout *volLayout = new QVBoxLayout(volGroup);
    auto makeSlider = [&](const QString &name, QSlider *&slider) {
        QHBoxLayout *row = new QHBoxLayout;
        row->addWidget(new QLabel(name, this));
        slider = new QSlider(Qt::Horizontal, this); slider->setRange(0, 100);
        row->addWidget(slider);
        volLayout->addLayout(row);
    };
    makeSlider("主音量", m_masterSlider);
    makeSlider("背景音乐", m_bgmSlider);
    makeSlider("音效", m_sfxSlider);
    main->addWidget(volGroup);

    QGroupBox *dispGroup = new QGroupBox("显示", this);
    QVBoxLayout *dispLayout = new QVBoxLayout(dispGroup);
    m_fullscreenCheck = new QCheckBox("全屏模式", this);
    dispLayout->addWidget(m_fullscreenCheck);
    m_resolutionCombo = new QComboBox(this);
    QScreen *screen = QApplication::primaryScreen();
    QSize size = screen->size();
    m_resolutionCombo->addItem(QString("%1x%2").arg(size.width()).arg(size.height()), size);
    m_resolutionCombo->addItem("1280x720", QSize(1280,720));
    m_resolutionCombo->addItem("1920x1080", QSize(1920,1080));
    dispLayout->addWidget(m_resolutionCombo);
    main->addWidget(dispGroup);

    QHBoxLayout *btnRow = new QHBoxLayout;
    QPushButton *applyBtn = new QPushButton("应用", this);
    QPushButton *backBtn = new QPushButton("返回", this);
    btnRow->addStretch(); btnRow->addWidget(applyBtn); btnRow->addWidget(backBtn);
    main->addLayout(btnRow);

    connect(applyBtn, &QPushButton::clicked, this, &SettingsWidget::applySettings);
    connect(backBtn, &QPushButton::clicked, this, [this](){ emit backClicked(); close(); });
}

void SettingsWidget::applySettings()
{
    m_settings->setValue("audio/master", m_masterSlider->value());
    m_settings->setValue("audio/bgm", m_bgmSlider->value());
    m_settings->setValue("audio/sfx", m_sfxSlider->value());
    m_settings->setValue("display/fullscreen", m_fullscreenCheck->isChecked());
    m_settings->setValue("display/resolution", m_resolutionCombo->currentData().toSize());
}

void SettingsWidget::loadSettings()
{
    m_masterSlider->setValue(m_settings->value("audio/master", 80).toInt());
    m_bgmSlider->setValue(m_settings->value("audio/bgm", 70).toInt());
    m_sfxSlider->setValue(m_settings->value("audio/sfx", 90).toInt());
    m_fullscreenCheck->setChecked(m_settings->value("display/fullscreen", false).toBool());
    int idx = m_resolutionCombo->findData(m_settings->value("display/resolution", QSize(1280,720)).toSize());
    if (idx >= 0) m_resolutionCombo->setCurrentIndex(idx);
}