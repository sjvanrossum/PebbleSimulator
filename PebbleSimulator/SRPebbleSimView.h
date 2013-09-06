//
//  SRPebbleSimView.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 14-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <CoreFoundation/CoreFoundation.h>
#import "pebble_sim.h"

@interface SRPebbleSimView : NSView

void setAppHandlers(PebbleAppHandlers * handlers);
void setGraphicsContext(SimulatorGContext * context);

@end
