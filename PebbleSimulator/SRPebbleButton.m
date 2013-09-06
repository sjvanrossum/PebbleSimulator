//
//  SRPebbleButton.m
//  PebbleSimulator
//
//  Created by Steven van Rossum on 15-08-13.
//  Copyright (c) 2013 Steven van Rossum. All rights reserved.
//

#import "SRPebbleButton.h"

@implementation SRPebbleButton

- (void)mouseDown:(NSEvent *)theEvent
{
    [[self cell] setHighlighted:true];
    
    if (clickConfig)
    {
        ClickRecognizer recognizer;
        recognizer.buttonID = (ButtonId)[self tag];
        
        [theEvent retain];
        recognizer.clickCount = [theEvent clickCount];
        [theEvent release];
        
        if (clickConfig->raw.down_handler)
        {
            clickConfig->raw.down_handler((ClickRecognizerRef)&recognizer, clickConfig->context);
        }
        if (clickConfig->long_click.handler)
        {
            [self performSelector:@selector(longClick:) withObject:theEvent afterDelay:(1000.0/clickConfig->long_click.delay_ms)];
        }
    }
}

- (void)longClick:(NSEvent *)theEvent
{    
    if (clickConfig  && [[self cell] isHighlighted])
    {
        ClickRecognizer recognizer;
        recognizer.buttonID = (ButtonId)[self tag];
        recognizer.clickCount = 1;
        
        if (clickConfig->long_click.handler)
        {
            clickConfig->long_click.handler((ClickRecognizerRef)&recognizer, clickConfig->context);
        }
    }
}

- (void)mouseUp:(NSEvent *)theEvent
{
    [[self cell] setHighlighted:false];
    
    if (clickConfig)
    {
        ClickRecognizer recognizer;
        recognizer.buttonID = (ButtonId)[self tag];
        
        [theEvent retain];
        recognizer.clickCount = [theEvent clickCount];
        [theEvent release];
        
        if (clickConfig->raw.up_handler)
        {
            clickConfig->raw.up_handler((ClickRecognizerRef)&recognizer, clickConfig->context);
        }
        
        if (clickConfig->multi_click.handler)
        {
            if (clickConfig->multi_click.min <= [theEvent clickCount] && [theEvent clickCount] <= clickConfig->multi_click.max)
                clickConfig->multi_click.handler((ClickRecognizerRef)&recognizer, clickConfig->context);
        }
        
        if (clickConfig->long_click.release_handler)
        {
            clickConfig->long_click.release_handler((ClickRecognizerRef)&recognizer, clickConfig->context);
        }
        
        if (clickConfig->click.handler)
        {
            clickConfig->click.handler((ClickRecognizerRef)&recognizer, clickConfig->context);
        }
    }
}

@end
