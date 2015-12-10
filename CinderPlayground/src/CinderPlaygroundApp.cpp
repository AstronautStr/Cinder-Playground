#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"
#include "cinder/Timeline.h"
#include "cinder/app/cocoa/PlatformCocoa.h"
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;

class CinderPlaygroundApp : public App
{
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
	void update() override;
	void draw() override;
    
protected:
    audio::GenPulseNodeRef mPulseNode;
    audio::GainNodeRef mGainNode;
    
    gl::GlslProgRef mGlsl;
    gl::BatchRef mPlane;
    
    double controlValue;
};

std::string readAllText(std::string const& path)
{
    std::ifstream ifstr(path);
    if (!ifstr)
        return std::string();
    std::stringstream text;
    ifstr >> text.rdbuf();
    return text.str();
}

void CinderPlaygroundApp::setup()
{
    auto ctx = audio::Context::master();
    mPulseNode = ctx->makeNode(new audio::GenPulseNode);
    mGainNode = ctx->makeNode(new audio::GainNode);
    
    mPulseNode >> mGainNode >> ctx->getOutput();
    mPulseNode->setFreq(50.0);
    mGainNode->setValue(1.0);
    mPulseNode->enable();
    //ctx->enable();
  
    std::string vert((char*)app::PlatformCocoa::get()->loadResource("plain.vert")->getBuffer()->getData());
    std::string frag((char*)app::PlatformCocoa::get()->loadResource("generator.frag")->getBuffer()->getData());
    
    mGlsl = gl::GlslProg::create(gl::GlslProg::Format().vertex(vert).fragment(frag));
    mPlane = gl::Batch::create(geom::Rect(Rectf(-1.0, -1.0, 1.0, 1.0)), mGlsl);
}

void CinderPlaygroundApp::mouseDown( MouseEvent event )
{
}

void CinderPlaygroundApp::update()
{
    float time = timeline().getCurrentTime();
    controlValue = sin(time);
    
    mGainNode->setValue(controlValue*controlValue);
}

void CinderPlaygroundApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    float time = timeline().getCurrentTime();
    mGlsl->uniform("time", time);
    mGlsl->uniform("screenWidth", (float)getWindowWidth());
    mGlsl->uniform("screenHeight", (float)getWindowHeight());
    mPlane->draw();
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
