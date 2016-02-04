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
    ~CinderPlaygroundApp();
    
    void setup() override;
    void mouseMove( cinder::app::MouseEvent event ) override;
    void mouseDown( cinder::app::MouseEvent event ) override;
    void mouseUp( cinder::app::MouseEvent event ) override;
    void update() override;
    void draw() override;
    
protected:
    GLuint _feedbackVbo;
    GLuint _feedbackVao;
    GLuint _feedbackTfbo;
    GLuint _feedbackShader;
    GLuint _feedbackProgram;
    
    GLfloat* _dataInputBuffer;
    GLfloat* _dataFeedbackBuffer;    
    int _dataLength;
    int _dataBytesSize;
    int _gridWidth;
    int _gridHeight;
    
    void _prepareFeedback();
    void _prepareFeedbackProgram();
    void _prepareFeedbackBuffers();
    void _prepareFeedbackVertexArray();
    
    void _prepareNextUpdate();
    void _updateFeedback();
};

#endif /* CinderPlaygroundApp_h */
