//
//  pbl_main.c
//  helloworld
//
//  Created by Steven van Rossum on 15-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#include "pebble_os.h"

Window window;

void rootupdate(Layer * layer, GContext * ctx)
{
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, GRect(0,0,144,168), 0, 0);
}

void handle_init(AppContextRef app_ctx)
{
    window_init(&window, "My First Window");
    layer_set_update_proc(window_get_root_layer(&window), &rootupdate);
    window_stack_push(&window, false);
}

void handle_tick(AppContextRef app_ctx, PebbleTickEvent * event)
{
    PblTm tm;
    get_time(&tm);
    char buf[256];
    char * fmt = "%X %x";
    string_format_time(buf, 256, fmt, &tm);
    printf("%s\n", buf);
}

void handle_timer(AppContextRef app_ctx, AppTimerHandle handle, uint32_t cookie)
{
    printf("Cookie: %i\n", cookie);
}

void pbl_main(void * params)
{
    PebbleAppHandlers handlers = (PebbleAppHandlers)
    {
        /*.tick_info = (PebbleAppTickInfo)
        {
            .tick_handler = &handle_tick,
            .tick_units = SECOND_UNIT
        },*/
        .init_handler = &handle_init,
        .timer_handler = &handle_timer
    };
    app_event_loop(params, &handlers);
}