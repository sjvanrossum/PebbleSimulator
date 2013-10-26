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
@synthesize bitmapContext;

- (void)drawRect:(NSRect)dirtyRect
{
    if (self.bitmapContext)
    {
        CGImageRef display = CGBitmapContextCreateImage(self.bitmapContext);
        CGContextDrawImage([[NSGraphicsContext currentContext] graphicsPort], CGRectMake(0, 0, 144, 168), display);
        CGImageRelease(display);
    }
}

@end
