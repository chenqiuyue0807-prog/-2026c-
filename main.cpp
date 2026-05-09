#include "GameEngine.h"
#include "ui/CharacterSelectWidget.h"
#include "ui/GameUI.h"
#include "ui/HelpWidget.h"
#include "ui/LoadingWidget.h"
#include "ui/MainMenuWidget.h"
#include "ui/SettingsWidget.h"
#include "utils/AudioManager.h"
#include <QApplication>
#include <QEventLoop>

static GameEngine *g_engine = nullptr;
static GameUI *g_gameUI = nullptr;
static MainMenuWidget *g_menu = nullptr;

// 辅助函数：为任意 QWidget 提供模态效果（使用事件循环）
void execWidget(QWidget *w)
{
    w->show();
    QEventLoop loop;
    QObject::connect(w, &QWidget::destroyed, &loop, &QEventLoop::quit);
    loop.exec();
}

void startGameWithSurvivor(SurvivorType type)
{
    if (g_engine) delete g_engine;
    if (g_gameUI) delete g_gameUI;

    g_engine = new GameEngine;
    g_engine->startGame(static_cast<int>(type));

    g_gameUI = new GameUI;
    g_gameUI->setGameEngine(g_engine);
    g_gameUI->initGame();
    g_gameUI->show();

    // 游戏窗口关闭或返回主菜单时，清理并重新显示菜单
    QObject::connect(g_gameUI, &GameUI::backToMainMenu, []() {
        if (g_gameUI) {
            g_gameUI->hide();
            delete g_gameUI;
            g_gameUI = nullptr;
        }
        if (g_engine) {
            delete g_engine;
            g_engine = nullptr;
        }
        if (g_menu) {
            g_menu->show();
            // 返回主菜单时播放主菜单音乐
            AudioManager::instance()->playMusic(AudioManager::MainMenu);
        }
    });
}

int main(int argc, char *argv[])
{
    qputenv("QT_MEDIA_BACKEND", "windows");
    QApplication a(argc, argv);

    // ======== 初始化音频管理器并启动主菜单音乐 ========
    AudioManager::instance()->initialize();
    AudioManager::instance()->playMusic(AudioManager::MainMenu);

    MainMenuWidget menu;
    g_menu = &menu;

    // 开始游戏（完整流程：角色选择 → 加载 → 游戏）
    QObject::connect(&menu, &MainMenuWidget::startGameClicked, [&]() {
        CharacterSelectWidget charSelect;
        charSelect.show();
        QEventLoop loop;
        SurvivorType selectedType = SurvivorType::Doctor;
        bool selected = false;

        QObject::connect(&charSelect, &CharacterSelectWidget::characterSelected, [&](SurvivorType type) {
            selectedType = type;
            selected = true;
            loop.quit();
        });
        QObject::connect(&charSelect, &CharacterSelectWidget::backClicked, [&]() {
            loop.quit();
        });
        loop.exec();

        if (selected) {
            LoadingWidget loading;
            loading.startLoading();
            loading.show();
            QEventLoop loadingLoop;
            QObject::connect(&loading, &LoadingWidget::loadingFinished, [&]() {
                loading.close();
                startGameWithSurvivor(selectedType);
                menu.hide();
            });
            QObject::connect(&loading, &LoadingWidget::loadingFinished, &loadingLoop, &QEventLoop::quit);
            loadingLoop.exec();
        }
    });

    // 设置
    QObject::connect(&menu, &MainMenuWidget::settingsClicked, [&]() {
        SettingsWidget settings;
        execWidget(&settings);
    });

    // 帮助
    QObject::connect(&menu, &MainMenuWidget::helpClicked, [&]() {
        HelpWidget help;
        execWidget(&help);
    });

    // 退出
    QObject::connect(&menu, &MainMenuWidget::quitClicked, &a, &QApplication::quit);

    menu.show();
    return a.exec();
}