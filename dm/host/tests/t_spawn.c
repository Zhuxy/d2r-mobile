#include "../dm_spawn.h"
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static int choice;
int dm_custom_get(const char *key) { assert(!strcmp(key, "spawn_rate")); return choice; }

static void fields(uint8_t *r, int den, int lo, int hi) {
    *(int32_t *)(r + 0x2c4) = den;
    r[0x2c8] = (uint8_t)lo;
    r[0x2c9] = (uint8_t)hi;
}
static void check(const uint8_t *r, int den, int lo, int hi) {
    assert(*(const int32_t *)(r + 0x2c4) == den);
    assert(r[0x2c8] == lo && r[0x2c9] == hi);
}
static void region(uint8_t *g, int level, uint8_t *r) { *(uint64_t *)(g + 0x1d0 + 8 * level) = (uint64_t)(uintptr_t)r; }

int main(void) {
    uint8_t *g = calloc(1, 0x800), *g2 = calloc(1, 0x800);
    uint8_t *a = calloc(1, 0x400), *b = calloc(1, 0x400), *late = calloc(1, 0x400);
    assert(g && g2 && a && b && late);
    choice = 2; *(uint32_t *)(g + 0x170) = 3;
    fields(a, 3000, 2, 4); fields(b, 9000, 200, 250);
    region(g, 2, a); region(g, 3, a); region(g, 4, b); // 同一指针出现在两关，不得乘两次
    dm_spawn_tick((uint64_t)(uintptr_t)g);
    check(a, 4500, 3, 6); check(b, 10000, 255, 255);

    choice = 0; *(uint32_t *)(g + 0x170) = 4; // 游戏内改设置，旧局保持原倍率
    dm_spawn_tick((uint64_t)(uintptr_t)g);
    check(a, 4500, 3, 6);
    fields(late, 400, 1, 2); region(g, 5, late);
    dm_spawn_tick((uint64_t)(uintptr_t)g); // 后出现的区域仍按本局锁定的 1.5×
    check(late, 600, 2, 3);

    *(uint32_t *)(g + 0x170) = 1; // 同一个 G 地址的新局，拍号倒退
    fields(a, 3000, 2, 4); fields(b, 9000, 200, 250);
    dm_spawn_tick((uint64_t)(uintptr_t)g);
    check(a, 1500, 1, 2); check(b, 4500, 100, 125);

    choice = 1; *(uint32_t *)(g2 + 0x170) = 1;
    fields(a, 3000, 2, 4); region(g2, 2, a);
    dm_spawn_tick((uint64_t)(uintptr_t)g2);
    check(a, 3000, 2, 4); // 原版档不改字节
    choice = 3; *(uint32_t *)(g2 + 0x170) = 0;
    fields(a, 2200, 0, 3);
    dm_spawn_tick((uint64_t)(uintptr_t)g2);
    check(a, 4400, 0, 6); // 2×，原值 0 的精英下限仍为 0
    free(g); free(g2); free(a); free(b); free(late);
    puts("t_spawn OK");
    return 0;
}
