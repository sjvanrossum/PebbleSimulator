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

- (void)windowDidLoad
{
    [super windowDidLoad];
}

- (BOOL)runPebbleApplicationAtURL:(NSURL *)url
{
    if (application)
        [application release];
    
    [url retain];
    application = [[SRPebbleApplication alloc] initWithPebbleApplicationAtPath:[url path]];
    [url release];
    
    [application runPebbleApplicationInBackground];
}

+ (SRSimulatorWindowController *)sharedController
{
    return sharedInstance ? sharedInstance : [[SRSimulatorWindowController alloc] initWithWindowNibName:@"SimulatorWindow"];
}

@end
