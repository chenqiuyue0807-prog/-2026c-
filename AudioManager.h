#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QSoundEffect>

class AudioManager : public QObject
{
    Q_OBJECT

public:
    enum MusicTrack {
        MainMenu,
        GameBGM,
        HunterNearby,
        HunterChase,
        GameOver
    };

    static AudioManager* instance();
    void initialize();
    void playMusic(MusicTrack track);
    void stopMusic();

    void setMasterVolume(int vol);    // 0-100
    void setBGMVolume(int vol);       // 0-100
    int masterVolume() const { return m_masterVolume; }
    int bgmVolume() const { return m_bgmVolume; }

    void loadVolumes();
    void saveVolumes();

private:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    QString musicPath(MusicTrack track) const;

    QSoundEffect *m_bgmEffect = nullptr;
    int m_masterVolume = 80;
    int m_bgmVolume = 70;
    MusicTrack m_currentTrack = MainMenu;
};

#endif // AUDIOMANAGER_H