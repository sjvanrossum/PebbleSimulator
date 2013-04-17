//
//  pbl_main.c
//  helloworld
//
//  Created by Steven van Rossum on 15-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#include "pebble_os.h"

void handle_render(AppContextRef app_ctx, PebbleRenderEvent *event)
{
    graphics_context_set_fill_color(event->ctx, GColorWhite);
    graphics_fill_rect(event->ctx, GRect(20, 20, 104, 128), 0, 0);
    graphics_context_set_fill_color(event->ctx, GColorBlack);
    graphics_fill_rect(event->ctx, GRect(40, 40, 64, 88), 0, 0);
}

void pbl_main(void * params)
{
    PebbleAppHandlers handlers = (PebbleAppHandlers)
    {
        .render_handler = &handle_render
    };
    app_event_loop(params, &handlers);
}