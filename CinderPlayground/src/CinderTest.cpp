#include "CinderTest.h"
#include "cinder/Timeline.h"
#include "cinder/app/cocoa/PlatformCocoa.h"
#include <fstream>

#include "cinder/audio/audio.h"

//#define _WTF std::cerr << gl::getErrorString(gl::getError()) << endl;

using namespace ci;
using namespace ci::app;
using namespace std;

void CinderPlaygroundApp::setup()
{
    auto ctx = audio::Context::master();
    
    audio::GenSineNodeRef firstSine = ctx->makeNode(new audio::GenSineNode);
    audio::GainNodeRef firstGain = ctx->makeNode(new audio::GainNode);
    firstSine->setFreq(40.0);
    firstGain->setValue(1.0);
    firstSine->enable();
    
    audio::GenSineNodeRef secondSine = ctx->makeNode(new audio::GenSineNode);
    audio::GainNodeRef secondGain = ctx->makeNode(new audio::GainNode);
    secondSine->setFreq(400.0);
    secondGain->setValue(1.0);
    secondSine->enable();
    
    audio::GenSineNodeRef thirdSine = ctx->makeNode(new audio::GenSineNode);
    audio::GainNodeRef thirdGain = ctx->makeNode(new audio::GainNode);
    thirdSine->setFreq(4000.0);
    thirdGain->setValue(1.0);
    thirdSine->enable();
    
    firstSine >> firstGain;
    secondSine >> secondGain;
    thirdSine >> thirdGain;
    
    audio::AddNodeRef add1 = ctx->makeNode(new audio::AddNode);
    audio::AddNodeRef add2 = ctx->makeNode(new audio::AddNode);
    add1->enable();
    add2->enable();
    
    firstGain >> add1;
    secondGain >> add1;
    
    add1 >> add2;
    thirdGain >> add2;
    
    add2 >> ctx->getOutput();
    
    ctx->enable();
}


void CinderPlaygroundApp::mouseMove( MouseEvent event )
{
}


void CinderPlaygroundApp::mouseUp( MouseEvent event )
{
    
}

void CinderPlaygroundApp::mouseDown( MouseEvent event )
{
}

void CinderPlaygroundApp::update()
{
}

void CinderPlaygroundApp::draw()
{
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
