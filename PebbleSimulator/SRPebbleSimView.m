//
//  SRPebbleSimView.m
//  PebbleSimulator
//
//  Created by Steven van Rossum on 14-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import "SRPebbleSimView.h"
#import "pebble_os.h"
#import "pebble_sim.h"
#import <CoreGraphics/CoreGraphics.h>
#include <dlfcn.h>

@implementation SRPebbleSimView

PebbleAppHandlers * appHandlers = NULL;
GContext * ctx = NULL;
CGContextRef graphicsContext = NULL;
CFMutableArrayRef windowStack;

SimulatorParams params;

void setAppHandlers(PebbleAppHandlers * handlers)
{
    appHandlers = handlers;
    // timer handler
    if (handlers->timer_handler)
    {
        
    }
    // tick info
    // tick handler
    if (handlers->tick_info.tick_handler)
    {
        
    }
    // input handlers
    // down
    if (handlers->input_handlers.buttons.down)
    {
        
    }
    // up
    if (handlers->input_handlers.buttons.up)
    {
        
    }
    // messaging info
    // in dropped
    if (handlers->messaging_info.default_callbacks.callbacks.in_dropped)
    {
        
    }
    // in received
    if (handlers->messaging_info.default_callbacks.callbacks.in_received)
    {
        
    }
    // out failed
    if (handlers->messaging_info.default_callbacks.callbacks.out_failed)
    {
        
    }
    // out sent
    if (handlers->messaging_info.default_callbacks.callbacks.out_sent)
    {
        
    }
    // deinit handler
    if (handlers->deinit_handler)
    {
        
    }
    // init handler
    if (handlers->init_handler)
    {
        
    }
}

void setGraphicsContext(GContext * context)
{
    while (!graphicsContext)
    {
    }
    context->coreGraphicsContext = graphicsContext;
    ctx = context;
}

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {

    }
    
    return self;
}

- (void)dealloc
{
    [super dealloc];
}

- (void)awakeFromNib
{
    [self performSelectorInBackground:@selector(awakePebbleApp) withObject:nil];
}

- (void)awakePebbleApp
{
    dlhandle = dlopen("/Users/stev/Documents/XCode/PebbleSimulator/DerivedData/PebbleSimulator/Build/Products/Debug/libhelloworld.dylib", RTLD_NOW | RTLD_FIRST);
    pbl_main = dlsym(dlhandle, "pbl_main");
    params.setGraphicsContext = &setGraphicsContext;
    params.setAppHandlers = &setAppHandlers;
    pbl_main(&params);
}

- (void)setUpGState
{
    if (!graphicsContext)
    {
        graphicsContext = [[NSGraphicsContext currentContext] graphicsPort];
        CGContextSetRGBFillColor(graphicsContext, 0.0f, 0.0f, 0.0f, 1.0f);
        CGContextFillRect(graphicsContext, [self bounds]);
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    if (graphicsContext && appHandlers && appHandlers->render_handler)
        appHandlers->render_handler((void*)&params, &((PebbleRenderEvent) { .window = NULL, .ctx = ctx }));
}

- (IBAction)upButtonClick:(id)sender
{
    //if ([windowStack count] > 0)
        ((ClickConfig**)window_stack_get_top_window()->click_config_context)[BUTTON_ID_UP]->click.handler(NULL, NULL);
}

- (IBAction)selectButtonClick:(id)sender
{
    //if ([windowStack count] > 0)
        ((ClickConfig**)window_stack_get_top_window()->click_config_context)[BUTTON_ID_SELECT]->click.handler(NULL, NULL);
}

- (IBAction)downButtonClick:(id)sender
{
    //if ([windowStack count] > 0)
        ((ClickConfig**)window_stack_get_top_window()->click_config_context)[BUTTON_ID_DOWN]->click.handler(NULL, NULL);
}

@end
