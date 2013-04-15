//
//  pebble_sim.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 14-04-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#pragma once

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include "pebble_os.h"

//NSCalendar * gregorian;
//NSTimeZone * tzone;
//NSPointerArray * windowStack;

struct GContext {
    CGContextRef coreGraphicsContext;
};

void initOS(void);
void deinitOS(void);

typedef struct SimulatorParams
{
    CGContextRef (*getGraphicsContext)(void);
    void (*setAppHandlers)(PebbleAppHandlers * handlers);
    void (*setWindowStack)(CFMutableArrayRef windowStack);
} SimulatorParams;