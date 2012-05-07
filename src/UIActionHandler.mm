//
//  UIActionHandler.m
//  noiseShaper
//
//  Created by Damien Di Fede on 3/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "UIActionHandler.h"
#import "App.h"

@implementation UIActionHandler

-(UIActionHandler*)init:(App*)pApp
{
	self = [super init];
	m_pApp = pApp;
	
	return self;
}

// UIActionSheetDelegate implementation
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex
{

}


- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController
{

}

- (void)handlePinchGesture:(UIGestureRecognizer*)recognizer
{
	m_pApp->handlePinchGesture();
}

@end
