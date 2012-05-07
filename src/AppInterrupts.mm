/*
 *  AppInterrupts.cpp
 *  noiseShaper
 *
 *  Created by Damien Di Fede on 7/1/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "App.h"
#include <AudioToolbox/AudioServices.h>
#include "AudioOutput.h"
#include "MultiChannelBuffer.h"
#include "ofxiPhoneExtras.h"

//--------------------------------------------------------------
void AudioInterruptionListener ( void *inClientData, UInt32 inInterruptionState )
{
	App	* pApp = static_cast<App*>( inClientData );
	
	switch ( inInterruptionState )
	{
		case  kAudioSessionBeginInterruption:
		{
			printf("Audio was interrupted!\n");
			pApp->pauseAudio();
		}
			break;
			
		case kAudioSessionEndInterruption:
		{
			AudioSessionSetActive(true);
			
			CFStringRef currentRoute;
			UInt32 propSize = sizeof(currentRoute);
			AudioSessionGetProperty(kAudioSessionProperty_AudioRoute, &propSize, &currentRoute);
			//if ( CFStringCompare(currentRoute, CFSTR("Speaker"), 0) != kCFCompareEqualTo )
			{
				printf("Audio is now resuming!\n");
				pApp->resumeAudio();
			}
		}
			break;
			
		default:
			break;
	}
}

//---------------------------------------------------------------
void AudioRouteChangeListener( void                      *inClientData,
							  AudioSessionPropertyID    inID,
							  UInt32                    inPropertySize,
							  const void                *inPropertyValue
							  )
{
	if ( inID == kAudioSessionProperty_AudioRouteChange )
	{
		App * pApp = static_cast<App*>( inClientData );
		
		CFDictionaryRef routeChangeDictionary = static_cast<CFDictionaryRef>(inPropertyValue);
        CFNumberRef routeChangeReasonRef = static_cast<CFNumberRef>( CFDictionaryGetValue (routeChangeDictionary, CFSTR(kAudioSession_AudioRouteChangeKey_Reason) ) );
		
        SInt32 routeChangeReason;
        CFNumberGetValue ( routeChangeReasonRef, kCFNumberSInt32Type, &routeChangeReason );
		
		CFStringRef previousRouteRef = static_cast<CFStringRef>( CFDictionaryGetValue(routeChangeDictionary, CFSTR(kAudioSession_AudioRouteChangeKey_OldRoute)) ); 
		printf("Previous audio route was %s\n", CFStringGetCStringPtr(previousRouteRef, CFStringGetFastestEncoding(previousRouteRef)));
		
        if ( routeChangeReason == kAudioSessionRouteChangeReason_OldDeviceUnavailable || routeChangeReason == kAudioSessionRouteChangeReason_NewDeviceAvailable )
		{
			if ( CFStringCompare(previousRouteRef, CFSTR("Headphone"), 0) == kCFCompareEqualTo )
			{
				printf("Pausing audio ... \n");
				pApp->pauseAudio();
			}
		}
	}
}

//--------------------------------------------------------------
void App::pauseAudio() 
{
	m_bWasPlaying = mOutput->getVolume() > 0.f;
	mOutput->setVolume( 0.f );
}

//--------------------------------------------------------------
void App::resumeAudio() 
{
	if ( m_bWasPlaying )
	{
		mOutput->setVolume( 1.f );
	}
}

//--------------------------------------------------------------
void App::lostFocus() 
{
}

//--------------------------------------------------------------
void App::gotFocus() 
{

}

//--------------------------------------------------------------
void App::gotMemoryWarning() 
{
	printf("OH SHIT SON GOT A MEMORY WARNING!");
}

void App::deviceOrientationChanged(int newOrientation)
{
	printf( "Orientation changed to %d.\n", newOrientation );
	
	if ( newOrientation == UIDeviceOrientationLandscapeLeft )
	{
        ofxiPhoneGetUIWindow().transform = CGAffineTransformRotate(CGAffineTransformIdentity, M_PI);
	}
	else if ( newOrientation == UIDeviceOrientationLandscapeRight )
	{
        ofxiPhoneGetUIWindow().transform = CGAffineTransformIdentity;
	}
}
