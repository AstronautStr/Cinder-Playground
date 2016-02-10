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

#include "cinder/Utilities.h"


class CinderPlaygroundApp : public ci::app::App
{
public:    
    void setup() override;
    void mouseMove( cinder::app::MouseEvent event ) override;
    void mouseDown( cinder::app::MouseEvent event ) override;
    void mouseUp( cinder::app::MouseEvent event ) override;
    void update() override;
    void draw() override;
};

#endif /* CinderPlaygroundApp_h */
