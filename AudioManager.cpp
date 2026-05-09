#include "AudioManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QDebug>

// ---------- 单例 ----------
AudioManager* AudioManager::instance()
{
    static AudioManager inst;
    return &inst;
}

AudioManager::AudioManager(QObject *parent) : QObject(parent) {}

AudioManager::~AudioManager()
{
    if (m_bgmEffect) {
        m_bgmEffect->stop();
        delete m_bgmEffect;
        m_bgmEffect = nullptr;
    }
}

void AudioManager::initialize()
{
    if (!m_bgmEffect) {
        m_bgmEffect = new QSoundEffect(this);
        m_bgmEffect->setLoopCount(QSoundEffect::Infinite);   // 无限循环
        m_bgmEffect->setVolume(m_masterVolume * m_bgmVolume / 10000.0);
    }
}

void AudioManager::playMusic(MusicTrack track)
{
    if (m_currentTrack == track) return;
    m_currentTrack = track;

    if (!m_bgmEffect) initialize();

    // 先停止当前播放
    m_bgmEffect->stop();

    // 设置新的源
    m_bgmEffect->setSource(QUrl(musicPath(track)));

    // 播放
    m_bgmEffect->play();
}

void AudioManager::stopMusic()
{
    if (m_bgmEffect) m_bgmEffect->stop();
    m_currentTrack = MainMenu;
}

QString AudioManager::musicPath(MusicTrack track) const
{
    switch (track) {
    case MainMenu:     return "qrc:/new/prefix3/sound/zhu.wav";
    case GameBGM:      return "qrc:/new/prefix3/sound/youxi.wav";
    case HunterNearby: return "qrc:/new/prefix3/sound/xiaoxin.wav";
    case HunterChase:  return "qrc:/new/prefix3/sound/zhuiji.wav";
    case GameOver:     return "qrc:/new/prefix3/sound/jieshu.wav";
    default: return {};
    }
}

void AudioManager::setMasterVolume(int vol)
{
    m_masterVolume = qBound(0, vol, 100);
    setBGMVolume(m_bgmVolume);   // 重新计算实际音量
}

void AudioManager::setBGMVolume(int vol)
{
    m_bgmVolume = qBound(0, vol, 100);
    if (m_bgmEffect) {
        m_bgmEffect->setVolume(m_masterVolume * m_bgmVolume / 10000.0);
    }
}

void AudioManager::loadVolumes()
{
    QSettings s;
    m_masterVolume = s.value("audio/master", 80).toInt();
    m_bgmVolume    = s.value("audio/bgm", 70).toInt();
    setMasterVolume(m_masterVolume);
}

void AudioManager::saveVolumes()
{
    QSettings s;
    s.setValue("audio/master", m_masterVolume);
    s.setValue("audio/bgm", m_bgmVolume);
}