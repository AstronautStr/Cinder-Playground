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


void CinderPlaygroundApp::_prepareFeedbackProgram()
{
    cinder::BufferRef buffer = PlatformCocoa::get()->loadResource("cellular.vert")->getBuffer();
    std::string vertexShaderSrcString((char*)buffer->getData());
    GLchar const* const vertexShaderSrc = vertexShaderSrcString.c_str();
    _feedbackShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(_feedbackShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(_feedbackShader);
    std::cerr << lastError(_feedbackShader) << std::endl;
    _WTF
    _feedbackProgram = glCreateProgram();
    glAttachShader(_feedbackProgram, _feedbackShader);
    
    // data format
    const GLchar* feedbackVaryings[] = { "outCellState" };
    glTransformFeedbackVaryings(_feedbackProgram, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glLinkProgram(_feedbackProgram);
    std::cerr << lastError(_feedbackShader) << std::endl;
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

void CinderPlaygroundApp::_prepareFeedbackBuffers()
{
    // data format
    _ruleRadius = 1;
    int diameter = 2 * _ruleRadius + 1;
    _neigborsCount = diameter * diameter - 1;
    
    _gridWidth = 5;
    _gridHeight = 3;
    _dataLength = _gridWidth * _gridHeight;
    _dataBytesSize = _dataLength * sizeof(GLfloat);
    
    _dataInputBuffer = new GLfloat[_dataLength];
    _dataFeedbackBuffer = new GLfloat[_dataLength];
    for (int i = 0; i < _dataLength; ++i)
    {
        _dataFeedbackBuffer[i] = _dataInputBuffer[i] = i + 1;
    }
    glGenBuffers(1, &_vboCells);
    glBindBuffer(GL_ARRAY_BUFFER, _vboCells);
    glBufferData(GL_ARRAY_BUFFER, _dataBytesSize, _dataInputBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    
    CellHelper** cellsTemp = new CellHelper*[_gridWidth];
    
    int maskLength = _gridWidth * _neigborsCount * _gridHeight;
    GLfloat* mask = new GLfloat[maskLength];
    
    for (int i = 0; i < _gridWidth; ++i)
    {
        cellsTemp[i] = new CellHelper[_gridHeight];
        for (int j = 0; j < _gridHeight; ++j)
        {
            cellsTemp[i][j] = CellHelper(i, j);
            cellsTemp[i][j].FillNeighbors(cellsTemp, _gridWidth, _gridHeight, _ruleRadius);
            for (int n = 0; n < _neigborsCount; ++n)
            {
                GLfloat nclass = (GLfloat)cellsTemp[i][j].neighborsClasses[n];
                mask[i * _gridHeight * _neigborsCount + j * _neigborsCount + n] = nclass;
            }
        }
    }
    glGenBuffers(1, &_vboMask);
    glBindBuffer(GL_ARRAY_BUFFER, _vboMask);
    glBufferData(GL_ARRAY_BUFFER, maskLength * sizeof(GLfloat), mask, GL_STATIC_DRAW);
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
    _WTF
    glBindBuffer(GL_ARRAY_BUFFER, _vboMask);
    _WTF
    
    GLint a = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &a);

    GLint inNClasses0Attr = glGetAttribLocation(_feedbackProgram, "inNClasses0");
    glVertexAttribPointer(inNClasses0Attr, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(inNClasses0Attr);
    
    GLint inNClasses1Attr = glGetAttribLocation(_feedbackProgram, "inNClasses1");
    glVertexAttribPointer(inNClasses1Attr, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNClasses1Attr);
    
    glBindBuffer(GL_ARRAY_BUFFER, _vboCells);
    
    int leftStep = -_gridHeight;
    int rightStep = _gridHeight;
    int upStep = -1;
    int downStep = 1;
    
    int leftEdge = leftStep * (_gridWidth - 1);
    int rightEdge = rightStep * (_gridWidth - 1);
    int upEdge = upStep * (_gridHeight - 1);
    int downEdge = downStep * (_gridHeight - 1);

    
    GLint inN012Attr = glGetAttribLocation(_feedbackProgram, "inN012");
    glVertexAttribPointer(inN012Attr, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)((leftStep + upStep) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inN012Attr);
    
    GLint inN3X4Attr = glGetAttribLocation(_feedbackProgram, "inN3X4");
    glVertexAttribPointer(inN3X4Attr, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)((upStep) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inN3X4Attr);
    
    GLint inN567Attr = glGetAttribLocation(_feedbackProgram, "inN567");
    glVertexAttribPointer(inN567Attr, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)((rightStep + upStep) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inN567Attr);
    
    GLint inNE66FAttr = glGetAttribLocation(_feedbackProgram, "inNE66F");
    glVertexAttribPointer(inNE66FAttr, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)((leftEdge + upStep) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNE66FAttr);
    
    GLint inNG11HAttr = glGetAttribLocation(_feedbackProgram, "inNG11H");
    glVertexAttribPointer(inNG11HAttr, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat), (void*)((rightEdge + upStep) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNG11HAttr);
    
    GLint inNBAttr = glGetAttribLocation(_feedbackProgram, "inNB");
    glVertexAttribPointer(inNBAttr, 1, GL_FLOAT, GL_FALSE, 0, (void*)((leftStep + downEdge) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNBAttr);
    
    GLint inN33Attr = glGetAttribLocation(_feedbackProgram, "inN33");
    glVertexAttribPointer(inN33Attr, 1, GL_FLOAT, GL_FALSE, 0, (void*)((downEdge) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inN33Attr);
    
    GLint inNAAttr = glGetAttribLocation(_feedbackProgram, "inNA");
    glVertexAttribPointer(inNAAttr, 1, GL_FLOAT, GL_FALSE, 0, (void*)((rightStep + downEdge) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNAAttr);
    
    GLint inNCAttr = glGetAttribLocation(_feedbackProgram, "inNC");
    glVertexAttribPointer(inNCAttr, 1, GL_FLOAT, GL_FALSE, 0, (void*)((leftStep + upEdge) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNCAttr);
    
    GLint inN44Attr = glGetAttribLocation(_feedbackProgram, "inN44");
    glVertexAttribPointer(inN44Attr, 1, GL_FLOAT, GL_FALSE, 0, (void*)((upEdge) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inN44Attr);
    
    GLint inNDAttr = glGetAttribLocation(_feedbackProgram, "inND");
    glVertexAttribPointer(inNDAttr, 1, GL_FLOAT, GL_FALSE, 0, (void*)((rightStep + upEdge) * sizeof(GLfloat)));
    glEnableVertexAttribArray(inNDAttr);
    
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
    glBindBuffer(GL_ARRAY_BUFFER, _vboCells);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _dataBytesSize, _dataFeedbackBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CinderPlaygroundApp::_updateFeedback()
{
    _prepareNextUpdate();
    
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(_feedbackProgram);
    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _feedbackTfbo);
    glBindVertexArray(_feedbackVao);
    
    // data format
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, _dataLength);
    glEndTransformFeedback();
    
    glDisable(GL_RASTERIZER_DISCARD);
    
    glFlush();

    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _dataBytesSize, _dataFeedbackBuffer);
    glUseProgram(0);
}

CinderPlaygroundApp::~CinderPlaygroundApp()
{
    delete [] _dataInputBuffer;
    delete [] _dataFeedbackBuffer;
    
    glDeleteProgram(_feedbackProgram);
    glDeleteShader(_feedbackShader);
    
    glDeleteBuffers(1, &_feedbackTfbo);
    glDeleteBuffers(1, &_vboCells);
    
    glDeleteVertexArrays(1, &_feedbackVao);
}

void CinderPlaygroundApp::setup()
{
    _prepareFeedback();
    _updateFeedback();
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
}

void CinderPlaygroundApp::draw()
{
	gl::clear( Color( 0, 0, 0 ) );
    
    for (int i = 0; i < _dataLength; ++i)
    {
        gl::drawString(toString(_dataFeedbackBuffer[i]), ivec2(i / _gridHeight, i - _gridHeight * (i / _gridHeight)) * ivec2(30, 30));
    }
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
