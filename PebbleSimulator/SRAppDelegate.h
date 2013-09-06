//
//  SRAppDelegate.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 13-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SRSimulatorWindowController.h"

@interface SRAppDelegate : NSObject <NSApplicationDelegate>
{
    SRSimulatorWindowController * windowController;
}

-(IBAction)openPebbleApplication:(id)sender;

@end
