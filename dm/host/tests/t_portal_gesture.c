#include "../../ios/Sources/dm_touch_controls.h"
#include <stdio.h>

static int bad, checks;
#define CHECK(c) do { ++checks; if (!(c)) { ++bad; printf("portal gesture failed at line %d\n", __LINE__); } } while (0)

int main(void) {
    dm_tc_state s; dm_tc_init(&s, 1000, 600); dm_tc_set_portal_tp(&s, 1);
    const dm_tc_target *p = &s.layout.target[DM_TC_PORTAL];
    float x = p->x, y = p->y, out = x - p->r - 40;

    dm_tc_begin_at(&s, 0, x, y, 1.0); dm_tc_end_at(&s, 0, 1.12);
    CHECK(s.portal_tp_seq == 1 && s.portal_town_seq == 0);

    dm_tc_begin_at(&s, 0, x, y, 2.0); dm_tc_tick(&s, 2.6); dm_tc_end_at(&s, 0, 2.7);
    CHECK(s.portal_tp_seq == 1 && s.portal_town_seq == 0); // 只长按不拖出，不施放

    dm_tc_begin_at(&s, 0, x, y, 3.0); dm_tc_tick(&s, 3.6); dm_tc_move_at(&s, 0, out, y, 3.7); dm_tc_end_at(&s, 0, 3.75);
    CHECK(s.portal_tp_seq == 1 && s.portal_town_seq == 1);

    dm_tc_begin_at(&s, 0, x, y, 4.0); dm_tc_move_at(&s, 0, out, y, 4.1); dm_tc_tick(&s, 4.7); dm_tc_end_at(&s, 0, 4.75);
    CHECK(s.portal_tp_seq == 1 && s.portal_town_seq == 1); // 未按够半秒就滑出，不施放

    dm_tc_begin_at(&s, 0, x, y, 5.0); dm_tc_move_at(&s, 0, out, y, 5.6); dm_tc_end_at(&s, 0, 5.65);
    CHECK(s.portal_tp_seq == 1 && s.portal_town_seq == 2); // 掉帧时没有 tick，仍按事件时间判定

    dm_tc_begin_at(&s, 0, x, y, 6.0); dm_tc_tick(&s, 6.6); dm_tc_move_at(&s, 0, out, y, 6.7); dm_tc_cancel_finger(&s, 0);
    CHECK(s.portal_tp_seq == 1 && s.portal_town_seq == 2); // 系统取消不施放

    printf("portal gesture: %d checks, %d failures\n", checks, bad);
    return bad != 0;
}
