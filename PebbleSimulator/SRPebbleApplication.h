//
//  SRPebbleApplication.h
//  PebbleSimulator
//
//  Created by Steven van Rossum on 03-09-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <dlfcn.h>
#import "pebble_sim.h"

@interface SRPebbleApplication : NSObject
{
    void * dlhandle;
    void (*pbl_main)(void * params);
    SimulatorParams applicationContext;
}

- (id)initWithPebbleApplicationAtPath:(NSString *)path;
- (bool)loadPebbleApplicationAtPath:(NSString *)path;
- (void)runPebbleApplicationInBackgroundWithParameters:(SimulatorParams)parameters;
- (void)stopPebbleApplication;

@end
