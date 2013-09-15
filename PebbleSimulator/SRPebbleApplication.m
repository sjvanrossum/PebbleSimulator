//
//  SRPebbleApplication.m
//  PebbleSimulator
//
//  Created by Steven van Rossum on 03-09-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import "SRPebbleApplication.h"
#import "pebble_sim.h"

@interface SRPebbleApplication ()

- (void)runPebbleApplication;

@end

@implementation SRPebbleApplication

- (id)initWithPebbleApplicationAtPath:(NSString *)path
{
    if (self = [super init])
    {
        [self loadPebbleApplicationAtPath:path];
    }
    
    return self;
}

- (bool)loadPebbleApplicationAtPath:(NSString *)path
{
    [path retain];
    const char * cPath = [path UTF8String];
    
    if (path)
    {
        dlhandle = dlopen(cPath, RTLD_NOW | RTLD_FIRST);
        [path release];
        
        if (dlhandle)
        {
            pbl_main = dlsym(dlhandle, "pbl_main");
        }
    }
    
    return dlhandle && pbl_main;
}

- (void)runPebbleApplication
{
    if (pbl_main)
    {
        applicationContext.runLoop = CFRunLoopGetCurrent();
        pbl_main(&applicationContext);
    }
}

- (void)runPebbleApplicationInBackgroundWithParameters:(SimulatorParams)parameters
{
    applicationContext = parameters;
    [self performSelectorInBackground:@selector(runPebbleApplication) withObject:nil];
    [self performSelectorInBackground:@selector(runPebbleApplication) withObject:nil];

}

- (void)stopPebbleApplication
{
    CFRunLoopStop(applicationContext.runLoop);
}

- (void)dealloc
{
    dlclose(dlhandle);
    [super dealloc];
}

@end
