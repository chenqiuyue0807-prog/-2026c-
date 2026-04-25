#include <QApplication>
#include "ui/GameUI.h"
#include "GameEngine.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    GameEngine *engine = new GameEngine;
    engine->startGame(0);   // 0 = 医生
    GameUI ui;
    ui.setGameEngine(engine);
    ui.initGame();
    ui.show();
    return a.exec();
}