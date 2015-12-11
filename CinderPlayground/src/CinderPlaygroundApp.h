//
//  CinderPlaygroundApp.h
//  CinderPlayground
//
//  Created by Ilya Solovyov on 11/12/15.
//
//

#ifndef CinderPlaygroundApp_h
#define CinderPlaygroundApp_h

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/audio/audio.h"


class FreqModulatorNode : public ci::audio::Node
{
public:
    FreqModulatorNode( const Format &format = Format() );
    
    void setFreq(float freq);
    void setMod(float mod);
    
protected:
    float mFreq;
    float mFreqValue;
    float mMod;
    float mModValue;
    
    void process( ci::audio::Buffer *buffer ) override;
};

class CinderPlaygroundApp : public ci::app::App
{
public:
    void setup() override;
    void mouseMove( cinder::app::MouseEvent event ) override;
    void mouseDown( cinder::app::MouseEvent event ) override;
    void mouseUp( cinder::app::MouseEvent event ) override;
    void update() override;
    void draw() override;
    
protected:
    ci::audio::GenSineNodeRef mSineNode;
    ci::audio::GainNodeRef mGainNode;
    
    ci::gl::GlslProgRef mGlsl;
    ci::gl::BatchRef mPlane;
    std::shared_ptr<FreqModulatorNode> modNode;
    
    glm::vec2 mouse;
    
    double controlValue;
};

#endif /* CinderPlaygroundApp_h */
