#include <stdio.h>
#include <unistd.h>

#include "rk-tw.h"

int handle_timeout(void *)
{
    printf("handle timeout\n");
    return 0;
}

int main()
{
    rk_tw_t *tw = rk_tw_create(10, 1000, 10000);
    if (!tw) return -1;

    do {
        rk_tw_create_task(tw, 5000, handle_timeout, NULL);
        sleep(2);
        rk_tw_check(tw);
    } while (1);
    rk_tw_destroy(tw);
    return 0;
}