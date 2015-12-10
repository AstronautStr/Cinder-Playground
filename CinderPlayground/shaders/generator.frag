#version 330 core

layout(origin_upper_left) in vec4 gl_FragCoord;

uniform float time;
uniform float dt;
uniform float fps;
uniform float screenWidth;
uniform float screenHeight;
uniform float mouseX;
uniform float mouseY;
uniform float scrollPos;
uniform float externalValue;


struct Joystick
{
    float x;
    float y;
};

uniform Joystick leftJoystick;
uniform Joystick rightJoystick;
uniform float leftTrigger;
uniform float rightTrigger;

out vec4 color;

#define ARGUMENTS_CONTROL

#define M_PI 3.1415926535897932384626433832795
#define M_2PI (M_PI * 2)
#define M_PI_2 (M_PI / 2)

#define NORM(x)                     ((x + 1.0) / 2)

#define NORMSIN(x)                  ((sin(x) + 1.0) / 2)
#define NORMSIN_T(x)                (NORMSIN(x * time))
#define NORMSIN_HZ(x)               (NORMSIN(M_2PI * x))
#define NORMSIN_HZ_T(x)             (NORMSIN(M_2PI * x * time))
#define NORMSIN_HZ_T_PH(x, phase)   (NORMSIN(M_2PI * x * time + phase))

#ifdef ARGUMENTS_CONTROL
    #define NORMSQUARE(x, w)            (NORMSIN(x) >= clamp(w, 0.0, 1.0) ? 1.0 : 0.0)
#else
    #define NORMSQUARE(x, w)            (NORMSIN(x) >= w ? 1.0 : 0.0)
#endif
#define NORMSQUARE_T(x, w)              (NORMSQUARE(x * time, w))
#define NORMSQUARE_HZ(x, w)             (NORMSQUARE(M_2PI * x, w))
#define NORMSQUARE_HZ_T(x, w)           (NORMSQUARE(M_2PI * x * time, w))
#define NORMSQUARE_HZ_T_PH(x, ph, w)    (NORMSQUARE(M_2PI * x * time + ph, w))

#define GOLD 1.6180339

vec3 additiveBlend(vec4 first, vec4 second)
{
    return vec3(first.w * first.x + second.w * second.x, first.w * first.y + second.w * second.y, first.w * first.z + second.w * second.z);
}

vec4 presence(vec4 objectColor, float objectPresence)
{
    return vec4(objectColor.x * objectPresence, objectColor.y * objectPresence, objectColor.z * objectPresence, objectColor.w);
}

float drawCircle(vec2 center, float radius)
{
    return clamp(radius - sqrt((gl_FragCoord.x - center.x) * (gl_FragCoord.x - center.x) + (gl_FragCoord.y - center.y) * (gl_FragCoord.y - center.y)), 0.0, 1.0);
}

float clip(float value)
{
    return clamp(value, 0.0, 1.0);
}

vec4 circlesDemo()
{
    float value = 0.0;
    float alpha = 0.0;
    float iterationsCount = 10;
    
    vec2 commonCenter = vec2(screenWidth / 2, screenHeight / 2);
    
    for (float i = 0; i < iterationsCount; i += 1.0)
    {
        float phaseOffset = -1 * i / iterationsCount * M_PI / 16;
        float amp = 1.0 - i / iterationsCount * 0.9;
        
        // Frequency in Hz
        float freq = 0.5;
        float modulatorFreq = 1.1 * freq;
        float modulatorPower = 0.3;
        float sinModulator = sin(modulatorFreq * time * M_2PI + phaseOffset);
        float sinSignal = sin(freq * time * M_2PI + modulatorPower * sinModulator * M_2PI + phaseOffset);
        float cosSignal = cos(freq * time * M_2PI + modulatorPower * sinModulator * M_2PI + phaseOffset);
        
        
        float circle1 = drawCircle(vec2(commonCenter.x - 25 - sinSignal * 50, commonCenter.y - cosSignal * 50), 20);
        float circle2 = drawCircle(vec2(commonCenter.x + 25 + sinSignal * 50, commonCenter.y + cosSignal * 50), 20);
        
        value += clamp(circle1 + circle2, 0.0, 1.0);
        alpha += amp;
    }
    
    return vec4(value / iterationsCount, 0.0, 0.0, alpha / iterationsCount);
}

vec4 circularWavesDemo()
{
    vec2 commonCenter = vec2(screenWidth / 2, screenHeight / 2);
    int phasesPerScreen = 8;
    float offsetFreq = 0.25;
    float offset = 10 * NORMSIN(M_2PI * time * offsetFreq / 2);
    
    float freqHz = 4.0;
    
    float first = 0.0;
    float second = 0.0;
    
    float pixelPhaseFirst = M_2PI * phasesPerScreen * distance(commonCenter + vec2(offset * cos(M_2PI * time * offsetFreq), offset * sin(M_2PI * time * offsetFreq)), gl_FragCoord.xy) / screenHeight;
    float pixelPhaseSecond = M_2PI * phasesPerScreen * distance(commonCenter - vec2(offset * cos(M_2PI * time * offsetFreq), offset * sin(M_2PI * time * offsetFreq)), gl_FragCoord.xy) / screenHeight;
    //first = NORMSQUARE(M_2PI * time * freqHz + pixelPhaseFirst, 0.99);
    //second = NORMSQUARE(M_2PI * time * freqHz + pixelPhaseSecond, 0.99);
    first = NORMSIN(M_2PI * time * freqHz + pixelPhaseFirst);
    second = NORMSIN(M_2PI * time * freqHz + pixelPhaseSecond);
    
    float merge = 1.0;//NORMSQUARE_HZ_T(8.0, 0.5);
    float signal = merge * first + (1.0 - merge) * second;
    
    return vec4(signal, signal, 0.0, 1.0);
}

vec2 polar(vec2 dPoint)
{
    return vec2(sqrt(dPoint.x * dPoint.x + dPoint.y * dPoint.y), atan(dPoint.y, dPoint.x));
}

vec2 hpolar(vec2 dPoint)
{
    return vec2((dPoint.x * dPoint.x + dPoint.y * dPoint.y), atan(dPoint.y, dPoint.x));
}

vec2 decart(vec2 pPoint)
{
    return vec2(pPoint.x * cos(pPoint.y), pPoint.x * sin(pPoint.y));
}


vec2 polar_lofi(vec2 dPoint)
{
    return vec2(sqrt(dPoint.x * dPoint.x + dPoint.y * dPoint.y), atan(dPoint.y, dPoint.x));
}

vec2 decart_lofi(vec2 pPoint)
{
    return vec2(pPoint.x * cos(pPoint.y), pPoint.x * sin(pPoint.y));
}

vec4 hyperpolarityDemo(vec2 fragmentPoint)
{
    float halfWidth = screenWidth / 2;
    float halfHeight = screenHeight / 2;
    vec2 center = vec2(halfWidth, halfHeight);
    vec2 camera = vec2(halfWidth, screenHeight);
    
    vec2 dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    vec2 pPoint = hpolar(dPoint);
    
    float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
    float distance = pPoint.x / maxRadius;
    
    float magic = 0.001;
    center = vec2(camera.x, camera.y) + (1.0 - distance) * vec2(center.x - camera.x, center.y - camera.y) * magic;
    
    dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    pPoint = hpolar(dPoint);
    
    float speed = 2.0;
    float width = 0.5;
    float delay = 0.5 * NORMSIN_HZ_T(1.25);
    
    float valueRed = NORMSIN(pPoint.x / 10 + time * speed);
    float valueBlue = NORMSIN(pPoint.x / 10 + (time - delay) * speed);
    
    return vec4(valueRed, 0.0, valueBlue, 1.0);
}

vec4 hyperpolaritySphereDemo()
{
    float halfWidth = screenWidth / 2;
    float halfHeight = screenHeight / 2;
    vec2 center = vec2(halfWidth, halfHeight);
    vec2 camera = vec2(halfWidth, halfHeight);
    
    vec2 dPoint = vec2(gl_FragCoord.x - center.x, gl_FragCoord.y - center.y);
    vec2 pPoint = polar(dPoint);
    
    float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
    float distance = pPoint.x / maxRadius;
    
    pPoint = hpolar(dPoint);
    pPoint.x *= (1.0 - distance) * (1.0 - distance);
    
    float speed = 8.0;
    float width = 0.5;
    float delay = 0.5 * NORMSIN_HZ_T(1.25);
    
    float valueRed = NORMSIN(pPoint.x / 10 + time * speed);
    float valueBlue = NORMSIN(pPoint.x / 10 + (time - delay) * speed);
    
    return vec4(valueRed, 0.0, valueBlue, 1.0);
}

float softSquare(float phase, float width, float aaEdge)
{
#ifdef ARGUMENTS_CONTROL
    width = clamp(width, 0.0, 1.0);
    aaEdge = clamp(aaEdge, 0.0, 1.0);
#endif
    
    float result = NORMSIN(phase);
    float alias = aaEdge * width;
    
    if (result > (width + alias))
        return 1.0;
    else if (result < (width - alias))
        return 0.0;
    else
        return NORMSIN(M_PI + M_PI * (width - result) / (2 * alias));
}


vec4 acidDemoHz(vec2 fragPoint, vec2 screen, float hz)
{
    vec2 viewPort = vec2(1920.0, 1080.0);
    float iGlobalTime = time;
    vec2 iResolution = screen;
    
    float halfWidth = 0.5 * viewPort.x;
    float halfHeight = 0.5 * viewPort.y;
    vec2 fragmentPoint = fragPoint.xy / iResolution.xy * viewPort;
    
    float phase = 2.0 * M_PI * iGlobalTime;
    float radius = halfHeight;
    
    vec4 sphere;
    {
        vec2 center = vec2(halfWidth, halfHeight);
        
        vec2 dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
        vec2 pPoint = polar(dPoint);
        
        float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        float dist = pPoint.x / maxRadius;
        
        pPoint = hpolar(dPoint);
        pPoint.x *= dist * dist * dist * dist;
        
        float speed = 8.0;
        float width = 0.5;
        float delay = 0.5 * NORMSIN(phase);
        
        float valueRed = NORMSIN(pPoint.x / 10.0 + iGlobalTime * speed);
        float valueBlue = NORMSIN(pPoint.x / 10.0 + (iGlobalTime - delay) * speed);
        float farFading = clamp(pPoint.x / maxRadius, 0.0, 1.0);
        
        sphere = vec4(farFading * valueRed, 0.0, farFading * valueBlue, 0.0);
    }
    
    vec2 center = vec2(halfWidth, halfHeight);
    vec2 camera = vec2(halfWidth + cos(phase * hz / 2) * radius, halfHeight + sin(phase * hz / 2) * radius);
    vec2 dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    vec2 pPoint = polar(dPoint);
    
    float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
    float dist = pPoint.x / maxRadius;
    
    float magic = 0.0625;
    center = center + dist * vec2(center.x - camera.x, center.y - camera.y) * magic;
    
    dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    pPoint = polar(dPoint);
    pPoint.x *= 1.0 - dist;
    
    float speed = 2.0 * hz;
    float width = 0.5;
    float delay = 0.5 * NORMSIN_HZ_T(0.25);
    
    float valueRed = softSquare(pPoint.x / 10.0 + iGlobalTime * speed, width, 0.1);
    float valueBlue = softSquare(pPoint.x / 10.0 + (iGlobalTime - delay) * speed, width, 0.1);
    
    vec4 color = vec4(valueRed, valueBlue, valueBlue, 1.0);
    
    float merge = sqrt( dPoint.x * dPoint.x + dPoint.y * dPoint.y ) / maxRadius;
    merge = 0.75 + 0.25 * merge * NORMSIN(phase + M_PI);
    
    return merge * color + (1.0 - merge) * sphere;
}
vec4 acidDemo(vec2 fragPoint, vec2 screen)
{
    return acidDemoHz(fragPoint, screen, 1.0);
}
vec4 tunnelDemo(vec2 fragmentPoint)
{
    vec2 dCenter = vec2(screenWidth / 2, screenHeight / 2);
    vec2 dPoint = fragmentPoint.xy - dCenter;
    vec2 pPoint = polar(dPoint);
    pPoint.x *= 1.0 - pPoint.x / 900;
    
    float value = NORMSIN(pPoint.x + M_2PI * time * 2 );
    
    return vec4(value, value, value, 1.0);
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

highp float randH(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

float sinlerp(float value)
{
#ifdef ARGUMENTS_CONTROL
    value = clamp(value, 0.0, 1.0);
#endif
    
    return NORMSIN(M_PI + M_PI * value);
}

float sqrtlerp(float value)
{
#ifdef ARGUMENTS_CONTROL
    value = clamp(value, 0.0, 1.0);
#endif
    
    return sqrt(value);
}

float quadlerp(float value)
{
#ifdef ARGUMENTS_CONTROL
    value = clamp(value, 0.0, 1.0);
#endif
    
    return value * value * value * value;
}

float squarelerp(float value)
{
#ifdef ARGUMENTS_CONTROL
    value = clamp(value, 0.0, 1.0);
#endif
    
    return value * value;
}

float scale(float value)
{
    float offset = 0.0021;
    value -= 1.0 - offset;
    value *= 1.0 / offset;
    
    return clamp(value, 0.0, 1.0);
}

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec2 scaleImageNORM(float scale)
{
    return floor(gl_FragCoord.xy / scale);
}

vec2 scaleImage(float scale)
{
    return floor( gl_FragCoord.xy / scale / vec2(screenWidth, screenHeight));
}

vec4 pixelCarpetDemo()
{
    vec2 point = scaleImage(12);
    float hzx = point.x * M_PI * 2;
    float hzy = point.y * M_PI * 2;
    
    hzx = rand(vec2(hzx, 0));
    hzy = rand(vec2(0, hzy));
    
    return vec4(NORMSIN_HZ_T(hzx), NORMSIN_HZ_T(hzx + hzy), NORMSIN_HZ_T(hzy), 1.0);
}

vec2 rotatePoint(vec2 point, float angle)
{
    return vec2(point.x * cos(angle) - point.y * sin(angle), point.x * sin(angle) + point.y * cos(angle));
}

vec4 coilDemo()
{
    float value = NORMSIN(M_2PI * time * 2.0 * NORMSIN_HZ_T(0.5));
    return vec4(value, value, value, 1.0) - acidDemo(scaleImageNORM(1.0 + abs(externalValue / 8)), vec2(screenWidth, screenHeight) / (1.0 + abs(externalValue / 8)));
}

vec2 norm(vec2 point)
{
    float l = sqrt(point.x * point.x + point.y * point.y);
    return vec2(point.x / l, point.y / l);
}

float mixValues(float a, float b, float dry)
{
    return (dry * a + (1.0 - dry) * b) / 2;
}

float valueFromBounds(float normValue, float bot, float top)
{
#ifdef ARGUMENTS_CONTROL
    normValue = clamp(normValue, 0.0, 1.0);
    if (bot > top)
    {
        bot = bot + top;
        top = bot - top;
        bot = bot - top;
    }
#endif
    return bot + (top - bot) * normValue;
}

vec4 trackBPMDemo(float demoTimeSec, float bpm)
{
    const float fadeFromWhiteTime = 8.0;
    const float bitcrashFadeInTime = 2.0;
    
    float mainLFO = NORMSIN_HZ_T_PH(1.0 / demoTimeSec, -M_PI_2);                      // 0.00151515 is good
    float hzBpm = bpm / 60.0;
    float bitcrashHz = hzBpm / 4;
    float noiseHz = hzBpm / 32;
    float shiverHz = hzBpm / 16;
    
    float shiver = NORMSIN(M_2PI * time * shiverHz * sin(M_2PI * time * shiverHz));
    //shiver = NORMSIN_HZ_T(floor(time));
    
    float bitcrashLowTreshold = 1.0;
    float bitcrashHightTreshold = 100;
    float bitcrashRythm = NORMSIN_HZ_T_PH(bitcrashHz * clamp(-1.0 + time / bitcrashFadeInTime, 0.0, 1.0), -M_PI_2);
    float acidDemoBitcrash = (0.125 * mainLFO * bitcrashHightTreshold + bitcrashLowTreshold) + (bitcrashHightTreshold - bitcrashLowTreshold) * (mainLFO * bitcrashRythm * shiver);
    
    float noiseFadeFromWhitePower = 3.0;
    float noiseLowTreshold = 1.0;
    float noiseHightTreshold = 20.0;
    float fading = clamp(-1.0 + time / fadeFromWhiteTime, 0.0, 1.0);
    float noiseRythm = mixValues(shiver, NORMSIN_HZ_T_PH(noiseHz, M_PI_2) * mainLFO * fading, 0.25);//NORMSIN_HZ_T_PH(noiseHz, M_PI_2) * fading * mainLFO;
    float noisePower = valueFromBounds(noiseRythm, noiseLowTreshold, noiseHightTreshold) - noiseFadeFromWhitePower * (1.0 - fading);
    //noiseLowTreshold + (noiseHightTreshold - noiseLowTreshold) * mixValues(noiseRythm, shiver, 0.75) - noiseLowTreshold * (1.0 - fading);

    vec2 point = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    vec3 value = vec3(0.0, 0.0, 0.0);
    float N = pow(2, 4.0 + floor(4.0 * shiver)); //rand(vec2(sin(time), sin(time)))));
    for (float i = N; i > 1; i /= 2)
    {
        value += rand(scaleImageNORM(i) * sin(time)) * (hsv2rgb(vec3(NORMSIN_HZ_T(0.25), 0.15, 0.75 + 0.25 * (i / log(N)))));
    }
    value /= log(N) + noisePower;

    float mergingFactor = 0.5;
    //return vec4(value, 1.0);// * (0.75 + (0.25 * (1.0 - mainLFO)));
    return (0.75 + (0.25 * (1.0 - mainLFO)) + mergingFactor) * acidDemoHz(scaleImageNORM(acidDemoBitcrash), vec2(screenWidth, screenHeight) / acidDemoBitcrash, hzBpm / 2) - mergingFactor * vec4(value, 1.0);
}

vec4 acidDemoJoystick(vec2 fragPoint, vec2 screen)
{
    vec2 viewPort = vec2(1920.0, 1080.0);// * (1.0 + 9.0 * NORM(rightJoystick.y));
    float iGlobalTime = time;
    vec2 iResolution = screen;
    
    float halfWidth = 0.5 * viewPort.x;
    float halfHeight = 0.5 * viewPort.y;
    vec2 fragmentPoint = fragPoint.xy / iResolution.xy * viewPort;
    
    float phase = 2.0 * M_PI * iGlobalTime;
    float radius = halfHeight;
    
    vec4 sphere;
    {
        vec2 center = vec2(halfWidth, halfHeight);
        
        vec2 dPoint = fragmentPoint - center;
        vec2 pPoint = polar(dPoint);
        
        float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        float dist = pPoint.x / maxRadius;
        
        pPoint = hpolar(dPoint);
        pPoint.x *= dist * dist * dist * dist;
        
        float speed = 8.0;
        float width = 0.5;
        float delay = 0.5 * NORMSIN(phase);
        
        float valueRed = NORMSIN(pPoint.x / 10.0 + iGlobalTime * speed);
        float valueBlue = NORMSIN(pPoint.x / 10.0 + (iGlobalTime - delay) * speed);
        float farFading = clamp(pPoint.x / maxRadius, 0.0, 1.0);
        
        sphere = vec4(farFading * valueRed, 0.0, farFading * valueBlue, 0.0);
    }
    
    vec2 center = vec2(halfWidth, halfHeight);
    float centerControllPower = 8;
    vec2 camera = vec2(NORM(leftJoystick.x) * viewPort.y * centerControllPower, NORM(leftJoystick.y) * viewPort.y * centerControllPower);
    vec2 dPoint = fragmentPoint - center;
    vec2 pPoint = polar(dPoint);
    
    float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
    float dist = pPoint.x / maxRadius;
    
    float magic = 0.0625;
    center = center + dist * (center.x - camera) * magic;
    
    dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    pPoint = polar(dPoint);
    pPoint.x *= 1.0 - dist;
    
    float speed = 2.0;
    float width = 0.5;
    float delay = NORMSIN_HZ_T(1.0 / 145.0) * 0.5;// + 0.45 * NORM(rightJoystick.y);
    
    float valueRed = softSquare(pPoint.x / 10.0 + iGlobalTime * speed, width, 0.1);
    float valueBlue = softSquare(pPoint.x / 10.0 + (iGlobalTime - delay) * speed, width, 0.1);
    
    vec4 color = vec4(valueRed, valueBlue, valueBlue, 1.0);
    
    float merge = sqrt( dPoint.x * dPoint.x + dPoint.y * dPoint.y ) / maxRadius;
    merge = 0.75 + 0.25 * merge * NORMSIN(phase + M_PI);
    
    return merge * color + (1.0 - merge) * sphere;
}

vec4 joystickDemo(float demoTimeSec, float bpm)
{
    const float fadeFromWhiteTime = 8.0;
    const float bitcrashFadeInTime = 2.0;
    
    float mainLFO = NORMSIN_HZ_T_PH(1.0 / demoTimeSec, -M_PI_2);                      // 0.00151515 is good
    float hzBpm = bpm / 60.0;
    float bitcrashHz = hzBpm / 4;
    float noiseHz = hzBpm / 32;
    float shiverHz = hzBpm / 16;
    
    float shiver = NORMSIN(M_2PI * time * shiverHz * sin(M_2PI * time * shiverHz));
    //shiver = NORMSIN_HZ_T(floor(time));
    
    float bitcrashLowTreshold = 1.0;
    float bitcrashHightTreshold = 40;
    float bitcrashRythm = NORMSIN_HZ_T_PH(bitcrashHz * clamp(-1.0 + time / bitcrashFadeInTime, 0.0, 1.0), -M_PI_2);
    float acidDemoBitcrash = valueFromBounds(NORM(-rightTrigger), bitcrashLowTreshold, bitcrashHightTreshold);
    
    float noiseFadeFromWhitePower = 3.0;
    float noiseLowTreshold = 1.0;
    float noiseHightTreshold = 20.0;
    float fading = 1.0;//clamp(-1.0 + time / fadeFromWhiteTime, 0.0, 1.0);
    float noiseRythm = mixValues(shiver, NORMSIN_HZ_T_PH(noiseHz, M_PI_2) * mainLFO * fading, 0.25);//NORMSIN_HZ_T_PH(noiseHz, M_PI_2) * fading * mainLFO;
    float noisePower = valueFromBounds(noiseRythm, noiseLowTreshold, noiseHightTreshold) - noiseFadeFromWhitePower * (1.0 - fading);
    //noiseLowTreshold + (noiseHightTreshold - noiseLowTreshold) * mixValues(noiseRythm, shiver, 0.75) - noiseLowTreshold * (1.0 - fading);
    
    vec2 point = gl_FragCoord.xy / vec2(screenWidth, screenHeight);
    vec3 value = vec3(0.0, 0.0, 0.0);
    float N = pow(2, 4.0 + 3.0 * NORM(-leftTrigger));//+ floor(4.0 * shiver)); //rand(vec2(sin(time), sin(time)))));
    for (float i = N; i > 1; i /= 2)
    {
       // value += rand(scaleImageNORM(point + norm(vec2(rand(vec2(time, -time)))), i) * sin(time)) * (hsv2rgb(vec3(NORMSIN_HZ_T(0.25), 0.15, 0.75 + 0.25 * (i / log(N)))));
    
        vec2 viewPort = vec2(screenWidth, screenHeight);
        vec2 center = viewPort / 2;
        vec2 truePoint = (gl_FragCoord.xy - center);
        value += rand(floor((truePoint + norm(vec2(rand(vec2(time, -time))))) / i) * sin(time)) * (hsv2rgb(vec3(NORMSIN_HZ_T(0.25), 0.15, 0.75 + 0.25 * (i / log(N))))) * length(truePoint) / 200;
    }
    value /= log(N) + noisePower;
    
    float mergingFactor = 0.5;
    //return vec4(value, 1.0);
    return (0.75 + (0.35 * (shiver)) + mergingFactor) * acidDemoJoystick(scaleImageNORM(acidDemoBitcrash), floor(vec2(screenWidth, screenHeight) / acidDemoBitcrash)) - mergingFactor * vec4(value, 1.0);
}


vec4 acidDemoJoystickTrackBPM(vec2 fragPoint, vec2 screen, float hz)
{
    vec2 viewPort = vec2(1920.0, 1080.0) * (1.0 + quadlerp(NORM(leftTrigger)) * 4);
    float iGlobalTime = time;
    vec2 iResolution = screen;
    
    float halfWidth = 0.5 * viewPort.x;
    float halfHeight = 0.5 * viewPort.y;
    vec2 fragmentPoint = fragPoint.xy / iResolution.xy * viewPort;
    
    float phase = 2.0 * M_PI * iGlobalTime;
    float radius = halfHeight;
    
    vec4 sphere;
    {
        vec2 center = vec2(halfWidth, halfHeight);
        
        vec2 dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
        vec2 pPoint = polar(dPoint);
        
        float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
        float dist = pPoint.x / maxRadius;
        
        pPoint = hpolar(dPoint);
        pPoint.x *= dist * dist * dist * dist;
        
        float speed = 8.0;
        float width = 0.5;
        float delay = 0.5 * NORMSIN(phase);
        
        float valueRed = NORMSIN(pPoint.x / 10.0 + iGlobalTime * speed);
        float valueBlue = NORMSIN(pPoint.x / 10.0 + (iGlobalTime - delay) * speed);
        float farFading = clamp(pPoint.x / maxRadius, 0.0, 1.0);
        
        sphere = vec4(farFading * valueRed, 0.0, farFading * valueBlue, 0.0);
    }
    
    vec2 center = vec2(halfWidth, halfHeight);
    vec2 camera = vec2(halfWidth + cos(phase * hz / 2) * radius * leftJoystick.x, halfHeight + sin(phase * hz / 2) * radius * leftJoystick.y);
    vec2 dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    vec2 pPoint = polar(dPoint);
    
    float maxRadius = sqrt(halfWidth * halfWidth + halfHeight * halfHeight);
    float dist = pPoint.x / maxRadius;
    
    float magic = 0.0625 + 0.0625 * NORM(leftTrigger) * 32;
    center = center + dist * vec2(center.x - camera.x, center.y - camera.y) * magic;
    
    dPoint = vec2(fragmentPoint.x - center.x, fragmentPoint.y - center.y);
    pPoint = polar(dPoint);
    pPoint.x *= 1.0 - dist;
    
    float speed = 2.0 * hz;
    float width = 0.5;
    float delay = 0.5 * NORMSIN_HZ_T(0.25);
    
    float valueRed = softSquare(pPoint.x / 10.0 + iGlobalTime * speed, width, 0.1);
    float valueBlue = softSquare(pPoint.x / 10.0 + (iGlobalTime - delay) * speed, width, 0.1);
    
    vec4 color = vec4(valueRed, valueBlue, valueBlue, 1.0);
    
    float merge = sqrt( dPoint.x * dPoint.x + dPoint.y * dPoint.y ) / maxRadius;
    merge = 0.75 + 0.25 * merge * NORMSIN(phase + M_PI);
    
    return merge * color + (1.0 - merge) * sphere;
}

vec4 joystickControlsTest()
{
    return (drawCircle(vec2(screenWidth / 4, screenHeight / 2) + vec2(leftJoystick.x * screenWidth / 4, leftJoystick.y * screenWidth / 4), 20.0) + drawCircle(vec2(3 * screenWidth / 4, screenHeight / 2) + vec2(rightJoystick.x * screenWidth / 4, rightJoystick.y * screenWidth / 4), 20.0)) * vec4(1.0, 1.0, 1.0, 1.0);
}

vec4 joystickTrackBPMDemo(float demoTimeSec, float bpm)
{
    const float fadeFromWhiteTime = 8.0;
    const float bitcrashFadeInTime = 2.0;
    
    float mainLFO = NORMSIN_HZ_T_PH(1.0 / demoTimeSec * (-rightJoystick.y), M_2PI);                      // 0.00151515 is good
    float hzBpm = bpm / 60.0;
    float bitcrashHz = hzBpm / 4;
    float noiseHz = hzBpm / 32;
    float shiverHz = hzBpm / 16;
    
    float shiver = sin(M_2PI * time * shiverHz * sin(M_2PI * time * shiverHz));
    float antiShiver = shiver + rightJoystick.x;
    shiver = NORM(shiver);
    //shiver = NORMSIN_HZ_T(floor(time));
    
    float bitcrashLowTreshold = 1.0;
    float bitcrashHightTreshold = 100;
    float bitcrashRythm = NORMSIN_HZ_T_PH(bitcrashHz * clamp(-1.0 + time / bitcrashFadeInTime, 0.0, 1.0), -M_PI_2);
    float acidDemoBitcrash = (0.125 * mainLFO * bitcrashHightTreshold + bitcrashLowTreshold) + (bitcrashHightTreshold - bitcrashLowTreshold) * (mainLFO * bitcrashRythm * NORM(antiShiver) );
    
    float noiseFadeFromWhitePower = 3.0;
    float noiseLowTreshold = 1.0;
    float noiseHightTreshold = 20.0;
    float fading = sin(M_2PI * time * shiverHz * leftTrigger);//clamp(-1.0 + time / fadeFromWhiteTime, 0.0, 1.0);
    float noiseRythm = mixValues(shiver, NORMSIN_HZ_T_PH(noiseHz, M_PI_2) * mainLFO * fading, 0.25);//NORMSIN_HZ_T_PH(noiseHz, M_PI_2) * fading * mainLFO;
    float noisePower = valueFromBounds(noiseRythm, noiseLowTreshold, noiseHightTreshold) - noiseFadeFromWhitePower * (1.0 - fading);
    //noiseLowTreshold + (noiseHightTreshold - noiseLowTreshold) * mixValues(noiseRythm, shiver, 0.75) - noiseLowTreshold * (1.0 - fading);
    
    vec3 value = vec3(0.0, 0.0, 0.0);
    float N = pow(2, 4.0 + floor(4.0 * NORM(rightJoystick.y))); //rand(vec2(sin(time), sin(time)))));
    for (float i = N; i > 1; i /= 2)
    {
        value += rand(scaleImageNORM(i) * sin(time)) * (hsv2rgb(vec3(NORMSIN_HZ_T(0.25), 0.15, 0.75 + 0.25 * (i / log(N)))));
    }
    value /= log(N) + noisePower;
    
    float mergingFactor = 0.5;
    //return vec4(value, 1.0);// * (0.75 + (0.25 * (1.0 - mainLFO)));
    return (0.75 + (0.25 * (1.0 - mainLFO)) + mergingFactor) * acidDemoJoystickTrackBPM(scaleImageNORM(acidDemoBitcrash), vec2(screenWidth, screenHeight) / acidDemoBitcrash, hzBpm / 2) - mergingFactor * vec4(value, 1.0);
}

vec4 metronome(float bpm, float width)
{
    float value = NORMSQUARE_HZ_T_PH(bpm / 60.0, 0 * M_PI_2, 0.5) * clamp(width - gl_FragCoord.x, 0.0, 1.0) * clamp(width - gl_FragCoord.y, 0.0, 1.0);
    return vec4(value, value, value, 1.0);
}

float tchn(float tchnTime, float tchnBandPass)
{
    float discNoise = tchnBandPass * rand(vec2(floor(tchnTime), 1));
    return floor(fract(tchnTime) + discNoise);
}

float tchn2(float tchnTime, float tchnBandPass)
{
    float discNoise = tchnBandPass * rand(vec2(floor(tchnTime), 2));
    return fract(tchnTime) + discNoise;
}

float optLen(vec2 point)
{
    return point.x * point.x + point.y * point.y;
}

vec4 universe(vec2 frag)
{
    const float disp = M_2PI;
    const float bandPass = 720.0;
    
    const float N = 50;
    float presence = 0.0;
    float test_presence = 0.0;
    float timeDelta = bandPass;
    float particleLifetime = 4.0;
    float globTime = time / particleLifetime;
    
    vec2 screen = vec2(screenWidth, screenHeight);
    vec2 center = screen / 2;
    
    float fov = M_PI_2 / 2;
    float d = 1.0 / tan(fov / 2);
    float a = screen.x / screen.y;
    mat4 Mproj;
    Mproj[0][0] = 1.0;//d / a;
    Mproj[1][1] = 1.0;//d;
    Mproj[2][2] = 1.0;//-1;
    Mproj[3][3] = 1.0;//-1;
    
    float fur = M_PI_2 * 0.9;
    
    for (float i = 0; i < N; i += 1.0)
    {
        float phase = ((2.0 * i / N) - 1.0);
        float time = globTime + timeDelta * phase;
        
        // [0; 1] saw
        float particleTime = fract(time + 0.5 * phase);
        float particleStatX = floor(tchn(time + 0.5 * phase, bandPass)) / (bandPass + 1.0);
        float particleStatY = floor(tchn2(time + 0.5 * phase, bandPass)) / (bandPass + 1.0);
        
        vec3 star;
        /*
        float rad = 0.95;
        star.x = particleStatX * 2.0 * rad - rad;
        star.x += (1.0 - rad) * sign(star.x);
        star.y = particleStatY * 2.0 * rad - rad;
        star.y += (1.0 - rad) * sign(star.y);*/
        vec2 pPoint;
        float cutoff = 0.15;
        pPoint.x = cutoff + (particleStatX * (1.0 - cutoff));
        pPoint.y = particleStatY * M_2PI;
        
        star.xy = decart(pPoint);
        star.z = particleTime;
        
        /*vec4 persV = Mproj * vec4(star, 1.0);
        vec2 dPoint = persV.xy * screen;*/
        vec2 dPoint = star.xy + vec2(tan(fur * star.x), tan(fur * star.y)) * star.z;
        dPoint.x = NORM(dPoint.x);
        dPoint.y = NORM(dPoint.y);
        dPoint *= screen;
        
        vec2 frag = frag.xy;
        
        
        float normX = pow(particleTime, 2);
        float particleSize = 1.0;
        //particleSize *= particleTime;
        particleSize = 0.5 * tan(fur * particleSize) * star.z;
        
        //particleSize = 2.0;
        float localPresence = particleSize - clamp(length(dPoint - frag), 0.0, particleSize);
        presence += localPresence;
        
        /*
        vec2 pPoint;
        float normX = pow(particleTime, 2);
        pPoint.x = length(center) * normX + 10;
        pPoint.y = floor(tchn(time + 0.5 * phase, bandPass)) / (bandPass + 1.0) * disp;
        
        vec2 dPoint = decart(pPoint);
        vec2 frag = frag.xy - center;
        
        float particleSize = 4.0;
        //particleSize *= particleTime;
        particleSize *= normX;
        //particleSize = 1.0;
        float localPresence = particleSize - clamp(length(dPoint - frag), 0.0, particleSize);
        presence += localPresence;*/
    }
    presence = clamp(presence, 0.0, 1.0);
    float alpha = 1.0;
    return vec4(0.9 * presence, 0.9 * presence, presence, alpha);
}

vec4 noiseSpiral()
{
    vec2 screen = vec2(screenWidth, screenHeight);
    vec2 center = screen / 2;
    vec2 frag = (gl_FragCoord.xy - center);
    vec2 pPoint = polar(frag);
    
    float a = 10.0;
    float b = M_2PI;
    
    float width = 1.0;
    float presence = abs(pPoint.x - a - b * pPoint.y) < width ? 1.0 : 0.0;
    
    return vec4(presence, presence, presence, 1.0);
}

vec3 redColor()
{
    return vec3(1.0, 0.0, 0.0);
}
vec3 yellowColor()
{
    return vec3(1.0, 1.0, 0.0);
}
vec3 cyanColor()
{
    return vec3(0.0, 1.0, 1.0);
}
vec3 whiteColor()
{
    return vec3(1.0, 1.0, 1.0);
}

vec4 test()
{
    vec3 color = whiteColor();
    
    vec2 screen = vec2(screenWidth, screenHeight);
    vec2 center = screen / 2;
    
    vec2 fragPoint = gl_FragCoord.xy / screen;
    vec2 pPoint = polar(fragPoint);
    
    float cutoff = 0.999;
    
    float presence;
    presence = rand(fragPoint + vec2(externalValue / screenWidth, 0));
    
    presence = clamp(presence - cutoff, 0.0, 1.0);
    presence *= 1.0 / (1.0 - cutoff); // restore brightness after clamp
    presence *= NORMSIN(M_2PI * time * presence * 0.25 + M_2PI * presence);
    return vec4(color * presence, 1.0);
        
    //return vec4(presence > 0.25 ? 1.0 : 0.0, presence > 0.5 ? 1.0 : 0.0, presence > 0.75 ? 1.0 : 0.0, 1.0);
    //return vec4(presence, presence, presence, 1.0);
}
























vec4 universe_polar(float t)
{
    vec2 screen = vec2(screenWidth, screenHeight);
    vec2 center = screen / 2;
    vec2 frag = gl_FragCoord.xy - center;
    vec2 fragPolar = polar(frag);
    float lenCenter = length(center);
    
    const float bandPass = 720.0;
    const float angleDisp = M_2PI / (bandPass + 1.0);
    
    const float particlesCount = 200.0;
    const float particleLifetime = 10.0;
    const float particleMaxSize = 7.5;
    float particleMaxSizeNorm = particleMaxSize / lenCenter;
    
    float globTime = time / particleLifetime;
    float r_fractGlobTime = 1.0 - fract(globTime);
    float timeDelta = bandPass;
    
    const float polarRadiusClip = 0.05;
    const float polarRadiusMax = 0.75;
    float polarRadiusDelta = polarRadiusMax - polarRadiusClip;
    
    float presence = 0.0;
    vec2 pPoint;

    for (float i = 0.0; i < particlesCount; i += 1.0)
    {
        float phase = i / particlesCount;
        
        float localTime = globTime + timeDelta * (2.0 * phase - 1.0) + phase;
        float particleTime = fract(localTime);
        if (particleTime < 0.5) continue;
        float spaceTransform = pow(particleTime, 8.0);
        
        pPoint.x = lenCenter * ((polarRadiusClip + polarRadiusDelta * phase) + spaceTransform);
        
        // +30 FPS :)
        if (abs(pPoint.x - fragPolar.x) > particleMaxSize) continue;
        
        pPoint.y = floor(particleTime + bandPass * rand(vec2(floor(localTime), 1))) * angleDisp;
        
        vec2 dPoint = decart(pPoint);
        float particleSize = particleMaxSize * spaceTransform;
        float localPresence = particleSize * (1.0 - clamp(length(dPoint - frag), 0.0, 1.0));
        presence += localPresence;
    }
    presence = clamp(presence, 0.0, 1.0);
    return vec4(presence, presence, presence, 1.0);
}

vec4 heart()
{
    vec2 viewPort = vec2(screenWidth, screenHeight);
    vec2 glFragcoord = gl_FragCoord.xy;
    
    vec2 screen = vec2(viewPort.x, viewPort.y);
    vec2 center = screen / 2.0;
    float lenCenter = length(center);
    vec2 frag = (glFragcoord - center);
    vec2 fragPolar = polar(frag);
    
    float controlValue = rand(vec2(time, fragPolar.y));
    
    // first was ... first
    float first = center.y;
    // second was small-gold of first
    float circleRadius = first / GOLD;
    
    const int maxCircles = 40;
    // third was small-gold of first
    float allLinesWidth = first - circleRadius;
    float linesWidth = allLinesWidth / maxCircles;//100.0 + linesWidth / 2.0;
    // fourth was big-gold of third(small)
    float squareWidth = allLinesWidth / GOLD;
    // fifth was small-gold of third(small)
    float centerZoneRadius = allLinesWidth - squareWidth;

    
    float heartRadiusNoised = centerZoneRadius + linesWidth * floor(controlValue * maxCircles + 0.5) - linesWidth / 2.0;
    float heartRadius = centerZoneRadius + linesWidth * maxCircles - linesWidth / 2.0;

    float centerClip = clamp(floor(fragPolar.x - centerZoneRadius), 0.0, 1.0);
    float outerEdgeClip = clamp(floor(heartRadiusNoised - fragPolar.x), 0.0, 1.0);
    float circles = centerClip * softSquare(M_2PI * fragPolar.x / linesWidth, 0.75 + (0.025) * sin(M_2PI * time * 1.0 + fragPolar.y), 0.75) * outerEdgeClip;
    circles *= 0.25 + 0.75 * controlValue;
    
    const float sqrt2_2 = sqrt(2.0) / 2.0;
    //float squareWidth = heartRadius * sqrt2_2 ;
    float squareClipWidth = squareWidth - linesWidth * sqrt2_2;
    
    float rotationAngle = M_PI / 4;
    mat2 rotation = mat2(cos(rotationAngle), -sin(rotationAngle),
                         sin(rotationAngle), cos(rotationAngle) );
    vec2 squareFrag = frag * rotation;
                         
    float square = (clamp((squareWidth - abs(squareFrag.x)), 0.0, 1.0)) * (clamp((squareWidth - abs(squareFrag.y)), 0.0, 1.0));
    square -= (clamp((squareClipWidth - abs(squareFrag.x)), 0.0, 1.0)) * (clamp((squareClipWidth - abs(squareFrag.y)), 0.0, 1.0));
    
    return vec4((circles - square) * whiteColor(), 1.0);
}

vec4 fractalNoise()
{
    const int maxIterations = 1;
    vec2 screen = vec2(screenWidth, screenHeight);
    vec2 center = screen / 2;
    float lenCenter = screenHeight;
    vec2 frag = gl_FragCoord.xy - center;
    
    vec3 cellColor = vec3(0.0, 0.0, 0.0);
    
    float iter = 1.0;
    do
    {
        vec2 cellFrag = floor(frag / (lenCenter / (2 * iter)));
        cellColor += whiteColor() * rand(vec2(cellFrag));
    }
    while (iter <= maxIterations);
    
    return vec4(cellColor, 1.0);
}

float circlePresence(vec2 fragPolar, float radius, float lineWidth)
{
    return lineWidth - clamp(abs(fragPolar.x - radius), 0.0, lineWidth);
}

vec4 goldenCircles()
{
    vec2 screen = vec2(screenWidth, screenHeight);
    vec2 center = screen / 2;
    
    vec2 cameraMove = (externalValue * 5.0) * ((vec2(mouseX, mouseY) - center) / screen);//vec2(cos(M_2PI * time * 0.5), sin(M_2PI * time * 0.5));
    //center += cameraMove;

    float lenCenter = length(center);
    vec2 frag = gl_FragCoord.xy - center;
    vec2 fragPolar = polar(frag);
    
    float lineWidth = 5.0;
    float maxIterations = 11.0;//floor(externalValue);
    float circles = 0;
    
    float fractTime = fract(time);
    
    for (float i = 0; i <= maxIterations; i += 1.0)
    {
        float phase = (1.0 - (i - fractTime) / maxIterations);
        vec2 polar = polar(gl_FragCoord.xy - (center + cameraMove * phase));
        circles += phase * circlePresence(polar, lenCenter / pow(GOLD, i - fractTime), 1.0 + lineWidth / pow(GOLD, i - fractTime));
    }
    /*
    float first = lenCenter * fract(time);
    float second = lenCenter / GOLD * fract(time);
    float third = lenCenter / GOLD / GOLD * fract(time);
    float lineWidth = 2.0;
    
    float circles = circlePresence(fragPolar, first, lineWidth) + circlePresence(fragPolar, second, lineWidth) + circlePresence(fragPolar, third, lineWidth);*/
    //float circles = floor(1.0 - clamp(abs(fragPolar.x - first), 0.0, 1.0)) + floor(1.0 - clamp(abs(fragPolar.x - second), 0.0, 1.0)) + floor(1.0 - clamp(abs(fragPolar.x - second), 0.0, 1.0));
    
    return vec4(whiteColor() * circles, 1.0);
}

void main()
{
    color = vec4(0.4, 0.1, 1.0, 1.0);
    //color = goldenCircles();
    //color = heart();
    //color = fractalNoise();
    
    //float externalValue = (-time * 8) * NORMSIN_HZ_T(floor(time) * 8);

    //color = acidDemoHz(gl_FragCoord.xy, vec2(screenWidth, screenHeight), externalValue);
    
    //color = noiseSpiral();
   // if (gl_FragCoord.x < screenWidth / 2)
//        color = universe(floor(gl_FragCoord.xy));
//    else
    /*
    const float n = 10.0;
    for (float i = 0; i < n; i += 1.0)
    {
        color += (i / n) * universe_polar(time + 0.5 * i / n);
    }*/
    //color = universe_polar(time);
    //color += drawFPS() * clamp(externalValue, 0.0, 1.0);
    //color = vec4(yellowColor(), 1.0);
    //color = noiseSpiral();

    
    //color += 0.5 * test();
    
    //float bpm = 145;
    //float time = (12 + 2 / 3) * 15;
    //color = trackBPMDemo(60 * 5, bpm);// * (1.0 - metronome(60, screenWidth / 4));
    //color = joystickDemo((12 + 2 / 3) * 15, bpm);
    //color = joystickControlsTest();
    
    //color = joystickTrackBPMDemo((12 + 2 / 3) * 15, bpm);
    
    
   // color = vec4(NORM(rightJoystick.x), 0, NORM(rightJoystick.y), 1.0);
    //color = coilDemo();
    //color = pixelCarpetDemo();
    //color = hyperpolarityDemo(gl_FragCoord.xy);
    //color = hyperpolaritySphereDemo(gl_FragCoord.xy);
    //color = acidDemo(gl_FragCoord.xy, vec2(screenWidth, screenHeight));
    //color = acidDemo(scaleImageNORM(gl_FragCoord.xy, (externalValue / 8)), vec2(screenWidth, screenHeight) / (externalValue / 8));
    //color = tunnelDemo(gl_FragCoord.xy);
}
