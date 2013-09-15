//
//  SRAppDelegate.m
//  PebbleSimulator
//
//  Created by Steven van Rossum on 13-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import "SRAppDelegate.h"

@implementation SRAppDelegate

- (void)dealloc
{
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    windowController = [SRSimulatorWindowController sharedController];
}

- (void)openPebbleApplication:(id)sender
{
    NSOpenPanel * openPanel = [NSOpenPanel openPanel];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanCreateDirectories:NO];
    if ([openPanel runModal] == NSFileHandlingPanelOKButton)
    {
        [windowController runPebbleApplicationAtURL:[openPanel URL]];
    }
}

@end
