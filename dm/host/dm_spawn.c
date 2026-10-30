#include "dm_spawn.h"
#include "dm_custom.h"
#include "dm.h"
#include <string.h>

// 原版建局后把当前难度的关卡表复制到 G 的区域数组。普通怪密度是 i32，
// 精英怪是每个区域的组数下/上限（u8），不是金怪单位的精确数量。
// A game copies the current difficulty's level records into G's region array.
// These fields govern ordinary density and elite pack count, not an exact monster count.
enum { REGION_COUNT = 140, DENSITY_MAX = 10000 };
static struct {
    uint64_t game, region[REGION_COUNT];
    uint32_t last_tick;
    int quarter;
} state;

static int scaled(int value, int quarter, int max) {
    int64_t n = ((int64_t)value * quarter + 2) / 4;
    return n > max ? max : (int)n;
}

void dm_spawn_tick(uint64_t game) {
    if (!game) return;
    uint32_t tick = *(uint32_t *)(uintptr_t)(game + 0x170);
    if (game != state.game || tick < state.last_tick) {
        static const int q[] = { 2, 4, 6, 8 };  // 0.5×, 1×, 1.5×, 2×
        int choice = dm_custom_get("spawn_rate");
        memset(&state, 0, sizeof state);
        state.game = game;
        state.quarter = q[choice >= 0 && choice < 4 ? choice : 1];
        LOG("怪物数量：本局 %s（普通怪密度与精英组数；地图首次生成时生效）",
            (const char *const[]){ "0.5×", "1×", "1.5×", "2×" }[choice >= 0 && choice < 4 ? choice : 1]);
    }
    state.last_tick = tick;
    if (state.quarter == 4) return;  // 缺省档逐字节保持原版，避免影响对拍 | exact original values by default

    int changed = 0;
    for (int level = 1; level < REGION_COUNT; level++) {
        uint64_t region = *(uint64_t *)(uintptr_t)(game + 0x1d0 + 8ull * level);
        if (!region || state.region[level] == region) continue;
        int duplicate = 0;
        for (int earlier = 1; earlier < level; earlier++) if (state.region[earlier] == region) { duplicate = 1; break; }
        if (duplicate) { state.region[level] = region; continue; }
        int32_t *density = (int32_t *)(uintptr_t)(region + 0x2c4);
        uint8_t *minimum = (uint8_t *)(uintptr_t)(region + 0x2c8);
        uint8_t *maximum = (uint8_t *)(uintptr_t)(region + 0x2c9);
        if (*density < 0 || *minimum > *maximum) { LOG("【红】怪物数量：关卡 %d 的原始区域值异常，保持原样", level); continue; }
        state.region[level] = region;
        *density = scaled(*density, state.quarter, DENSITY_MAX);
        *minimum = (uint8_t)scaled(*minimum, state.quarter, 255);
        *maximum = (uint8_t)scaled(*maximum, state.quarter, 255);
        changed++;
    }
    if (changed) LOG("怪物数量：本局已设置 %d 个区域；预设怪与任务首领不受影响", changed);
}
