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
    dlhandle = dlopen("/Users/stev/Documents/XCode/PebbleSimulator/DerivedData/PebbleSimulator/Build/Products/Debug/libhelloworld.dylib", RTLD_NOW | RTLD_FIRST);
    pbl_main = dlsym(dlhandle, "pbl_main");
    pbl_main(NULL);
}

- (void)setUpGState
{
    if (!gCtx.coreGraphicsContext)
    {
        gCtx.coreGraphicsContext = [[NSGraphicsContext currentContext] graphicsPort];
        clear = CGColorCreateGenericRGB(0.0f, 0.0f, 0.0f, 0.0f);
        black = CGColorCreateGenericRGB(0.0f, 0.0f, 0.0f, 1.0f);
        white = CGColorCreateGenericRGB(1.0f, 1.0f, 1.0f, 1.0f);
        CGContextSetFillColorWithColor(gCtx.coreGraphicsContext, black);
        CGContextFillRect(gCtx.coreGraphicsContext, [self bounds]);
    }
}

- (void)drawRect:(NSRect)dirtyRect
{
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
