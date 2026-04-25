#include "SaveManager.h"

SaveManager* SaveManager::m_instance = nullptr;

SaveManager* SaveManager::instance() {
    if (!m_instance) m_instance = new SaveManager;
    return m_instance;
}

SaveManager::SaveManager(QObject *parent) : QObject(parent) {}

void SaveManager::initialize() { if (!m_settings) m_settings = new QSettings("MyGame", "AsymmetricGame", this); }

void SaveManager::saveVolumeSettings(int m, int b, int s) { if(!m_settings) initialize(); m_settings->setValue("audio/master",m); m_settings->setValue("audio/bgm",b); m_settings->setValue("audio/sfx",s); }
void SaveManager::loadVolumeSettings(int &m, int &b, int &s) { if(!m_settings) initialize(); m=m_settings->value("audio/master",80).toInt(); b=m_settings->value("audio/bgm",70).toInt(); s=m_settings->value("audio/sfx",90).toInt(); }
void SaveManager::saveGameStats(const GameStats &st) {
    if(!m_settings) initialize();
    m_settings->setValue("stats/totalGames",st.totalGames);
    m_settings->setValue("stats/survivorWins",st.survivorWins);
    m_settings->setValue("stats/hunterWins",st.hunterWins);
    m_settings->setValue("stats/totalCiphers",st.totalCiphers);
    m_settings->setValue("stats/totalRescues",st.totalRescues);
    m_settings->setValue("stats/totalDestroys",st.totalDestroys);
    m_settings->setValue("stats/totalEscapes",st.totalEscapes);
    m_settings->setValue("stats/totalEliminations",st.totalEliminations);
}
void SaveManager::loadGameStats(GameStats &st) {
    if(!m_settings) initialize();
    st.totalGames = m_settings->value("stats/totalGames",0).toInt();
    st.survivorWins = m_settings->value("stats/survivorWins",0).toInt();
    st.hunterWins = m_settings->value("stats/hunterWins",0).toInt();
    st.totalCiphers = m_settings->value("stats/totalCiphers",0).toInt();
    st.totalRescues = m_settings->value("stats/totalRescues",0).toInt();
    st.totalDestroys = m_settings->value("stats/totalDestroys",0).toInt();
    st.totalEscapes = m_settings->value("stats/totalEscapes",0).toInt();
    st.totalEliminations = m_settings->value("stats/totalEliminations",0).toInt();
}
void SaveManager::addGameResult(bool w, int ciph, int resc, int dest, int esc, int elim) {
    GameStats st; loadGameStats(st);
    st.totalGames++; if(w) st.survivorWins++; else st.hunterWins++;
    st.totalCiphers += ciph; st.totalRescues += resc; st.totalDestroys += dest;
    st.totalEscapes += esc; st.totalEliminations += elim;
    saveGameStats(st);
}
void SaveManager::resetStats() { GameStats st; saveGameStats(st); }