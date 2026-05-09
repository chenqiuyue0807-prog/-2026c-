#ifndef GAMECONFIG_H
#define GAMECONFIG_H

#include <QPointF>
#include <QList>
#include <QtGlobal>

enum class SurvivorType { Doctor, Mechanic, AirForce };
enum class ObstacleType { Wall, Box, Board };
enum class Direction { Up, Down, Left, Right, None };

class GameConfig {
public:
    // ---------- 地图尺寸 ----------
    static constexpr int MAP_WIDTH = 2400;
    static constexpr int MAP_HEIGHT = 1800;

    // ---------- 移动速度（像素/帧）----------
    static constexpr qreal SURVIVOR_BASE_SPEED = 5.0;
    static constexpr qreal HUNTER_BASE_SPEED = 4.0;

    // 受伤影响
    static constexpr qreal HURT_SPEED_RATIO = 0.8;
    static constexpr qreal HURT_DECODE_RATIO = 0.85;

    // 逃脱阶段监管者加速
    static constexpr qreal ESCAPE_HUNTER_SPEED_RATIO = 1.2;
    static constexpr int HUNTER_STUN_AFTER_ATTACK = 60; // 攻击命中后僵直帧数

    // ---------- 交互距离 ----------
    static constexpr int INTERACT_CIPHER_DIST = 30;
    static constexpr int INTERACT_RESCUE_DIST = 30;
    static constexpr int INTERACT_GATE_DIST = 50;   // 原来是 30，加大后 AI 不容易擦肩而过
    static constexpr int INTERACT_DESTROY_DIST = 20;

    // ---------- 草丛 ----------
    static constexpr int BUSH_SHAKE_DIST = 50;
    static constexpr int BUSH_REVEAL_DURATION = 2;

    // ---------- AI 参数 ----------
    static constexpr int AI_HUNTER_CHASE_DIST = 250;   // 从150提高到250，更容易发现幸存者
    static constexpr int AI_HUNTER_LOSE_DIST = 380;    // 从280提高，避免追两步就丢
    static constexpr int AI_HUNTER_LOSE_TIME = 4;      // 减小丢失时间，更快放弃搜索
    static constexpr int AI_HUNTER_SEARCH_TIME = 2;    // 缩短搜索徘徊时间

    // ---------- 帧率相关 ----------
    static constexpr int FRAMES_PER_SECOND = 60;
    // ---------- 监管者攻击 ----------
    static constexpr int ATTACK_RANGE = 80;
    static constexpr int ATTACK_ANGLE = 90;
    static constexpr int ATTACK_HIT_ANGLE_TOLERANCE = 55;
    static constexpr int ATTACK_COOLDOWN_FRAMES = 5 * FRAMES_PER_SECOND; // 5秒冷却
    static constexpr int FRAMES_ATTACK_COOLDOWN = 5 * FRAMES_PER_SECOND; // 保持一致
    static constexpr int HUNTER_ATTACK_STUN_FRAMES = 0;

    // ---------- 破坏障碍物时间 ----------
    static constexpr float DESTROY_WALL_TIME = 3.0f;
    static constexpr float DESTROY_BOX_TIME = 2.0f;
    static constexpr float DESTROY_BOARD_TIME = 1.0f;

    // ---------- 技能冷却（秒）----------
    static constexpr int DOCTOR_COOLDOWN = 18;
    static constexpr int MECHANIC_COOLDOWN = 22;
    static constexpr int AIRFORCE_COOLDOWN = 30;

    static constexpr int AIRFORCE_STUN_DURATION = 3;
    static constexpr int MECHANIC_PUPPET_DURATION = 15;
    static constexpr qreal PUPPET_DECODE_SPEED_RATIO = 0.7;
    static constexpr int DOCTOR_HEAL_TIME = 2;

    // ---------- 求生者状态 ----------
    static constexpr int SURVIVOR_MAX_HEALTH = 2;
    static constexpr int BURN_DURATION = 30;
    static constexpr int RESCUE_TIME = 2;

    // ---------- 自救系统 ----------
    static constexpr int SELF_REVIVE_TIME = 15;      // 倒地后自救等待时间（秒）
    static constexpr int MAX_DOWN_COUNT = 2;          // 最大自救次数

    // ---------- 破译与大门 ----------
    static constexpr int CIPHER_COUNT = 3;
    static constexpr int SINGLE_CIPHER_DECODE_TIME = 40;
    static constexpr int GATE_OPEN_TIME = 8;
    static constexpr int MAX_SURVIVORS_PER_CIPHER = 2;
    static constexpr qreal DECODE_SPEED_BOOST_PER_EXTRA = 1.3;

    static constexpr int DECODE_MISTAKE_INTERVAL = 5;
    static constexpr int DECODE_MISTAKE_CHANCE = 20;
    static constexpr qreal DECODE_MISTAKE_PENALTY = 0.05;
    static constexpr int DECODE_MISTAKE_REVEAL_TIME = 3;

    static constexpr qreal GATE_STAGE1_RATIO = 0.3;
    static constexpr qreal GATE_STAGE2_RATIO = 0.7;
    static constexpr qreal GATE_STAGE3_RATIO = 1.0;

    // ---------- 帧率相关 ----------
    static int secondsToFrames(float seconds) { return static_cast<int>(seconds * FRAMES_PER_SECOND); }

    static constexpr int FRAMES_PREPARATION = 10 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DECODING = 240 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_ESCAPE = 60 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_BURN = 30 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DOCTOR_COOLDOWN = 18 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_MECHANIC_COOLDOWN = 22 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_AIRFORCE_COOLDOWN = 30 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_AIRFORCE_STUN = 3 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DOCTOR_HEAL = 2 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_RESCUE = 2 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DESTROY_WALL = 3 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DESTROY_BOX = 2 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_DESTROY_BOARD = 1 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_PUPPET_LIFETIME = 15 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_MISTAKE_REVEAL = 3 * FRAMES_PER_SECOND;
    static constexpr int FRAMES_BUSH_REVEAL = 2 * FRAMES_PER_SECOND;

    static constexpr int FRAMES_SELF_REVIVE = SELF_REVIVE_TIME * FRAMES_PER_SECOND;

    // ---------- 固定坐标 ----------
    static QList<QPointF> getCipherPositions() {
        return { QPointF(400, 600), QPointF(1200, 800), QPointF(1900, 1400) };
    }
    static QList<QPointF> getGatePositions() {
        return { QPointF(80, 900), QPointF(2320, 900) };
    }
    static QPointF getHunterSpawnPoint() { return QPointF(MAP_WIDTH/2.0, MAP_HEIGHT/2.0); }
    static QPointF getSurvivorSpawnPoint(int index) {
        static const QList<QPointF> spawns = { QPointF(200,200), QPointF(2200,200), QPointF(200,1600) };
        if (index>=0 && index<spawns.size()) return spawns[index];
        return QPointF(200,200);
    }
};
#endif