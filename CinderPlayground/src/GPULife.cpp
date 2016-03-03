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
    std::string fragmentShaderSrcString = readAllText(PlatformCocoa::get()->getResourcePath("cellular.frag").string());
    GLchar const* const fragmentShaderSrc = fragmentShaderSrcString.c_str();
    _drawingFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(_drawingFragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(_drawingFragmentShader);
    std::cerr << lastError(_drawingFragmentShader) << std::endl;
    
    std::string vertexShaderSrcString = readAllText(PlatformCocoa::get()->getResourcePath("plain.vert").string());
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
    
    GLint cycleNLoc = glGetUniformLocation(_drawingProgram, "cycleN");
    glProgramUniform1i(_drawingProgram, cycleNLoc, _cycleN);
    GLint cycleStepLoc = glGetUniformLocation(_drawingProgram, "cycleStep");
    glProgramUniform1f(_drawingProgram, cycleStepLoc, _cycleStep);
    
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
    std::string vertexShaderSrcString = readAllText(PlatformCocoa::get()->getResourcePath("cellular.vert").string());
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
    
    GLint ruleRadiusLoc = glGetUniformLocation(_CAProgram, "ruleRadius");
    glProgramUniform1i(_CAProgram, ruleRadiusLoc, _ruleRadius);
    
    GLint cycleNLoc = glGetUniformLocation(_CAProgram, "cycleN");
    glProgramUniform1i(_CAProgram, cycleNLoc, _cycleN);
    GLint cycleStepLoc = glGetUniformLocation(_CAProgram, "cycleStep");
    glProgramUniform1f(_CAProgram, cycleStepLoc, _cycleStep);
}
void CinderPlaygroundApp::_prepareFeedbackBuffers()
{
    srand(time(0));
    // data format
    _dataLength = _gridWidth * _gridHeight;
    _dataBytesSize = _dataLength * sizeof(CellAttrib);
    
    _dataResultBuffer = new CellAttrib[_dataLength];
    int positionBytesSize = _dataLength * 2 * sizeof(GLint);
    GLint* positionData = new GLint[positionBytesSize];
    for (int i = 0; i < _dataLength; ++i)
    {
        GLint x = i / _gridHeight;
        GLint y = i - x * _gridHeight;
        
        CellAttrib newCellState = getRandomCell();
        _dataResultBuffer[i] = newCellState;
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
    glTexBuffer(GL_TEXTURE_BUFFER, getFeedbackDataFormat(), _cellVBOCells);
    
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
    TwTerminate();
    
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
    _stepTime = 0.04;
    _pause = false;
    
    _ruleRadius = 1;
    _cycleN = 4;
    _cycleStep = 1.0 / (float)_cycleN;
    
    _gridWidth = 200;
    _gridHeight = _gridWidth;
    
    _prepareFeedback();
    _prepareDrawing();
    
    Rectf window = Rectf(vec2(0, 0), vec2(_gridWidth, _gridHeight));
    Rectf display = Rectf(vec2(0, 0), getDisplay()->getSize());
    float xRatio = display.getWidth() / window.getWidth();
    float yRatio = display.getHeight() / window.getHeight();
    
    float ratio = math<float>::min(xRatio, yRatio);
    float width = window.getWidth() * ratio;
    float height = window.getHeight() * ratio;
    float x = (display.getWidth() - width) / 2;
    float y = (display.getHeight() - height) / 2;
    //setWindowPos(x, y);
    //setWindowSize(width, height);
    
    int stealthPos = _gridWidth;
    setWindowPos(display.getWidth() - stealthPos, display.getHeight() - stealthPos);
    setWindowSize(stealthPos, stealthPos);
    
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(getWindowWidth(), getWindowHeight());
    
    TwBar* myBar;
    myBar = TwNewBar("TweakBar");
    TwAddVarCB(myBar, "TweakVar", TW_TYPE_FLOAT, _setCallback, _getCallback, this, "min=0.0 max=1.0 step=0.005");
    
    mFont = Font("Helvetica", 12.0f);
}

void TW_CALL CinderPlaygroundApp::_setCallback(const void* value, void* clientData)
{
    static_cast<CinderPlaygroundApp*>(clientData)->_tweakVar = *static_cast<const float*>(value);
}

void TW_CALL CinderPlaygroundApp::_getCallback(void* value, void* clientData)
{
    *static_cast<float*>(value) = static_cast<const CinderPlaygroundApp*>(clientData)->_tweakVar;
}

void CinderPlaygroundApp::modifyCell(vec2 point, bool state)
{
    ivec2 gridPoint = ivec2(math<int>::clamp(point.x / getWindowWidth() * _gridWidth, 0, _gridWidth - 1), math<int>::clamp(point.y / getWindowHeight() * _gridHeight, 0, _gridHeight - 1));
    _dataResultBuffer[gridPoint.x * _gridHeight + gridPoint.y] = state ? 1.0 : 0.0;
}

void CinderPlaygroundApp::keyDown( KeyEvent event )
{
    TwKeyPressed(event.getCode(), KeyEventProxy(event).getModifiers());
    
    switch (event.getCode())
    {
        case KeyEvent::KEY_SPACE:
            _pause = !_pause;
            break;
    }
}

void CinderPlaygroundApp::mouseMove( MouseEvent event )
{
    TwMouseMotion(event.getX(), event.getY());
}

void CinderPlaygroundApp::mouseDrag( MouseEvent event )
{
    TwMouseMotion(event.getX(), event.getY());
    
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

void CinderPlaygroundApp::mouseWheel( cinder::app::MouseEvent event )
{
    TwMouseWheel(event.getWheelIncrement());
}

void CinderPlaygroundApp::mouseUp( MouseEvent event )
{
    TwMouseButton(TwMouseAction::TW_MOUSE_RELEASED, event.isLeft() ? TwMouseButtonID::TW_MOUSE_LEFT : event.isRight() ? TwMouseButtonID::TW_MOUSE_RIGHT : TW_MOUSE_MIDDLE);
    
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
    TwMouseButton(TwMouseAction::TW_MOUSE_PRESSED, event.isLeft() ? TwMouseButtonID::TW_MOUSE_LEFT : event.isRight() ? TwMouseButtonID::TW_MOUSE_RIGHT : TW_MOUSE_MIDDLE);
}

void CinderPlaygroundApp::update()
{
    TwWindowSize(getWindowWidth(), getWindowHeight());
    
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
    
    /*
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

    //gl::drawString("FPS " + toString(getAverageFps()), vec2( 10.0f, 10.0f ), Color::white(), mFont );
    */
    TwDraw();
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
