 //
//  CinderPlaygroundApp.h
//  CinderPlayground
//
//  Created by Ilya Solovyov on 11/12/15.
//
//

#ifndef GPULife_h
#define GPULife_h

#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

#include "cinder/Utilities.h"


class CinderPlaygroundApp : public ci::app::App
{
public:
    ~CinderPlaygroundApp();
    
    void modifyCell(cinder::vec2 point, bool state);
    
    void setup() override;
    void mouseMove( cinder::app::MouseEvent event ) override;
    void mouseDrag( cinder::app::MouseEvent event ) override;
    void mouseDown( cinder::app::MouseEvent event ) override;
    void mouseUp( cinder::app::MouseEvent event ) override;
    void keyDown( cinder::app::KeyEvent event ) override;
    void update() override;
    void draw() override;
    
protected:
    
    typedef GLfloat CellAttrib;
    static GLenum getFeedbackDataFormat()
    {
        return GL_R32F;
    }
    CellAttrib getRandomCell()
    {
        return _cycleStep * (float)(rand() % _cycleN);
    }

    GLuint _cellVAO;
    GLuint _cellVBOCells;
    GLuint _cellVBOPos;
    
    GLuint _TBOTex;
    
    GLuint _feedbackBuffer;
    
    GLuint _CAShader;
    GLuint _CAProgram;
    
    GLuint _drawingVAO;
    GLuint _drawingVBO;
    
    GLint _drawingScreenSizeLoc;
    GLuint _drawingFragmentShader;
    GLuint _drawingVertexShader;
    GLuint _drawingProgram;
    
    CellAttrib* _dataResultBuffer;
    
    cinder::Font mFont;
    cinder::vec2 _mousePos;
    
    int _dataLength;
    int _dataBytesSize;
    int _gridWidth;
    int _gridHeight;
    int _ruleRadius;
    int _cycleN;
    float _cycleStep;
    
    float _time;
    float _stepTime;
    float _gridTime;
    bool _pause;
    
    void _prepareFeedback();
    void _prepareFeedbackProgram();
    void _prepareFeedbackBuffers();
    void _prepareFeedbackVertexArray();
    
    void _prepareNextUpdate();
    void _updateFeedback();
    
    void _prepareDrawing();
    void _prepareDrawingProgram();
    void _prepareDrawingBuffers();
    void _prepareDrawingVertexArray();
};

#endif /* GPULife_h */
