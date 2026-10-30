// 全局刷怪数量倍率：在服务端拍入口、秘境处理之前调用。
// Global monster-count multiplier: call at the server tick entry, before Rift setup.
#pragma once
#include <stdint.h>
void dm_spawn_tick(uint64_t game);
