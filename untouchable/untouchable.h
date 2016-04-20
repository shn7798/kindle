/*  
    untouchable - filter touch event in kindle voyage
    Copyright (c) 2016 by shn7798, with MIT license:
    http://www.opensource.org/licenses/mit-license.php

*/


#include <X11/Xutil.h>
#include <X11/XKBlib.h>

struct clickdata{
    unsigned long start_time;
    unsigned long last_click_time;
    unsigned long cur_click_time;
    int click_count;
};

struct rect{
    int left;
    int right;
    int top;
    int bottom;
};

bool process_event(Display *display, XEvent ev);
void cleanup();
void ignore_button(Display *dpy, XEvent ev);
void passthru_button(Display *dpy, XEvent ev);

unsigned long get_current_time(void);
bool check_click(int click_count, unsigned long timeout_us, struct clickdata * cd, bool update_data);
bool check_pos_in_rect(int x, int y, struct rect rt);


