#include "CinderPlaygroundApp.h"
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


void CinderPlaygroundApp::_prepareFeedbackProgram()
{
    cinder::BufferRef buffer = PlatformCocoa::get()->loadResource("cellular.vert")->getBuffer();
    std::string vertexShaderSrcString((char*)buffer->getData());
    GLchar const* const vertexShaderSrc = vertexShaderSrcString.c_str();
    _feedbackShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_feedbackShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(_feedbackShader);
    std::cerr << lastError(_feedbackShader) << std::endl;
    
    _feedbackProgram = glCreateProgram();
    glAttachShader(_feedbackProgram, _feedbackShader);
    
    // data format
    const GLchar* feedbackVaryings[] = { "outValue" };
    glTransformFeedbackVaryings(_feedbackProgram, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glLinkProgram(_feedbackProgram);
    std::cerr << lastError(_feedbackShader) << std::endl;
}

void CinderPlaygroundApp::_prepareFeedbackBuffers()
{
    // data format
    _dataLength = 5;
    _dataBytesSize = _dataLength * sizeof(GLfloat);
    _dataInputBuffer = new GLfloat[_dataLength];
    _dataFeedbackBuffer = new GLfloat[_dataLength];
    for (int i = 0; i < _dataLength; ++i)
    {
        _dataFeedbackBuffer[i] = _dataInputBuffer[i] = (GLfloat)i;
    }
    
    glGenBuffers(1, &_feedbackVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _feedbackVbo);
    glBufferData(GL_ARRAY_BUFFER, _dataBytesSize, _dataInputBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &_feedbackTfbo);
    glBindBuffer(GL_ARRAY_BUFFER, _feedbackTfbo);
    glBufferData(GL_ARRAY_BUFFER, _dataBytesSize, NULL, GL_STATIC_COPY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CinderPlaygroundApp::_prepareFeedbackVertexArray()
{
    glGenVertexArrays(1, &_feedbackVao);
    glBindVertexArray(_feedbackVao);
    glBindBuffer(GL_ARRAY_BUFFER, _feedbackVbo);
    
    GLint inputAttrib = glGetAttribLocation(_feedbackProgram, "inValue");
    // data format
    glVertexAttribPointer(inputAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(inputAttrib);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void CinderPlaygroundApp::_prepareFeedback()
{
    _prepareFeedbackProgram();
    _prepareFeedbackBuffers();
    _prepareFeedbackVertexArray();
}

void CinderPlaygroundApp::_prepareNextUpdate()
{
    glBindBuffer(GL_ARRAY_BUFFER, _feedbackVbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _dataBytesSize, _dataFeedbackBuffer);                           _WTF
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CinderPlaygroundApp::_updateFeedback()
{
    _prepareNextUpdate();
    
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(_feedbackProgram);
    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _feedbackTfbo);
    glBindVertexArray(_feedbackVao);                                                                    _WTF
    
    // data format
    glBeginTransformFeedback(GL_POINTS);                                                                _WTF
    glDrawArrays(GL_POINTS, 0, _dataLength);                                                            _WTF
    glEndTransformFeedback();
    
    glDisable(GL_RASTERIZER_DISCARD);
    
    glFlush();

    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _dataBytesSize, _dataFeedbackBuffer);           _WTF
    glUseProgram(0);
}

CinderPlaygroundApp::~CinderPlaygroundApp()
{
    delete [] _dataInputBuffer;
    delete [] _dataFeedbackBuffer;
    
    glDeleteProgram(_feedbackProgram);
    glDeleteShader(_feedbackShader);
    
    glDeleteBuffers(1, &_feedbackTfbo);
    glDeleteBuffers(1, &_feedbackVbo);
    
    glDeleteVertexArrays(1, &_feedbackVao);
}

void CinderPlaygroundApp::setup()
{
    _prepareFeedback();
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
    _updateFeedback();
}

void CinderPlaygroundApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    for (int i = 0; i < _dataLength; ++i)
    {
        gl::drawString(toString(_dataFeedbackBuffer[i]), ivec2(0, 50 * i));
    }
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
