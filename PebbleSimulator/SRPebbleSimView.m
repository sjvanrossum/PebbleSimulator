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
CGContextRef getGraphicsContext(void);

PebbleAppHandlers * appHandlers;

CGContextRef graphicsContext;
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
    CGColorRelease(clear);
    CGColorRelease(black);
    CGColorRelease(white);
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
    params.getGraphicsContext = &getGraphicsContext;
    params.setAppHandlers = &setAppHandlers;
    pbl_main(&params);
}

- (void)setUpGState
{
    if (!graphicsContext)
    {
        graphicsContext = [[NSGraphicsContext currentContext] graphicsPort];
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
    //if (appHandlers && appHandlers->render_handler)
    //    appHandlers->render_handler(NULL, NULL);
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
    handlers->init_handler((void*)&params);
}

CGContextRef getGraphicsContext(void)
{
    while (!graphicsContext)
    {
    }
    return graphicsContext;
}
