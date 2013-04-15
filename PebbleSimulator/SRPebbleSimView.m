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

void setAppHandlers(PebbleAppHandlers * handlers);
void setGraphicsContext(GContext * context);

PebbleAppHandlers * appHandlers = NULL;
GContext * ctx = NULL;
CGContextRef graphicsContext = NULL;
CFMutableArrayRef windowStack;

SimulatorParams params;

@implementation SRPebbleSimView

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

void setAppHandlers(PebbleAppHandlers * handlers)
{
    appHandlers = handlers;
    if (handlers->init_handler)
        handlers->init_handler((void*)&params);
}

void setGraphicsContext(GContext * context)
{
    while (!graphicsContext)
    {
    }
    context->coreGraphicsContext = graphicsContext;
    ctx = context;
}
