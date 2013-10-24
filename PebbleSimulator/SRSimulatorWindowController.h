//
//  SRSimulatorWindowController.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 03-09-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreGraphics/CoreGraphics.h>
#import "SRPebbleApplication.h"
#import "SRPebbleSimView.h"
#import "SRPebbleButton.h"

@interface SRSimulatorWindowController : NSWindowController <NSWindowDelegate>
{
    // Variables
    @private SRPebbleApplication * application;
    @private SRPebbleButton * upButton;
    @private SRPebbleButton * downButton;
    @private SRPebbleButton * selectButton;
    @private SRPebbleButton * backButton;
    @private SRPebbleSimView * displayView;
}

// Properties
@property (assign) IBOutlet SRPebbleButton * upButton;
@property (assign) IBOutlet SRPebbleButton * downButton;
@property (assign) IBOutlet SRPebbleButton * selectButton;
@property (assign) IBOutlet SRPebbleButton * backButton;
@property (assign) IBOutlet SRPebbleSimView * displayView;

// Class methods
+ (SRSimulatorWindowController *)sharedController;

// Instance methods
- (void)runPebbleApplicationAtURL:(NSURL *)url;

@end
