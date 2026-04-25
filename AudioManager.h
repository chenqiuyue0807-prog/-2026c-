#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QString>

class AudioManager
{
public:
    enum SoundEffect { Footstep, Decode, Attack, Skill, Rescue, GateOpen, Hit, CipherComplete, Escape };

    static AudioManager* instance() {
        static AudioManager m;
        return &m;
    }

    void initialize() {}
    void playBGM(const QString& = QString()) {}
    void stopBGM() {}
    void playSound(SoundEffect) {}
    void setMasterVolume(int) {}
    void setBgmVolume(int) {}
    void setSfxVolume(int) {}
    int masterVolume() const { return 80; }
    int bgmVolume() const { return 70; }
    int sfxVolume() const { return 90; }
    void loadVolumes() {}
    void saveVolumes() {}

private:
    AudioManager() = default;
};

#endif