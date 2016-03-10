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
#include "AntTweakBar.h"

class KeyEventProxy : public ci::app::KeyEvent
{
protected:
    ci::app::KeyEvent _mEvent;
public:
    KeyEventProxy(ci::app::KeyEvent event)
    {
        _mEvent = event;
    }
    
    unsigned int getModifiers()
    {
        return mModifiers;
    }
};

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
    void mouseWheel( cinder::app::MouseEvent event ) override;
    
    void keyDown( cinder::app::KeyEvent event ) override;
    void update() override;
    void draw() override;
    
protected:
    
    typedef GLfloat CellAttrib;
    typedef GLint RulesBinType;
    static GLenum getFeedbackDataFormat()
    {
        return GL_RGBA32F;
    }
    static unsigned int getAttrCount()
    {
        return 4;
    }
    void getRandomCell(CellAttrib* attr)
    {
        float min = 20.0;
        float max = 22000.0;
        float randFreq = pow(2.0, (log2(min) + (log2(max) - log2(min)) * ((double)rand() / RAND_MAX)));
        float randAmp = (double)rand() / RAND_MAX; randAmp *= randAmp;
        
        unsigned int i = 0;
        if (i < getAttrCount())
            attr[i++] = randAmp;
        
        if (i < getAttrCount())
            attr[i++] = randFreq;
    }
    void getInitialCell(CellAttrib* attr)
    {
        unsigned int i = 0;
        if (i < getAttrCount())
            attr[i++] = 0.0;
        
        if (i < getAttrCount())
            attr[i++] = 440.0;
    }
    void getTrueCell(CellAttrib* attr)
    {
        unsigned int i = 0;
        if (i < getAttrCount())
            attr[i++] = 1.0;
        
        if (i < getAttrCount())
            attr[i++] = 440.0;
    }
    unsigned int encodeNeighborhood(CellAttrib* cells, unsigned int size)
    {
        RulesBinType result = 0;
        
        for (int i = 0; i < size; ++i)
        {
            if (cells[i * getAttrCount()] > 0)
            {
                result = result | (1 << i);
            }
        }
        
        return result;
    }

    GLuint _cellVAO;
    GLuint _cellVBOCells;
    GLuint _cellVBOPos;
    
    GLuint _TBOTex;
    
    GLuint _rulesBuffer;
    GLuint _rulesTexture;
    
    GLuint _cellsSamplerLoc;
    GLuint _rulesSamplerLoc;
    
    GLuint _feedbackBuffer;
    
    GLuint _CAShader;
    GLuint _CAProgram;
    GLuint _timeUniformCALoc;
    
    GLuint _drawingVAO;
    GLuint _drawingVBO;
    
    GLint _drawingScreenSizeLoc;
    GLuint _drawingFragmentShader;
    GLuint _drawingVertexShader;
    GLuint _drawingProgram;
    
    CellAttrib* _dataResultBuffer;
    
    cinder::Font mFont;
    cinder::vec2 _mousePos;
    
    RulesBinType* _rulesData;
    double _lambda;
    int _rulesBits;
    int _rulesLength;
    int _rulesBytesSize;
    int _vertexCount;
    int _dataLength;
    int _dataBytesSize;
    int _gridWidth;
    int _gridHeight;
    
    int _ruleRadius;
    int _cycleN;
    float _cycleStep;
    float _tweakVar;
    
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
    
    void _clearField();
    void _randomField();
    void _randomRules();
    void _logRules();
    void _loadRule(RulesBinType* rule);
    void _updateRulesBuffer();
    
    static void TW_CALL _setCallback(const void* value, void* clientData);
    static void TW_CALL _getCallback(void* value, void* clientData);
};

#endif /* GPULife_h */
