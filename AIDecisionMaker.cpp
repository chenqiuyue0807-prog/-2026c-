#include "AIDecisionMaker.h"
#include "ai/HunterBehavior.h"
#include "ai/SurvivorBehavior.h"

AIDecisionMaker::AIDecisionMaker(QObject *parent)
    : QObject(parent)
    , m_scene(nullptr)
    , m_hunter(nullptr)
    , m_player(nullptr)
    , m_hunterBehavior(nullptr)
    , m_survivorBehavior(nullptr)
{
    m_timer.setInterval(50);
    connect(&m_timer, &QTimer::timeout, this, &AIDecisionMaker::makeDecision);
}

AIDecisionMaker::~AIDecisionMaker() { stop(); }
void AIDecisionMaker::start() { m_timer.start(); }
void AIDecisionMaker::stop() { m_timer.stop(); }
void AIDecisionMaker::reset() { stop(); }

void AIDecisionMaker::makeDecision()
{
    if (m_hunterBehavior)
        m_hunterBehavior->updateDecision();

    if (m_survivorBehavior) {
        for (AISurvivor *ai : m_aiSurvivors)
            m_survivorBehavior->updateDecision(ai);
    }
}