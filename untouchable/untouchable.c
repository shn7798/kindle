/*  
    untouchable - filter touch event in kindle voyage
    Copyright (c) 2016 by shn7798, with MIT license:
    http://www.opensource.org/licenses/mit-license.php

*/

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <time.h>
#include <sys/time.h>

#include "untouchable.h"

const unsigned long click_triger_timeout_us = 1500 * 1000; // 1500ms
struct rect triger_rect = {
    .top = 1350,
    .left = 950,
    .right = 1072,
    .bottom = 1448
};

struct rect hook_rect = {
    .top = 500,
    .left = 0,
    .right = 1072,
    .bottom = 1400
};


int debug = true;
Display * display;
Window root;


unsigned long get_current_time(void){
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}


void ignore_button(Display * dpy, XEvent ev)
{
    XAllowEvents(dpy, SyncBoth, ev.xbutton.time);
    XFlush(dpy); 
}

void passthru_button(Display * dpy, XEvent ev)
{
    XAllowEvents(dpy, ReplayPointer, ev.xbutton.time);
    XFlush(dpy); 
}

void cleanup(){
    printf("cleanup...\n");
    XUngrabButton(display, AnyButton, AnyModifier, root);
    XCloseDisplay(display);
}

bool process_event(Display *display, XEvent ev){
    char *wm_name;
    Window focus;
    int revert;
    
    if(debug) printf("Button Press - %d\n", ev.xbutton.button);
    if(debug) printf("Pos=[%d,%d]\n", ev.xbutton.x, ev.xbutton.y);
    
    XGetInputFocus(display, &focus, &revert);
    XFetchName(display, focus, &wm_name);
    printf("wm_name == [%s]\n", wm_name);
    if(NULL != wm_name && NULL != strstr(wm_name, "com.lab126.booklet.reader_M")){
        XFree(wm_name);
        // focus in reader
        if(check_pos_in_rect(ev.xbutton.x, ev.xbutton.y, hook_rect)){
            ignore_button(display, ev);
            printf("y == %d, ignore\n", ev.xbutton.y);
            return true;
        }
    }
    else{
        XFree(wm_name);
    }
    
    return false;
}

bool check_click(int click_count, unsigned long timeout_us, struct clickdata * cd, bool update_data){
    if(!update_data){
        void *ncd = malloc(sizeof(struct clickdata));
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
    bool result = cd->click_count >= click_count;
    if(!update_data){
        free(cd);
    }
    return result;
}

bool check_pos_in_rect(int x, int y, struct rect rt){
    return (x >= rt.left && x <= rt.right
        && y >= rt.top && y <= rt.bottom);
}

int main(int argc, char * argv[]) {
    XEvent ev;
    int click_count = 0;
    double last_click_time = 0.0;
    double cur_click_time = 0.0;
    
    struct clickdata triger_cd = {0};
    struct clickdata control_cd = {0};
    
    bool hook_enable = true;


    display = XOpenDisplay(NULL);
    if(display == NULL) {
        if(debug) printf("Could not open display\n");
        return 1;
    }
    root = DefaultRootWindow(display);
    
    // start hook
    XGrabButton(display, AnyButton, AnyModifier, root, true, ButtonPressMask | ButtonReleaseMask, GrabModeSync, GrabModeSync, None, None);
    signal(SIGINT, cleanup);
    
    while(1){
        XNextEvent(display, &ev);
        printf("hook_enable == %s\n", hook_enable ? "ENABLED" : "DISABLED");
        switch(ev.type){
            case ButtonPress:
            // 1448x1072
                if(check_pos_in_rect(ev.xbutton.x, ev.xbutton.y, triger_rect)){
                    if(check_click(3, click_triger_timeout_us, &triger_cd, true)){
                        hook_enable = !hook_enable;
                        printf("tiger hook to %s\n", hook_enable ? "ENABLED" : "DISABLED");
                        ignore_button(display, ev);
                    }
                }
                
                if(check_pos_in_rect(ev.xbutton.x, ev.xbutton.y, hook_rect)){
                    if(check_click(2, click_triger_timeout_us, &control_cd, false)){
                        passthru_button(display, ev);
                        continue;
                    }
                }
                
                if(hook_enable && process_event(display, ev)){
                    continue;
                }
            break;
            case ButtonRelease:
                if(check_pos_in_rect(ev.xbutton.x, ev.xbutton.y, hook_rect)){
                    if(check_click(2, click_triger_timeout_us, &control_cd, true)){
                        passthru_button(display, ev);
                        continue;
                    }
                }
                if(hook_enable && process_event(display, ev)){
                    continue;
                }
            break;
        }
        passthru_button(display, ev);
    }

    cleanup();
    return 0;
}
