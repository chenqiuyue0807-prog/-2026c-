#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include <QObject>
#include <QSettings>

class SaveManager : public QObject
{
    Q_OBJECT
public:
    static SaveManager* instance();
    void initialize();

    struct GameStats {
        int totalGames = 0, survivorWins = 0, hunterWins = 0;
        int totalCiphers = 0, totalRescues = 0, totalDestroys = 0;
        int totalEscapes = 0, totalEliminations = 0;
    };

    void saveVolumeSettings(int master, int bgm, int sfx);
    void loadVolumeSettings(int &master, int &bgm, int &sfx);
    void saveGameStats(const GameStats &stats);
    void loadGameStats(GameStats &stats);
    void addGameResult(bool survivorWin, int ciphers, int rescues, int destroys, int escapes, int eliminations);
    void resetStats();

private:
    explicit SaveManager(QObject *parent = nullptr);
    QSettings *m_settings = nullptr;
    static SaveManager *m_instance;
};

#endif