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


- (void)setUpGState
{
    CGContextRef graphicsContext = [[NSGraphicsContext currentContext] graphicsPort];
    CGContextSetFillColorWithColor(graphicsContext, CGColorGetConstantColor(kCGColorBlack));
    CGContextFillRect(graphicsContext, [self bounds]);
    CGContextSetAllowsAntialiasing(graphicsContext, false);
}

- (void)drawRect:(NSRect)dirtyRect
{
    CGContextRef graphicsContext = [[NSGraphicsContext currentContext] graphicsPort];
    CGContextSetFillColorWithColor(graphicsContext, CGColorGetConstantColor(kCGColorBlack));
    CGContextFillRect(graphicsContext, [self bounds]);
    CGContextSetFillColorWithColor(graphicsContext, CGColorGetConstantColor(kCGColorWhite));
    CGContextFillEllipseInRect(graphicsContext, [self bounds]);
}

@end
