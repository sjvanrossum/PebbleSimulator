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

typedef struct SimulatorParams
{
    void (*setGraphicsContext)(GContext * context);
    void (*setAppHandlers)(PebbleAppHandlers * handlers);
    void (*setWindowStack)(CFMutableArrayRef windowStack);
    PebbleAppHandlers * handlers;
} SimulatorParams;