#include "TimerCounter.h"

TimerCounter::TimerCounter(QObject *parent) : QObject(parent) {}

void TimerCounter::tick()
{
    if (!m_paused) {
        m_frameCount++;
        emit ticked(m_frameCount);
    }
}

void TimerCounter::reset() { m_frameCount = 0; }
void TimerCounter::pause() { m_paused = true; }
void TimerCounter::resume() { m_paused = false; }

int TimerCounter::secondsToFrames(float seconds) { return static_cast<int>(seconds * 60); }
float TimerCounter::framesToSeconds(int frames) { return frames / 60.0f; }
bool TimerCounter::hasElapsed(long long intervalFrames) const { return (m_frameCount % intervalFrames) == 0; }