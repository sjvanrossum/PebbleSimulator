//
//  SRSimulatorWindowController.m
//  PebbleSimulator
//
//  Created by Steven van Rossum on 03-09-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import "SRSimulatorWindowController.h"

@implementation SRSimulatorWindowController
@synthesize backButton, upButton, selectButton, downButton, displayView;
void redisplayCallback();
static SRSimulatorWindowController * sharedInstance = nil;

- (void)windowWillClose:(NSNotification *)notification
{
    NSLog(@"%s", __func__);
    [application stopPebbleApplication];
    [application release];
}

- (void)runPebbleApplicationAtURL:(NSURL *)url
{
    if ([[self window] isVisible])
        [[self window] close];
    
    [[self displayView] allocateGState];
    
    [url retain];
    application = [[SRPebbleApplication alloc] initWithPebbleApplicationAtPath:[url path]];
    [url release];
    [self showWindow:self];
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
    CGContextRef ctx = CGBitmapContextCreate(NULL, 144, 168, 8, 144, colorSpace, 0);
    CGColorSpaceRelease(colorSpace);
    [[self displayView] setBitmapContext:ctx];
    [application runPebbleApplicationInBackgroundWithParameters:(SimulatorParams){
        .backButton = backButton,
        .upButton = upButton,
        .selectButton = selectButton,
        .downButton = downButton,
        .graphicsContext = (SimulatorGContext){
            .coreGraphicsContext = ctx,
            .compositingMode = 0
        },
        .redisplay = &redisplayCallback
    }];
}

+ (SRSimulatorWindowController *)sharedController
{
    return sharedInstance ? sharedInstance : (sharedInstance = [[SRSimulatorWindowController alloc] initWithWindowNibName:@"SimulatorWindow"]);
}

void redisplayCallback()
{
    [[sharedInstance displayView] display];
    //[[sharedInstance displayView] drawRect:[[sharedInstance displayView] bounds]];
}

@end
