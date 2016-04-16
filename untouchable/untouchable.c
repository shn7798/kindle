/*  
    untouchable - filter touch event in kindle voyage
    Copyright (c) 2016 by shn7798, with MIT license:
    http://www.opensource.org/licenses/mit-license.php

*/

#include <stdio.h> // printf
#include <stdbool.h> // bool
#include <string.h> // memset
#include <stdlib.h> // strtol
#include <unistd.h> // usleep
#include <X11/XKBlib.h>
#include <signal.h>
#include <X11/Xutil.h>
#include <time.h>

bool process_event(Display *display, XEvent ev);
void cleanup();
void ignore_button(Display *dpy, XEvent ev);
void passthru_button(Display *dpy, XEvent ev);

const int click_tiger_timeout = 2; // seconds

int debug = true;
Display * display;
Window root;


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
    if(NULL != strstr(wm_name, "com.lab126.booklet.reader_M")){
    // focus in reader
        if(ev.xbutton.y > 700){
            ignore_button(display, ev);
            printf("y == %d, ignore\n", ev.xbutton.y);
            return true;
        }
    }
    
    return false;
}

int main(int argc, char * argv[]) {
	XEvent ev;
    int click_count = 0;
    long last_click_time = 0L;
    long cur_click_time = 0L;
    
    bool hook_enable = true;


	display = XOpenDisplay(NULL);
	if(display == NULL) {
		if(debug) printf("Could not open display\n");
		return 1;
	}
    root = DefaultRootWindow(display);
    
    // start hook
    XGrabButton(display, AnyButton, AnyModifier, root, true, ButtonPressMask | ButtonReleaseMask, GrabModeSync, GrabModeSync, None, None);
    XSelectInput(display, root, KeyPressMask | KeyReleaseMask);
    signal(SIGINT, cleanup);

    while(1){
        XNextEvent(display, &ev);
        printf("hook_enable == %s\n", hook_enable ? "ENABLED" : "DISABLED");
        switch(ev.type){
			case ButtonPress:
            case ButtonRelease:
            // 1448x1072
                if(ev.type == ButtonPress && ev.xbutton.x > 1000 && ev.xbutton.y > 1380){
                    cur_click_time = time(NULL);
                    if(cur_click_time - last_click_time >= click_tiger_timeout){
                        last_click_time = cur_click_time;
                        click_count = 1;
                    }
                    else{
                        ++click_count;
                    }
                    
                    if(click_count >= 3){
                        click_count = 0;
                        hook_enable = !hook_enable;
                        printf("tiger hook to %s\n", hook_enable ? "ENABLED" : "DISABLED");
                        ignore_button(display, ev);
                    }
                }
                
                if(hook_enable && process_event(display, ev)){
                   continue; 
                }
			break;
            case KeyPress:
            case KeyRelease:
                printf("key press/release\n");
            break;
		}
        passthru_button(display, ev);
    }
	
    cleanup();
	return 0;
}