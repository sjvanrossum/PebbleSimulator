//
//  SRPebbleButton.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 15-08-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "pebble_sim.h"

@interface SRPebbleButton : NSButton
{
    ClickConfig * clickConfig;
}

@end
