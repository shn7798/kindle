#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <string.h>
#include <sys/time.h>
struct clickdata{
    unsigned long start_time;
    unsigned long last_click_time;
    unsigned long cur_click_time;
    int click_count;
};


unsigned long get_current_time(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}



bool check_click(int click_count, unsigned long timeout_us, struct clickdata * cd, bool update_data){
    bool result = false;
    void *ncd = NULL;
    if(!update_data){
        ncd = malloc(sizeof(struct clickdata));
        memcpy(ncd, cd, sizeof(struct clickdata));
        cd = (struct clickdata*)ncd;
    }
    cd->cur_click_time = get_current_time();
    printf("diff=[%ld], timeout_ms=[%ld]\n", cd->cur_click_time - cd->start_time, timeout_us);
    if(cd->cur_click_time - cd->start_time > timeout_us){
        cd->start_time = cd->cur_click_time;
        cd->last_click_time = cd->cur_click_time;
        cd->click_count = 1;
    }
    else{
        cd->last_click_time = cd->cur_click_time;
        ++(cd->click_count);
    }
    printf("click_count == [%d]\n", cd->click_count);
    result = cd->click_count >= click_count;
    if(!update_data){
        free(ncd);
    }
    return result;
}


int main(void){
    struct clickdata cd = {0};

    check_click(3, 1500, &cd, false);
    check_click(3, 1500, &cd, false);
    check_click(3, 1500, &cd, false);
    check_click(3, 1500, &cd, false);
    check_click(3, 1500, &cd, false);
    check_click(3, 1500, &cd, false);
}
