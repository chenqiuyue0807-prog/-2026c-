QT += core gui widgets multimedia

CONFIG += c++17
QT += core gui multimedia
CONFIG(release, debug|release) {
    DEFINES += QT_NO_DEBUG_OUTPUT
}

SOURCES += \
    GameEngine.cpp \
    GameScene.cpp \
    ai/AIDecisionMaker.cpp \
    ai/HunterBehavior.cpp \
    ai/SurvivorBehavior.cpp \
    entities/AISurvivor.cpp \
    entities/Character.cpp \
    entities/CipherMachine.cpp \
    entities/Gate.cpp \
    entities/Hunter.cpp \
    entities/MechanicalPuppet.cpp \
    entities/PlayerSurvivor.cpp \
    entities/Survivor.cpp \
    main.cpp \
    ui/CharacterSelectWidget.cpp \
    ui/GameHUDWidget.cpp \
    ui/GameUI.cpp \
    ui/HelpWidget.cpp \
    ui/LoadingWidget.cpp \
    ui/MainMenuWidget.cpp \
    ui/MiniMapWidget.cpp \
    ui/PauseWidget.cpp \
    ui/ResultWidget.cpp \
    ui/SettingsWidget.cpp \
    utils/AudioManager.cpp \
    utils/CameraFollow.cpp \
    utils/CharacterAnimator.cpp \
    utils/CollisionHelper.cpp \
    utils/MathHelper.cpp \
    utils/SaveManager.cpp \
    utils/TimerCounter.cpp \
    utils/UIManager.cpp

HEADERS += \
    GameConfig.h \
    GameEngine.h \
    GameScene.h \
    ai/AIDecisionMaker.h \
    ai/HunterBehavior.h \
    ai/SurvivorBehavior.h \
    entities/AISurvivor.h \
    entities/Character.h \
    entities/CipherMachine.h \
    entities/Gate.h \
    entities/Hunter.h \
    entities/MechanicalPuppet.h \
    entities/PlayerSurvivor.h \
    entities/Survivor.h \
    ui/CharacterSelectWidget.h \
    ui/GameHUDWidget.h \
    ui/GameUI.h \
    ui/HelpWidget.h \
    ui/LoadingWidget.h \
    ui/MainMenuWidget.h \
    ui/MiniMapWidget.h \
    ui/PauseWidget.h \
    ui/ResultWidget.h \
    ui/SettingsWidget.h \
    utils/AudioManager.h \
    utils/CameraFollow.h \
    utils/CharacterAnimator.h \
    utils/CollisionHelper.h \
    utils/MathHelper.h \
    utils/SaveManager.h \
    utils/TimerCounter.h \
    utils/UIManager.h

INCLUDEPATH += $$PWD

RESOURCES += \
    resources.qrc