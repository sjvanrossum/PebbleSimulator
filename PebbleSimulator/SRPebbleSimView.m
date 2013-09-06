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
    
    // Handled in pebble_os.
    
    /*
    // timer handler
    if (handlers->timer_handler)
    {
        
    }
    // tick info
    // tick handler
    if (handlers->tick_info.tick_handler)
    {
        
    }
    */
    
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
    
    // Handled in pebble_os.
    
    /*
    // deinit handler
    if (handlers->deinit_handler)
    {
        
    }
    // init handler
    if (handlers->init_handler)
    {
        
    }
    */
}

void setGraphicsContext(SimulatorGContext * context)
{
    while (!graphicsContext)
    {
    }
    context->coreGraphicsContext = graphicsContext;
    ctx = context;
}

void setClickConfigs(ClickConfig** clickConfigs, void * obj)
{
    //self.backButton.clickConfig = clickConfigs[0];
    //self.upButton.clickConfig = clickConfigs[1];
    //self.selectButton.clickConfig = clickConfigs[2];
    //self.downButton.clickConfig = clickConfigs[3];
}

- (void)awakeFromNib
{
    [self allocateGState];
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

- (void)setUpGState
{
    if (!graphicsContext)
    {
        graphicsContext = [[NSGraphicsContext currentContext] graphicsPort];
        CGContextSetFillColorWithColor(graphicsContext, CGColorGetConstantColor(kCGColorBlack));
        CGContextFillRect(graphicsContext, [self bounds]);
        CGContextSetAllowsAntialiasing(graphicsContext, false);
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    CGContextSetFillColorWithColor(graphicsContext, CGColorGetConstantColor(kCGColorBlack));
    CGContextFillRect(graphicsContext, [self bounds]);
    CGContextSetFillColorWithColor(graphicsContext, CGColorGetConstantColor(kCGColorWhite));
    CGContextFillEllipseInRect(graphicsContext, [self bounds]);
    if (graphicsContext && appHandlers && appHandlers->render_handler)
        appHandlers->render_handler((void*)&params, &((PebbleRenderEvent) { .window = NULL, .ctx = ctx }));
}

@end
