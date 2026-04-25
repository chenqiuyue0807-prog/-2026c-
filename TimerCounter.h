#ifndef TIMERCOUNTER_H
#define TIMERCOUNTER_H

#include <QObject>

class TimerCounter : public QObject
{
    Q_OBJECT
public:
    explicit TimerCounter(QObject *parent = nullptr);
    void tick();                    // 每帧调用，帧计数+1（未暂停时）
    void reset();                   // 重置计数为0
    void pause();
    void resume();
    bool isPaused() const { return m_paused; }
    long long frameCount() const { return m_frameCount; }

    static int secondsToFrames(float seconds);
    static float framesToSeconds(int frames);
    bool hasElapsed(long long intervalFrames) const; // 是否经过指定帧数

signals:
    void ticked(long long frameCount);

private:
    long long m_frameCount = 0;
    bool m_paused = false;
};

#endif