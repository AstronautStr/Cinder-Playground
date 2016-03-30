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

#include "cinder/audio/audio.h"

#include "cinder/Utilities.h"
#include "AntTweakBar.h"

#include <iostream>

#define _WTF std::cerr << cinder::gl::getErrorString(cinder::gl::getError()) << std::endl;

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

class RingBuffer
{
protected:
    int _ringBufferSize;
    
    int _ringWritePos;
    int _ringReadPos;
    
    int _partSize;
    int _partsCount;
    
    const int _ringIndex(int index) const
    {
        return index % _ringBufferSize;
    }
    
public:
    
    float* _ringBuffer;
    RingBuffer(int partSize, int partsCount)
    {
        _partSize = partSize;
        _partsCount = partsCount;
        _ringBufferSize = partSize * partsCount;
        _ringBuffer = new float[_ringBufferSize];
        _ringWritePos = 0;
        _ringReadPos = 0;
    }
    ~RingBuffer()
    {
        delete [] _ringBuffer;
    }
    
    int getWritePos() const
    {
        return _ringWritePos;
    }
    void incWritePos(int inc)
    {
        _ringWritePos = _ringIndex(_ringWritePos + inc);
    }
    
    int getReadPos() const
    {
        return _ringReadPos;
    }
    void incReadPos(int inc)
    {
        _ringReadPos = _ringIndex(_ringReadPos + inc);
    }
    
    int getPartSize()
    {
        return _partSize;
    }
    
    float& operator[](int index)
    {
        return _ringBuffer[_ringIndex(index)];
    }
    const float& operator[](int index) const
    {
        return _ringBuffer[_ringIndex(index)];
    }
};

class CinderPlaygroundApp;

class CAGenNode : public ci::audio::GenNode
{
protected:
    RingBuffer* _externalBuffer;
    CinderPlaygroundApp* _CAController;
    int _processedSamples;
    
public:
    CAGenNode(RingBuffer* externalBuffer, CinderPlaygroundApp* CAController);
    void process(ci::audio::Buffer* buffer);
};

typedef std::shared_ptr<class CAGenNode> CAGenNodeRef;

class UniformLink
{
protected:
    GLuint _program;
    
    GLfloat _value;
    GLint _valueLoc;
    
public:
    UniformLink(GLuint program, const std::string& uniformName, GLfloat value = 0.0)
    {
        _program = program;
        _valueLoc = glGetUniformLocation(program, uniformName.c_str());
        setValue(value);
    }
    
    void setValue(GLfloat value)
    {
        _value = value;
        glProgramUniform1f(_program, _valueLoc, _value);
    }
    
    GLfloat getValue() const
    {
        return _value;
    }
};

class CinderPlaygroundApp : public ci::app::App
{
public:
    CinderPlaygroundApp()
    {
        
    }
    ~CinderPlaygroundApp();
    
    void modifyCell(cinder::vec2 point, bool state);
    
    void reportSkippedFrames(int samplesSkipped);
    void processStep();
    
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
    static float getRandFreq()
    {
        float min = 20.0;
        float max = 22000.0;
        return /*200.0 * (1.0 + rand() % 20);*/pow(2.0, (log2(min) + (log2(max) - log2(min)) * ((double)rand() / RAND_MAX)));
    }
    
    void getRandomCell(CellAttrib* attr)
    {
        float randFreq = getRandFreq();
        float randAmp = (double)rand() / RAND_MAX;
        
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
            attr[i++] = 0.0;
    }
    void getTrueCell(CellAttrib* attr)
    {
        unsigned int i = 0;
        if (i < getAttrCount())
            attr[i++] = 1.0;
        
        if (i < getAttrCount())
            attr[i++] = getRandFreq();
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
    GLuint _soundCellsSamplerLoc;
    GLuint _rulesSamplerLoc;
    
    GLuint _feedbackBuffer;
    GLuint _soundFeedbackBuffer;
    
    GLuint _CAShader;
    GLuint _CAProgram;
    GLuint _timeUniformCALoc;
    
    GLuint _soundShader;
    GLuint _soundProgram;
    GLuint _samplesElapsedUniformLoc;
    
    int _samplesBufferLength;
    int _soundBytesSize;
    int _samplesElapsed;
    
    GLfloat* _soundBuffer;
    GLuint _soundVAO;
    GLuint _soundVBOPos;
    
    GLuint _drawingVAO;
    GLuint _drawingVBO;
    
    GLint _drawingScreenSizeLoc;
    GLuint _drawingFragmentShader;
    GLuint _drawingVertexShader;
    GLuint _drawingProgram;
    
    CellAttrib* _dataResultBuffer;
    CellAttrib* _snapshotBuffer;
    
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
    
    UniformLink* _rulesBirthCenter;
    UniformLink* _rulesBirthRadius;
    UniformLink* _rulesKeepCenter;
    UniformLink* _rulesKeepRadius;
    UniformLink* _rulesDelta;
    bool _linkRuleCenters;
    
    float _time;
    float _stepTime;
    float _gridTime;
    bool _pause;
    bool _showBar;
    
    bool _isScheduledStep;
    void _processStep();
    
    void _prepareFeedback();
    void _prepareFeedbackProgram();
    void _prepareFeedbackBuffers();
    void _prepareFeedbackVertexArray();
    
    void _prepareNextUpdate();
    void _updateFeedback();
    
    void _prepareSoundFeedback();
    void _prepareSoundFeedbackProgram();
    void _prepareSoundFeedbackBuffers();
    void _prepareSoundFeedbackVertexArray();
    
    void _prepareNextBuffer();
    void _generateSoundBuffer();
    
    int getSampleRate();
    RingBuffer* _ringBuffer;
    int _samplesSkipped;
    CAGenNodeRef _CAGenNode;
    
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
    void _saveSnapshot();
    void _loadSnapshot();
    
    static void TW_CALL _setCallback(const void* value, void* clientData);
    static void TW_CALL _getCallback(void* value, void* clientData);
};

#endif /* GPULife_h */
