#include "GPULife.h"
#include "cinder/Timeline.h"
#include "cinder/app/cocoa/PlatformCocoa.h"
#include <fstream>

#define _WTF std::cerr << gl::getErrorString(gl::getError()) << endl;

using namespace ci;
using namespace ci::app;
using namespace std;

std::string readAllText(std::string const& path)
{
    std::ifstream ifstr(path);
    if (!ifstr)
        return std::string();
    std::stringstream text;
    ifstr >> text.rdbuf();
    return text.str();
}

typedef std::basic_string<GLchar>  GLstring;

GLstring lastError(GLuint shader) 
{
    GLint logLen{ 0 };
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
    std::vector<GLchar> log;
    log.resize(logLen, '\0');
    glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
    
    return GLstring(std::begin(log), std::end(log));
}

int cycledIndex(int index, int length)
{
    if (index < 0)
        return cycledIndex(length + index, length);
    else if (index >= length)
        return cycledIndex(index - length, length);
    else
        return index;
}



void CinderPlaygroundApp::_prepareDrawing()
{
    _prepareDrawingProgram();
    _prepareDrawingBuffers();
    _prepareDrawingVertexArray();
}
void CinderPlaygroundApp::_prepareDrawingProgram()
{
    cinder::BufferRef bufferF = PlatformCocoa::get()->loadResource("cellular.frag")->getBuffer();
    std::string fragmentShaderSrcString((char*)bufferF->getData());
    GLchar const* const fragmentShaderSrc = fragmentShaderSrcString.c_str();
    _drawingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(_drawingFragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(_drawingFragmentShader);
    std::cerr << lastError(_drawingFragmentShader) << std::endl;
    
    cinder::BufferRef bufferV = PlatformCocoa::get()->loadResource("plain.vert")->getBuffer();
    std::string vertexShaderSrcString((char*)bufferV->getData());
    GLchar const* const vertexShaderSrc = vertexShaderSrcString.c_str();
    _drawingVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_drawingVertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(_drawingVertexShader);
    std::cerr << lastError(_drawingVertexShader) << std::endl;
    
    _drawingProgram = glCreateProgram();
    glAttachShader(_drawingProgram, _drawingVertexShader);
    glAttachShader(_drawingProgram, _drawingFragmentShader);
    
    glLinkProgram(_drawingProgram);
    std::cerr << lastError(_drawingProgram) << std::endl;
    
    GLint gridSizeLoc = glGetUniformLocation(_drawingProgram, "gridSize");
    glProgramUniform2f(_drawingProgram, gridSizeLoc, (GLfloat)_gridWidth, (GLfloat)_gridHeight);
    
    _drawingScreenSizeLoc = glGetUniformLocation(_drawingProgram, "screenSize");
}
void CinderPlaygroundApp::_prepareDrawingBuffers()
{
    GLfloat plane[] =   {  -1.0,   -1.0,   0.0,
                           -1.0,    1.0,   0.0,
                            1.0,   -1.0,   0.0,
                            1.0,    1.0,   0.0
                        };
    
    glGenBuffers(1, &_drawingVBO);
    glBindBuffer(GL_ARRAY_BUFFER, _drawingVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void CinderPlaygroundApp::_prepareDrawingVertexArray()
{
    glGenVertexArrays(1, &_drawingVAO);
    glBindVertexArray(_drawingVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, _drawingVBO);
    
    GLint inPositionAttr = glGetAttribLocation(_drawingProgram, "ciPosition");
    glVertexAttribPointer(inPositionAttr, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(inPositionAttr);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CinderPlaygroundApp::_prepareFeedback()
{
    _prepareFeedbackProgram();
    _prepareFeedbackBuffers();
    _prepareFeedbackVertexArray();
}
void CinderPlaygroundApp::_prepareFeedbackProgram()
{
    cinder::BufferRef buffer = PlatformCocoa::get()->loadResource("cellular.vert")->getBuffer();
    std::string vertexShaderSrcString((char*)buffer->getData());
    GLchar const* const vertexShaderSrc = vertexShaderSrcString.c_str();
    _CAShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_CAShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(_CAShader);
    std::cerr << lastError(_CAShader) << std::endl;
    _WTF
    _CAProgram = glCreateProgram();
    glAttachShader(_CAProgram, _CAShader);
    
    // data format
    const GLchar* feedbackVaryings[] = { "outCellState" };
    glTransformFeedbackVaryings(_CAProgram, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glLinkProgram(_CAProgram);
    std::cerr << lastError(_CAProgram) << std::endl;
    
    GLint gridSizeLoc = glGetUniformLocation(_CAProgram, "GridSize");
    glProgramUniform2i(_CAProgram, gridSizeLoc, _gridWidth, _gridHeight);
}
void CinderPlaygroundApp::_prepareFeedbackBuffers()
{
    // data format
    _dataLength = _gridWidth * _gridHeight;
    _dataBytesSize = _dataLength * sizeof(GLfloat);
    
    _dataResultBuffer = new GLfloat[_dataLength];
    int positionBytesSize = _dataLength * 2 * sizeof(GLint);
    GLint* positionData = new GLint[positionBytesSize];
    for (int i = 0; i < _dataLength; ++i)
    {
        GLint x = i / _gridHeight;
        GLint y = i - x * _gridHeight;
        
        _dataResultBuffer[i] = i;
        positionData[i * 2] = x;
        positionData[i * 2 + 1] = y;
    }

    glGenBuffers(1, &_cellVBOCells);
    glBindBuffer(GL_ARRAY_BUFFER, _cellVBOCells);
    glBufferData(GL_ARRAY_BUFFER, _dataBytesSize, _dataResultBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenTextures(1, &_TBOTex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _TBOTex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, _cellVBOCells);
    
    glGenBuffers(1, &_cellVBOPos);
    glBindBuffer(GL_ARRAY_BUFFER, _cellVBOPos);
    glBufferData(GL_ARRAY_BUFFER, positionBytesSize, positionData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    delete [] positionData;
    
    glGenBuffers(1, &_feedbackBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _feedbackBuffer);
    glBufferData(GL_ARRAY_BUFFER, _dataBytesSize, NULL, GL_STATIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void CinderPlaygroundApp::_prepareFeedbackVertexArray()
{
    glGenVertexArrays(1, &_cellVAO);
    glBindVertexArray(_cellVAO);

    glBindBuffer(GL_ARRAY_BUFFER, _cellVBOPos);
    GLint inPositionAttr = glGetAttribLocation(_CAProgram, "inPosition");
    glVertexAttribIPointer(inPositionAttr, 2, GL_INT, 0, NULL);
    glEnableVertexAttribArray(inPositionAttr);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}



void CinderPlaygroundApp::_prepareNextUpdate()
{
    glBindBuffer(GL_ARRAY_BUFFER, _cellVBOCells);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _dataBytesSize, _dataResultBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void CinderPlaygroundApp::_updateFeedback()
{
    _prepareNextUpdate();
    
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(_CAProgram);
    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _feedbackBuffer);
    glBindVertexArray(_cellVAO);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _TBOTex);
    glBindSampler(0, 0);
    
    // data format
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, _dataLength);
    glEndTransformFeedback();
    
    glDisable(GL_RASTERIZER_DISCARD);
    
    glFlush();

    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _dataBytesSize, _dataResultBuffer);
    glUseProgram(0);
}

CinderPlaygroundApp::~CinderPlaygroundApp()
{
    delete [] _dataResultBuffer;
    
    glDeleteProgram(_CAProgram);
    glDeleteShader(_CAShader);
    
    glDeleteProgram(_drawingProgram);
    glDeleteShader(_drawingFragmentShader);
    glDeleteShader(_drawingVertexShader);
    
    glDeleteBuffers(1, &_feedbackBuffer);
    glDeleteBuffers(1, &_cellVBOCells);
    glDeleteBuffers(1, &_cellVBOPos);
    glDeleteBuffers(1, &_drawingVBO);
    
    glDeleteTextures(1, &_TBOTex);
    
    glDeleteVertexArrays(1, &_cellVAO);
    glDeleteVertexArrays(1, &_drawingVAO);
}

void CinderPlaygroundApp::setup()
{
    _time = _gridTime = 0.0;
    _stepTime = 0.5;
    _pause = false;
    
    _ruleRadius = 1;
    _gridWidth = 10;
    _gridHeight = 10;
    
    _prepareFeedback();
    _prepareDrawing();
}

void CinderPlaygroundApp::modifyCell(vec2 point, bool state)
{
    ivec2 gridPoint = ivec2(math<int>::clamp(point.x / getWindowWidth() * _gridWidth, 0, _gridWidth - 1), math<int>::clamp(point.y / getWindowHeight() * _gridHeight, 0, _gridHeight - 1));
    _dataResultBuffer[gridPoint.x * _gridHeight + gridPoint.y] = state ? 1.0 : 0.0;
}

void CinderPlaygroundApp::keyDown( KeyEvent event )
{
    switch (event.getCode())
    {
        case KeyEvent::KEY_SPACE:
            _pause = !_pause;
            break;
    }
}

void CinderPlaygroundApp::mouseMove( MouseEvent event )
{
}

void CinderPlaygroundApp::mouseDrag( MouseEvent event )
{
    _mousePos = event.getPos();
    
    if (event.isLeft())
    {
        modifyCell(event.getPos(), true);
    }
    else if (event.isRight())
    {
        modifyCell(event.getPos(), false);
    }
}

void CinderPlaygroundApp::mouseUp( MouseEvent event )
{
    _mousePos = event.getPos();
    
    if (event.isLeft())
    {
        modifyCell(event.getPos(), true);
    }
    else if (event.isRight())
    {
        modifyCell(event.getPos(), false);
    }
}

void CinderPlaygroundApp::mouseDown( MouseEvent event )
{
}

void CinderPlaygroundApp::update()
{
    float newTime = timeline().getCurrentTime();
    float dt = newTime - _time;
    _time = newTime;
    
    if (!_pause)
    {
        _gridTime += dt;
        if (_gridTime >= _stepTime)
        {
            _updateFeedback();
            _gridTime -= _stepTime;
        }
    }
}

void CinderPlaygroundApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    glUseProgram(_drawingProgram);
    glBindVertexArray(_drawingVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _drawingVBO);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _TBOTex);
    glBindSampler(0, 0);
    
    glProgramUniform2f(_drawingProgram, _drawingScreenSizeLoc, (GLfloat)getWindowWidth(), (GLfloat)getWindowHeight());
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);
    /*
    for (int i = 0; i < _dataLength; ++i)
    {
        gl::drawString(toString(_dataResultBuffer[i]), ivec2(i / _gridHeight, i - _gridHeight * (i / _gridHeight)) * ivec2(30, 30));
    }*/
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
