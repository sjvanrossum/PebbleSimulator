//
//  SRSimulatorWindowController.m
//  PebbleSimulator
//
//  Created by Steven van Rossum on 03-09-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import "SRSimulatorWindowController.h"

@implementation SRSimulatorWindowController
@synthesize backButton, upButton, selectButton, downButton;

static SRSimulatorWindowController * sharedInstance = nil;

- (void)windowWillClose:(NSNotification *)notification
{
    NSLog(@"%s", __func__);
    [application stopPebbleApplication];
    [application release];
    [[self displayView] releaseGState];
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
    [application runPebbleApplicationInBackgroundWithParameters:(SimulatorParams){
        .backButton = backButton,
        .upButton = upButton,
        .selectButton = selectButton,
        .downButton = downButton,
        .graphicsContext = (SimulatorGContext){
            .coreGraphicsContext = 0,
            .compositingMode = 0
        },
        
    }];
}

+ (SRSimulatorWindowController *)sharedController
{
    return sharedInstance ? sharedInstance : [[SRSimulatorWindowController alloc] initWithWindowNibName:@"SimulatorWindow"];
}

@end
