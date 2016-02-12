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
    
    _feedbackProgram = glCreateProgram();
    glAttachShader(_feedbackProgram, _feedbackShader);
    
    // data format
    const GLchar* feedbackVaryings[] = { "outCellState" };
    glTransformFeedbackVaryings(_feedbackProgram, 1, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);
    
    glLinkProgram(_feedbackProgram);
    std::cerr << lastError(_feedbackShader) << std::endl;
}

const int CinderPlaygroundApp::_ruleRadius = 1;

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
    _gridWidth = _gridHeight = 10;
    _dataLength = _gridWidth * _gridHeight;
    _dataBytesSize = _dataLength * sizeof(GLfloat);
    
    _dataInputBuffer = new GLfloat[_dataLength];
    _dataFeedbackBuffer = new GLfloat[_dataLength];
    for (int i = 0; i < _dataLength; ++i)
    {
        _dataFeedbackBuffer[i] = _dataInputBuffer[i] = 1;
    }
    glGenBuffers(1, &_vboCells);
    glBindBuffer(GL_ARRAY_BUFFER, _vboCells);
    glBufferData(GL_ARRAY_BUFFER, _dataBytesSize, _dataInputBuffer, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    CellHelper** cellsTemp = new CellHelper*[_gridWidth];
    for (int i = 0; i < _gridWidth; ++i)
    {
        cellsTemp[i] = new CellHelper[_gridHeight];
        for (int j = 0; j < _gridHeight; ++j)
        {
            cellsTemp[i][j] = CellHelper(i, j);
            cellsTemp[i][j].FillNeighbors(cellsTemp, _gridWidth, _gridHeight, _ruleRadius);
        }
    }
    const int sectionWidth = 8;
    const int sectionHeigth = 4;
    int maskLength = _gridWidth * (sectionWidth * sectionHeigth) * _gridHeight;
    float* mask = new float[maskLength];

    glGenBuffers(1, &_vboMask);
    glBindBuffer(GL_ARRAY_BUFFER, _vboMask);
    glBufferData(GL_ARRAY_BUFFER, maskLength, mask, GL_STATIC_DRAW);
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
    glBindBuffer(GL_ARRAY_BUFFER, _vboCells);
    
    int counter = 0;
    for (int i = -_ruleRadius; i <= _ruleRadius; ++i)
    {
        for (int j = -_ruleRadius; j <= _ruleRadius; j++)
        {
            std::cerr << counter << endl;
            GLint cellAttrib = glGetAttribLocation(_feedbackProgram, (std::string("inCell") + toString(counter)).c_str());  //_WTF
            int offset = j + i * _gridWidth;
            
            glVertexAttribPointer(cellAttrib, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(offset * sizeof(GLfloat)));              // _WTF
            glEnableVertexAttribArray(cellAttrib);_WTF
            
            counter++;
        }
    }
    
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
    glDrawArrays(GL_POINTS, 0, _dataLength);                                                        _WTF
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
        gl::drawString(toString(_dataFeedbackBuffer[i]), ivec2(i / _gridWidth, i - _gridWidth * (i / _gridWidth)) * ivec2(30, 30));
    }
}
/*
 for (int i = 0; i < _gridWidth; ++i)
 {
 for (int j = 0; j < _gridHeight; ++j)
 {
 int indexCell = i + j * _gridWidth * sectionWidth * sectionHeigth;
 for (int k = 0; k < sectionWidth; ++k)
 {
 int targetInRow = 0;
 bool topEdge = j == 0;
 bool botEdge = j == _gridHeight - 1;
 bool leftEdge = i == 0;
 bool rightEdge = i == _gridWidth - 1;
 
 switch (k)
 {
 case 0:
 if (leftEdge && topEdge)
 targetInRow = 3; // AA
 else if (leftEdge && !topEdge)
 targetInRow = 2; // G
 else if (!leftEdge && topEdge)
 targetInRow = 1; // B
 break;
 case 1:
 if (topEdge)
 targetInRow = 1;
 break;
 case 2:
 if (rightEdge && topEdge)
 targetInRow = 3; // BB
 else if (rightEdge && !topEdge)
 targetInRow = 2; // E
 else if (!rightEdge && topEdge)
 targetInRow = 1; // A
 break;
 case 3:
 if (leftEdge)
 targetInRow = 1;
 break;
 case 4:
 if (rightEdge)
 targetInRow = 1;
 break;
 case 5:
 break;
 case 6:
 if (botEdge)
 targetInRow = 1;
 break;
 case 7:
 if (rightEdge && botEdge)
 targetInRow = 3; // CC
 else if (rightEdge && !botEdge)
 targetInRow = 2; // F
 else if (!rightEdge && botEdge)
 targetInRow = 1; // D
 break;
 }
 for (int l = 0; l < sectionHeigth; ++l)
 {
 int sectionIndex = i + j * sectionWidth;
 mask[indexCell + sectionIndex] = (targetInRow == l) ? 1.0 : 0.0;
 }
 }
 }
 }*/

CINDER_APP( CinderPlaygroundApp, RendererGl )
