
#include "ofMain.h"
#include "App.h"


int main(int argc, char *argv[]) 
{
    NSAutoreleasePool* pool = [NSAutoreleasePool new];
    
	ofSetupOpenGL(1024,768, OF_FULLSCREEN);			// <-------- setup the GL context

	ofRunApp(new App);
    
    [pool release];
}
