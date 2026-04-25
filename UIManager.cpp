#include "UIManager.h"
#include "GameEngine.h"
#include "ui/GameHUDWidget.h"
#include "entities/PlayerSurvivor.h"
#include "entities/Gate.h"

UIManager::UIManager(QObject *parent) : QObject(parent) {}
void UIManager::setEngine(GameEngine *e) { m_engine = e; }
void UIManager::setHUD(GameHUDWidget *h) { m_hud = h; }
void UIManager::setPlayer(PlayerSurvivor *p) { m_player = p; }

void UIManager::connectSignals() {
    if (!m_engine || !m_hud) return;
    connect(m_engine, &GameEngine::timeUpdated, this, &UIManager::onTimeUpdated);
    connect(m_engine, &GameEngine::cipherProgressChanged, this, &UIManager::onCipherChanged);
    connect(m_engine, &GameEngine::gameStateChanged, this, &UIManager::onGameStateChanged);
    if (m_player) {
        connect(m_player, &Survivor::healthChanged, this, &UIManager::onHealthChanged);
        connect(m_player, &PlayerSurvivor::skillUsed, this, &UIManager::onSkillUsed);
        connect(m_player, &PlayerSurvivor::interactionPrompt, this, &UIManager::onPrompt);
    }
    refresh();
}

void UIManager::refresh() {
    if (!m_engine || !m_hud) return;
    onTimeUpdated(m_engine->remainingTime());
    onCipherChanged(m_engine->completedCipherCount(), 3);
    if (m_player) {
        onHealthChanged(m_player->health());
        m_hud->updateSkillCooldown(m_player->skillCooldownRatio());
    }
    updateGateStatus();
}

void UIManager::onTimeUpdated(int sec) { if(m_hud) m_hud->updateTimeDisplay(sec); }
void UIManager::onCipherChanged(int c, int t) { if(m_hud) m_hud->updateCipherProgress(c, t); }
void UIManager::onGameStateChanged(int s) { if(s==2 && m_hud) m_hud->updateGateStatus(true,0); }
void UIManager::onHealthChanged(int h) { if(m_hud) m_hud->updateHealthDisplay(h); }
void UIManager::onSkillUsed() { if(m_hud && m_player) m_hud->updateSkillCooldown(m_player->skillCooldownRatio()); }
void UIManager::onPrompt(const QString &t) { if(m_hud) m_hud->showMessage(t,1500); }
void UIManager::updateGateStatus() {
    if (!m_engine || !m_hud) return;
    auto gates = m_engine->getGates();
    bool unlocked = false; int progress = 0;
    for (auto *g : gates) if (g->isUnlocked()) { unlocked = true; progress = qMax(progress, g->progress()); }
    m_hud->updateGateStatus(unlocked, progress);
}