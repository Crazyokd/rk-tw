#include "rk-tw.h"

#include <stdlib.h>

rk_tw_t *rk_tw_create(unsigned int slot_cnt, unsigned int granularity,
                      unsigned int capacity)
{
    if (!slot_cnt || !granularity || !capacity) return NULL;
    rk_tw_t *tw = malloc(sizeof(rk_tw_t));
    if (!tw) return NULL;

    tw->free = malloc(sizeof(rk_task_t) * capacity);
    if (!tw->free) {
        free(tw);
        return NULL;
    }

    tw->slots = calloc(slot_cnt, sizeof(rk_task_t *));
    if (!tw->slots) {
        free(tw->free);
        free(tw);
        return NULL;
    }

    tw->free_tasks = tw->free;
    for (unsigned int i = 0; i < capacity - 1; i++) {
        tw->free_tasks[i].next = tw->free_tasks + i + 1;
    }
    tw->free_tasks[capacity - 1].next = NULL;

    tw->slot_cnt = slot_cnt;
    tw->granularity = granularity;
    tw->rot_cnt = 0;
    tw->idx = 0;
    tw->capacity = capacity;

    return tw;
}

void rk_tw_destroy(rk_tw_t *tw)
{
    if (!tw) return;
    free(tw->slots);
    free(tw->free);
    free(tw);
}

int rk_tw_create_task(rk_tw_t *tw, unsigned int interval, rk_task_cb cb,
                      void *data)
{
    if (!tw || !interval || !cb) return -1;
    rk_task_t *task = tw->free_tasks;
    if (!task) return -1;

    tw->free_tasks = task->next;
    /* now fill task */
    unsigned int scale = tw->idx + interval / tw->granularity;
    task->idx = scale % tw->slot_cnt;
    task->rot_threshold = tw->rot_cnt + scale / tw->slot_cnt;
    task->cb = cb;
    task->data = data;

    task->next = tw->slots[task->idx];
    if (tw->slots[task->idx]) tw->slots[task->idx]->prev = task;
    tw->slots[task->idx] = task;

    return 0;
}

void rk_tw_destroy_task(rk_tw_t *tw, rk_task_t *task)
{
    if (!tw || !task) return;
    if (task->prev)
        task->prev->next = task->next;
    else
        tw->slots[task->idx] = task->next;

    if (task->next) task->next->prev = task->prev;

    task->next = tw->free_tasks;
    tw->free_tasks = task;
}

void rk_tw_update_task(rk_tw_t *tw, rk_task_t *task, int delta)
{
    if (!tw || !task || !delta) return;

    /* todo: distinguish between delta positive and negative */
    int scale = task->idx + delta / tw->granularity;
    int next_idx = scale % tw->slot_cnt;
    if (next_idx < 0) next_idx += tw->slot_cnt;
    int rot_delta = scale / tw->slot_cnt;
    if (scale < 0) rot_delta += -1;
    if (rot_delta < 0 && (unsigned int)(-rot_delta) > task->rot_threshold) {
        /* avoid overflow */
        task->rot_threshold = 0;
    } else {
        task->rot_threshold += rot_delta;
    }

    /* fetch task from current slot */
    if (task->prev)
        task->prev->next = task->next;
    else
        tw->slots[task->idx] = task->next;
    if (task->next) task->next->prev = task->prev;
    /* insert task into slots[next_idx] */
    task->idx = next_idx;
    if (tw->slots[next_idx]) tw->slots[next_idx]->prev = task;
    task->next = tw->slots[next_idx];
    tw->slots[next_idx] = task;
}

static inline void rk_tw_do_tasks(rk_tw_t *tw, rk_task_t *task)
{
    if (!tw) return;
    while (task) {
        rk_task_t *next = task->next;
        if (task->rot_threshold <= tw->rot_cnt) {
            int res = task->cb(task->data);
            if (res <= 0) {
                /* free task */
                rk_tw_destroy_task(tw, task);
            } else {
                /* update task */
                rk_tw_update_task(tw, task, res);
            }
        }
        task = next;
    }
}

void rk_tw_check(rk_tw_t *tw)
{
    if (!tw) return;

    rk_tw_do_tasks(tw, tw->slots[tw->idx]);
    tw->idx++;
    if (tw->idx >= tw->slot_cnt) {
        /* unlikely */
        tw->idx %= tw->slot_cnt;
        tw->rot_cnt++;
    }
}
