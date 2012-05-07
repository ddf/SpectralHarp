//
//  UIActionHandler.h
//  noiseShaper
//
//  Created by Damien Di Fede on 3/5/11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#ifndef UIACTIONHANDLER_H
#define UIACTIONHANDLER_H

#import <Foundation/Foundation.h>

class App;

@interface UIActionHandler : NSObject <UIActionSheetDelegate, UIPopoverControllerDelegate, UIWebViewDelegate>
{
	App * m_pApp;
}

- (UIActionHandler*)init:(App*)pApp;
- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex;
- (void)popoverControllerDidDismissPopover:(UIPopoverController *)popoverController;
- (void)handlePinchGesture:(UIGestureRecognizer*)recognizer;

@end

#endif // UIACTIONHANDLER_H