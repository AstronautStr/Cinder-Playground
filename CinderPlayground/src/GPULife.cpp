#include "GPULife.h"
#include "cinder/Timeline.h"
#include "cinder/app/cocoa/PlatformCocoa.h"
#include <fstream>

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
    
    _timeUniformCALoc = glGetUniformLocation(_CAProgram, "time");
    _cellsSamplerLoc = glGetUniformLocation(_CAProgram, "cellsSampler");
    _rulesSamplerLoc = glGetUniformLocation(_CAProgram, "rulesSampler");
    
    
    TwBar* CAControl;
    CAControl = TwNewBar("CAControl");
    _showBar = false;
    TwDefine(" CAControl visible=false ");
    
    _rulesBirthCenter = new UniformLink(_CAProgram, "rulesBirthCenter", 3.5);
    _rulesBirthRadius = new UniformLink(_CAProgram, "rulesBirthRadius", 0.5);
    _rulesKeepCenter = new UniformLink(_CAProgram, "rulesKeepCenter", 3.0);
    _rulesKeepRadius = new UniformLink(_CAProgram, "rulesKeepRadius", 1.0);
    
    TwAddVarCB(CAControl, "rulesBirthCenter", TW_TYPE_FLOAT, _setCallback, _getCallback, _rulesBirthCenter, "min=0.0 max=9.0 step=0.01");
    TwAddVarCB(CAControl, "rulesBirthRadius", TW_TYPE_FLOAT, _setCallback, _getCallback, _rulesBirthRadius, "min=0.0 max=9.0 step=0.01");
    TwAddVarCB(CAControl, "rulesKeepCenter", TW_TYPE_FLOAT, _setCallback, _getCallback, _rulesKeepCenter, "min=0.0 max=9.0 step=0.01");
    TwAddVarCB(CAControl, "rulesKeepRadius", TW_TYPE_FLOAT, _setCallback, _getCallback, _rulesKeepRadius, "min=0.0 max=9.0 step=0.01");
}
void TW_CALL CinderPlaygroundApp::_setCallback(const void* value, void* clientData)
{
    static_cast<UniformLink*>(clientData)->setValue(*static_cast<const GLfloat*>(value));
}

void TW_CALL CinderPlaygroundApp::_getCallback(void* value, void* clientData)
{
    *static_cast<GLfloat*>(value) = static_cast<const UniformLink*>(clientData)->getValue();
}
void CinderPlaygroundApp::_prepareFeedbackBuffers()
{
    srand(time(0));
    // data format
    _vertexCount = _gridWidth * _gridHeight;
    _dataLength = _vertexCount * getAttrCount();
    _dataBytesSize = _dataLength * sizeof(CellAttrib);
    
    _dataResultBuffer = new GLfloat[_dataLength];
    
    int positionBytesSize = _vertexCount * 2 * sizeof(GLint);
    GLint* positionData = new GLint[positionBytesSize];
    
    for (int i = 0; i < _vertexCount; ++i)
    {
        GLint x = i / _gridHeight;
        GLint y = i - x * _gridHeight;
        
        getRandomCell(&_dataResultBuffer[i * getAttrCount()]);
        
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
    
    _rulesBits = 9;
    const int stateCount = 2;
    _rulesLength = pow(stateCount, _rulesBits);
    _rulesBytesSize = _rulesLength * sizeof(RulesBinType);
    
    glGenBuffers(1, &_rulesBuffer);
    
    glGenTextures(1, &_rulesTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, _rulesTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _rulesBuffer);
    
    _rulesData = new RulesBinType[_rulesLength];
    _randomRules();
    
    glGenBuffers(1, &_rulesBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _rulesBuffer);
    glBufferData(GL_ARRAY_BUFFER, _rulesBytesSize, _rulesData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenTextures(1, &_rulesTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, _rulesTexture);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, _rulesBuffer);

    
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
    glProgramUniform1f(_CAProgram, _timeUniformCALoc, _time);
    
    glBindBuffer(GL_ARRAY_BUFFER, _cellVBOCells);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _dataBytesSize, _dataResultBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void CinderPlaygroundApp::_updateFeedback()
{
    glEnable(GL_RASTERIZER_DISCARD);
    glUseProgram(_CAProgram);
    
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _feedbackBuffer);
    glBindVertexArray(_cellVAO);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, _TBOTex);
    glUniform1i(_cellsSamplerLoc, 0);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, _rulesTexture);
    glUniform1i(_rulesSamplerLoc, 1);
    
    // data format
    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, _vertexCount);
    glEndTransformFeedback();
    
    glDisable(GL_RASTERIZER_DISCARD);
    
    glFlush();
    glUseProgram(0);
    
    // recieve processed data from feedback
    glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _dataBytesSize, _dataResultBuffer);
}

CinderPlaygroundApp::~CinderPlaygroundApp()
{
    TwTerminate();
    
    delete _rulesBirthCenter;
    delete _rulesBirthRadius;
    delete _rulesKeepCenter;
    delete _rulesKeepRadius;
    
    delete [] _dataResultBuffer;
    delete [] _rulesData;
    
    glDeleteProgram(_CAProgram);
    glDeleteShader(_CAShader);
    
    glDeleteProgram(_drawingProgram);
    glDeleteShader(_drawingFragmentShader);
    glDeleteShader(_drawingVertexShader);
    
    glDeleteBuffers(1, &_feedbackBuffer);
    glDeleteBuffers(1, &_cellVBOCells);
    glDeleteBuffers(1, &_cellVBOPos);
    glDeleteBuffers(1, &_drawingVBO);
    glDeleteBuffers(1, &_rulesBuffer);
    
    glDeleteTextures(1, &_TBOTex);
    glDeleteTextures(1, &_rulesTexture);
    
    glDeleteVertexArrays(1, &_cellVAO);
    glDeleteVertexArrays(1, &_drawingVAO);
}



void CinderPlaygroundApp::setup()
{
    _time = _gridTime = 0.0;
    _stepTime = 0.01;
    _pause = false;
    
    _lambda = 0.25;
    _ruleRadius = 1;
    _cycleN = 4;
    _cycleStep = 1.0 / (float)_cycleN;
    
    _gridWidth = 64;
    _gridHeight = _gridWidth;
    
    TwInit(TW_OPENGL_CORE, NULL);
    TwWindowSize(getWindowWidth(), getWindowHeight());
    
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
    setWindowPos(x, y);
    setWindowSize(width, height);
    
    bool stealth = true;
    if (stealth)
    {
        int stealthPos = _gridWidth;
        setWindowPos(display.getWidth() - stealthPos, display.getHeight() - stealthPos);
        setWindowSize(stealthPos, stealthPos);
    }

    mFont = Font("Helvetica", 12.0f);
}

void CinderPlaygroundApp::modifyCell(vec2 point, bool state)
{
    ivec2 gridPoint = ivec2(math<int>::clamp(point.x / getWindowWidth() * _gridWidth, 0, _gridWidth - 1), math<int>::clamp(point.y / getWindowHeight() * _gridHeight, 0, _gridHeight - 1));
    
    CellAttrib* attr = &_dataResultBuffer[(gridPoint.x * _gridHeight + gridPoint.y) * getAttrCount()];
    if (state)
        getTrueCell(attr);
    else
        getInitialCell(attr);
}

void CinderPlaygroundApp::_clearField()
{
    for (int i = 0; i < _vertexCount; ++i)
    {
        getInitialCell(&_dataResultBuffer[i * getAttrCount()]);
    }
}

void CinderPlaygroundApp::_randomField()
{
    for (int i = 0; i < _vertexCount; ++i)
    {
        getRandomCell(&_dataResultBuffer[i * getAttrCount()]);
    }
}


void CinderPlaygroundApp::_updateRulesBuffer()
{
    glBindBuffer(GL_ARRAY_BUFFER, _rulesBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _rulesBytesSize, _rulesData);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void CinderPlaygroundApp::_randomRules()
{
    RulesBinType center = _rulesBits / 2;
    for (RulesBinType i = 0; i < _rulesLength; ++i)
    {
        RulesBinType centralState = ( (i & (1 << center)) >> center);
        
        double randR = (double)rand() / RAND_MAX;
        if (randR <= _lambda)
            _rulesData[i] = (centralState + 1) % 2;
        else
            _rulesData[i] = centralState;
    }
    
    _updateRulesBuffer();
}

void CinderPlaygroundApp::_logRules()
{
    std::cout << "<ruledump>";
    for (RulesBinType i = 0; i < _rulesLength; ++i)
    {
        std::cout << _rulesData[i];
    }
    std::cout << "</ruledump>" << std::endl;
}

void CinderPlaygroundApp::_loadRule(RulesBinType* rule)
{
    char* rule1 =  "00000101000100001111000101111110000001001000010011110101110110010000011000000000001111011110111110110110000000101110101000011111000000001000010011111100101100110000000010000011001010011101011001000100000010101111111111100111000000010000100001011110001100000011000000000000110111111101111100001101100100011111110110001101000000000000000001100111101001110010010011000000010100111011110001101000000100001011111011011011011111000100010011111110110010010000000000000100011111111111011110100000000000101011111000111011";
    
    char* triangle = "01011000011001000011010100101011001010010010000011010011011111110000100001100001011011011011111100001000000001010111111110100100100000100001000011011111110111110011010100001010111110010110011110001010000001001101110101011110110001000000101011011111101010110100110000000100011101111111001101101001000010001011110001111011000000000000000111110011111001110000000000000010101111110011111110001010000000001111110111111111110100110010000010101111110111110000000001001001111110111110011100000000100000101111011111110101";
    
    for (int i = 0; i < _rulesLength; ++i)
    {
        _rulesData[i] = (triangle[i] == '0' ? 0 : 1);
    }
    _updateRulesBuffer();
}

void CinderPlaygroundApp::keyDown( KeyEvent event )
{
    //TwKeyPressed(event.getCode(), KeyEventProxy(event).getModifiers());
    
    switch (event.getCode())
    {
        case KeyEvent::KEY_SPACE:
            _pause = !_pause;
            break;
            
        case KeyEvent::KEY_c:
            _clearField();
            break;
            
        case KeyEvent::KEY_r:
            _randomField();
            break;
            
        case KeyEvent::KEY_q:
            _randomRules();
            _randomField();
            break;
            
        case KeyEvent::KEY_s:
            _logRules();
            break;
            
        case KeyEvent::KEY_l:
            _loadRule(0);
            break;
            
        case KeyEvent::KEY_b:
            _showBar = !_showBar;
            if (_showBar)
                TwDefine(" CAControl visible=true ");
            else
                TwDefine(" CAControl visible=false ");
            break;
    }
}

void CinderPlaygroundApp::mouseMove( MouseEvent event )
{
    _mousePos = event.getPos();
    
    if (TwMouseMotion(event.getX(), event.getY()))
        return;
}

void CinderPlaygroundApp::mouseDrag( MouseEvent event )
{
    _mousePos = event.getPos();
    
    if (TwMouseMotion(event.getX(), event.getY()))
        return;
    
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
    if (TwMouseWheel(event.getWheelIncrement()))
        return;
}

void CinderPlaygroundApp::mouseUp( MouseEvent event )
{
    _mousePos = event.getPos();
    
    if (TwMouseButton(TwMouseAction::TW_MOUSE_RELEASED, event.isLeft() ? TwMouseButtonID::TW_MOUSE_LEFT : event.isRight() ? TwMouseButtonID::TW_MOUSE_RIGHT : TW_MOUSE_MIDDLE))
        return;
    
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
    if (TwMouseButton(TwMouseAction::TW_MOUSE_PRESSED, event.isLeft() ? TwMouseButtonID::TW_MOUSE_LEFT : event.isRight() ? TwMouseButtonID::TW_MOUSE_RIGHT : TW_MOUSE_MIDDLE))
        return;
}

void CinderPlaygroundApp::update()
{
    TwWindowSize(getWindowWidth(), getWindowHeight());
    
    float newTime = timeline().getCurrentTime();
    float dt = newTime - _time;
    _time = newTime;
    
    bool enableGridUpdate = true;
    _prepareNextUpdate();
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

    //gl::drawString("FPS " + toString(getAverageFps()), vec2( 10.0f, 10.0f ), Color::white(), mFont );
    
    vec2 cellRectSize = vec2((float)getWindowWidth() / _gridWidth, (float)getWindowHeight() / _gridHeight);
    ivec2 mouseCell = (ivec2)(_mousePos / cellRectSize);
    vec2 mouseCellPoint = (vec2)mouseCell * cellRectSize;

    gl::color(1.0, 0.0, 0.0);
    gl::drawSolidRect(Rectf(mouseCellPoint, mouseCellPoint + cellRectSize));
    
    TwDraw();
}

CINDER_APP( CinderPlaygroundApp, RendererGl )
