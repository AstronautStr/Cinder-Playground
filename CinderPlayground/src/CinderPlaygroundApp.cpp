#include "CinderPlaygroundApp.h"
#include "cinder/Timeline.h"
#include "cinder/app/cocoa/PlatformCocoa.h"
#include <fstream>

using namespace ci;
using namespace ci::app;
using namespace std;


FreqModulatorNode::FreqModulatorNode( const Format &format ) : audio::Node( format )
{
    mFreq = 0;
    mMod = 0;
    
    mFreqValue = 0;
    mModValue = 0;
}

void FreqModulatorNode::setFreq(float freq)
{
    lock_guard<mutex> lock( getContext()->getMutex() );
    mFreq = freq;
}

void FreqModulatorNode::setMod(float mod)
{
    lock_guard<mutex> lock( getContext()->getMutex() );
    mMod = mod;
}

void FreqModulatorNode::process( audio::Buffer *buffer )
{
    float* data = (float*)buffer->getData();
    size_t N = buffer->getSize();
    float sec = getContext()->getNumProcessedSeconds();
    
    for (size_t i = 0; i < N; ++i)
    {
        float sampleTime = sec + i / getSampleRate();
        
        
        float sgn = glm::sign(mFreq - mFreqValue);
        mFreqValue += 2000.0 / getSampleRate() * sgn;
        if ((sgn >= 0 && mFreqValue >= mFreq) || (sgn < 0 && mFreqValue <= mFreq))
            mFreqValue = mFreq;
        /*
        sgn = glm::sign(mMod - mModValue);
        mModValue += 1.0 / getSampleRate() * sgn;
        if ((sgn >= 0 && mModValue >= mMod) || (sgn < 0 && mModValue <= mMod))
            mModValue = mMod;
        /**/
        data[i] = mFreqValue;// * (1.0 + sin(2 * M_PI * sampleTime * 0.5 * mModValue) / 2.0);
    }
}

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
    mSineNode = ctx->makeNode(new audio::GenSineNode);
    mGainNode = ctx->makeNode(new audio::GainNode);
    
    mSineNode >> mGainNode >> ctx->getOutput();
    mGainNode->setValue(1.0);
    mSineNode->enable();
    
    modNode = ctx->makeNode(new FreqModulatorNode);
    mSineNode->getParamFreq()->setProcessor(modNode);
    modNode->enable();
    
    ctx->enable();

    cinder::BufferRef buffer = PlatformCocoa::get()->loadResource("plain.vert")->getBuffer();
    char* shaderText = (char*)buffer->getData();
    std::string vert(shaderText);
    
    buffer = PlatformCocoa::get()->loadResource("generator.frag")->getBuffer();
    shaderText = (char*)buffer->getData();
    std::string frag(shaderText);
    
    mGlsl = gl::GlslProg::create(gl::GlslProg::Format().vertex(vert).fragment(frag));
    mPlane = gl::Batch::create(geom::Rect(Rectf(-1.0, -1.0, 1.0, 1.0)), mGlsl);
}


void CinderPlaygroundApp::mouseMove( MouseEvent event )
{
    mouse = event.getPos();
    /*
    modNode->setFreq(40.0 + 200.0 * fabs(mouse.x / getWindowWidth() - 0.5));
    
    vec2 delta = mouse - getWindowCenter();
    delta /= getWindowSize();
    modNode->setMod(sqrt(delta.x * delta.x + delta.y * delta.y));*/
}


void CinderPlaygroundApp::mouseUp( MouseEvent event )
{
    
}

void CinderPlaygroundApp::mouseDown( MouseEvent event )
{
}

void CinderPlaygroundApp::update()
{
    modNode->setFreq(880.0 + 440.0 * ((1.0 + sin(timeline().getCurrentTime() * 2 * M_PI)) / 2));
}

void CinderPlaygroundApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );

    float time = timeline().getCurrentTime();
    mGlsl->uniform("time", time);
    mGlsl->uniform("screenWidth", (float)getWindowWidth());
    mGlsl->uniform("screenHeight", (float)getWindowHeight());
    mGlsl->uniform("mouseX", mouse.x);
    mGlsl->uniform("mouseY", mouse.y);
    mPlane->draw();
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
