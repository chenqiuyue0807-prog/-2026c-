#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <QPointF>
#include <QList>

// 角色类型枚举
enum class SurvivorType {
    Doctor,
    Mechanic,
    AirForce
};

enum class ObstacleType {
    Wall,
    Box,
    Board
};

enum class Direction {
    Up,
    Down,
    Left,
    Right,
    None
};

class GameConfig {
public:
    // ---------- 地图尺寸 ----------
    static constexpr int MAP_WIDTH = 1200;
    static constexpr int MAP_HEIGHT = 800;

    // ---------- 移动速度（像素/帧）----------
    static constexpr qreal SURVIVOR_BASE_SPEED = 3.0;
    static constexpr qreal HUNTER_BASE_SPEED = 3.8;

    // 受伤影响
    static constexpr qreal HURT_SPEED_RATIO = 0.8;
    static constexpr qreal HURT_DECODE_RATIO = 0.85;

    // 逃脱阶段监管者加速
    static constexpr qreal ESCAPE_HUNTER_SPEED_RATIO = 1.2;

    // ---------- 交互距离（像素）----------
    static constexpr int INTERACT_CIPHER_DIST = 30;
    static constexpr int INTERACT_RESCUE_DIST = 30;
    static constexpr int INTERACT_GATE_DIST = 30;
    static constexpr int INTERACT_DESTROY_DIST = 20;

    // ---------- 草丛相关 ----------
    static constexpr int BUSH_SHAKE_DIST = 50;        // 监管者靠近草丛晃动距离
    static constexpr int BUSH_REVEAL_DURATION = 2;    // 扫草显形持续时间(秒)

    // ---------- AI 参数 ----------
    static constexpr int AI_HUNTER_CHASE_DIST = 120;  // 追击触发距离
    static constexpr int AI_HUNTER_LOSE_DIST = 200;   // 放弃追击距离
    static constexpr int AI_HUNTER_LOSE_TIME = 8;     // 丢失目标时间(秒)
    static constexpr int AI_HUNTER_SEARCH_TIME = 4;   // 搜索时间(秒)

    // ---------- 监管者攻击 ----------
    static constexpr int ATTACK_RANGE = 60;           // 攻击半径
    static constexpr int ATTACK_ANGLE = 90;           // 扇形角度
    static constexpr int ATTACK_HIT_ANGLE_TOLERANCE = 45; // 命中夹角容差
    static constexpr int ATTACK_COOLDOWN_FRAMES = 480;    // 8秒 * 60FPS
    static constexpr int HUNTER_ATTACK_STUN_FRAMES = 60;  // 攻击命中后自身停顿1秒

    // ---------- 破坏障碍物时间（秒）----------
    static constexpr float DESTROY_WALL_TIME = 3.0f;
    static constexpr float DESTROY_BOX_TIME = 2.0f;
    static constexpr float DESTROY_BOARD_TIME = 1.0f;

    // ---------- 技能冷却（秒）----------
    static constexpr int DOCTOR_COOLDOWN = 20;
    static constexpr int MECHANIC_COOLDOWN = 25;
    static constexpr int AIRFORCE_COOLDOWN = 35;

    static constexpr int AIRFORCE_STUN_DURATION = 3;   // 眩晕监管者秒数
    static constexpr int MECHANIC_PUPPET_DURATION = 15; // 傀儡存在秒数
    static constexpr qreal PUPPET_DECODE_SPEED_RATIO = 0.7;
    static constexpr int DOCTOR_HEAL_TIME = 2;

    // ---------- 求生者状态 ----------
    static constexpr int SURVIVOR_MAX_HEALTH = 2;
    static constexpr int BURN_DURATION = 30;          // 燃烧时间(秒)
    static constexpr int RESCUE_TIME = 2;              // 救助耗时(秒)

    // ---------- 破译与大门 ----------
    static constexpr int CIPHER_COUNT = 3;
    static constexpr int SINGLE_CIPHER_DECODE_TIME = 45;   // 单台破译秒数
    static constexpr int GATE_OPEN_TIME = 10;              // 大门总开启秒数
    static constexpr int MAX_SURVIVORS_PER_CIPHER = 2;     // 最多同时破译人数
    static constexpr qreal DECODE_SPEED_BOOST_PER_EXTRA = 1.3; // 额外每人提速倍率

    // 破译失误
    static constexpr int DECODE_MISTAKE_INTERVAL = 5;          // 检测间隔秒数
    static constexpr int DECODE_MISTAKE_CHANCE = 20;           // 触发概率(%)
    static constexpr qreal DECODE_MISTAKE_PENALTY = 0.05;      // 进度倒退比例
    static constexpr int DECODE_MISTAKE_REVEAL_TIME = 3;       // 暴露持续时间(秒)

    // 大门分段比例
    static constexpr qreal GATE_STAGE1_RATIO = 0.3;
    static constexpr qreal GATE_STAGE2_RATIO = 0.7;
    static constexpr qreal GATE_STAGE3_RATIO = 1.0;

    // ---------- 帧率相关 ----------
    static constexpr int FRAMES_PER_SECOND = 60;

    static int secondsToFrames(float seconds) {
        return static_cast<int>(seconds * FRAMES_PER_SECOND);
    }

    // 常用帧数常量（直接使用，无需计算）
    static constexpr int FRAMES_PREPARATION = 10 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DECODING = 240 * FRAMES_PER_SECOND;    // 4分钟
    static constexpr int FRAMES_ESCAPE = 60 * FRAMES_PER_SECOND;       // 1分钟
    static constexpr int FRAMES_BURN = 30 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_ATTACK_COOLDOWN = 8 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DOCTOR_COOLDOWN = 20 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_MECHANIC_COOLDOWN = 25 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_AIRFORCE_COOLDOWN = 35 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_AIRFORCE_STUN = 3 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DOCTOR_HEAL = 2 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_RESCUE = 2 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DESTROY_WALL = 3 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DESTROY_BOX = 2 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DESTROY_BOARD = 1 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_PUPPET_LIFETIME = 15 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_MISTAKE_REVEAL = 3 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_BUSH_REVEAL = 2 * FRAMES_PER_SECOND;

    // ---------- 固定坐标 ----------
    static QList<QPointF> getCipherPositions() {
        return {
            QPointF(300, 200),
            QPointF(600, 400),
            QPointF(900, 600)
        };
    }

    static QList<QPointF> getGatePositions() {
        return {
            QPointF(50, 400),
            QPointF(1150, 400)
        };
    }

    static QPointF getHunterSpawnPoint() {
        return QPointF(MAP_WIDTH / 2.0, MAP_HEIGHT / 2.0);
    }

    static QPointF getSurvivorSpawnPoint(int index) {
        static const QList<QPointF> spawns = {
            QPointF(100, 100),   // 左上
            QPointF(1100, 100),  // 右上
            QPointF(100, 700)    // 左下
        };
        if (index >= 0 && index < spawns.size())
            return spawns[index];
        return QPointF(200, 200); // fallback
    }
};
#endif