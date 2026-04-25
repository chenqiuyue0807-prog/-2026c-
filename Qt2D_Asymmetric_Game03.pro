QT += widgets multimedia
INCLUDEPATH += $$PWD/entities

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    GameEngine.cpp \
    GameScene.cpp \
    Widget.cpp \
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
    Widget.h \
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

FORMS +=

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
