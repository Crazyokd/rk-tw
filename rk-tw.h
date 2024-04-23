#ifndef _KR_TW_H_
#define _KR_TW_H_

/**
 * the callback for timeout
 * 
 * @param data
 * @return the result of callback
 * 
 *  < 0, error, and the task will be deleted.
 *  = 0, successful handling and deletion.
 *  > 0, successful handling, and the task will be added back to the tw.
 *       the result will be the next interval.
*/
typedef int (*rk_task_cb)(void *);

typedef struct rk_task_s {
    struct rk_task_s *prev;
    struct rk_task_s *next;
    rk_task_cb cb;
    void *data;
    unsigned int rot_threshold; /* rotate threshold */
    unsigned int idx; /* the slots where the task is located */
} rk_task_t;

typedef struct rk_tw_s {
    rk_task_t **slots;
    unsigned int slot_cnt; /* slot count */
    unsigned int granularity;
    unsigned int rot_cnt; /* rotate count */
    unsigned int idx; /* the current index of slots */

    rk_task_t *free_tasks;
    unsigned int capacity;
    rk_task_t *free; /* for free all tasks */
} rk_tw_t;

rk_tw_t *rk_tw_create(unsigned int slot_cnt, unsigned int granularity,
                      unsigned int capacity);
void rk_tw_destroy(rk_tw_t *tw);
void rk_tw_check(rk_tw_t *tw);
int rk_tw_create_task(rk_tw_t *tw, unsigned int interval, rk_task_cb cb,
                      void *data);
void rk_tw_destroy_task(rk_tw_t *tw, rk_task_t *task);
void rk_tw_update_task(rk_tw_t *tw, rk_task_t *task, int delta);

#endif
