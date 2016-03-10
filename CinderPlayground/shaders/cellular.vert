#version 330 core
#define DATA_TYPE vec4

uniform samplerBuffer cellsSampler;
uniform isamplerBuffer rulesSampler;

uniform ivec2 GridSize;

uniform int ruleRadius;
uniform int cycleN;
uniform float cycleStep;

uniform float time;

in ivec2 inPosition;

out DATA_TYPE outCellState;

vec4 getFullCellState(ivec2 pos)
{
    return texelFetch(cellsSampler, int(mod(pos.x, GridSize.x)) * GridSize.y + int(mod(pos.y, GridSize.y)));
}

float getCellState(ivec2 pos)
{
    return getFullCellState(pos).r;
}



bool checkNeumann(int i, int j)
{
    return !(((i == 0 && j == 0) || (i != 0 && j != 0)));
}

bool checkMoore(int i, int j)
{
    return !(i == 0 && j == 0);
}

float gameOfLife()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkMoore(i, j))
                continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j)) * clamp(float(i * i + j * j), 0.0, 1.0);
        }
    }
    
    if (state > 0)
    {
        if (neighborsSum >= 2 && neighborsSum <= 3)
            nextState = state;
        else
            nextState = 0.0;
    }
    else
    {
        if (neighborsSum == 3)
            nextState = 1.0;
    }
    return nextState;
}

float cyclicCA()
{
    float state = getCellState(inPosition);
    float nextState = mod(state + cycleStep, 1.0);
    
    int neighborsSum = 0;
    int treshold = 2;
     
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkMoore(i, j))
                continue;
            
            if (abs(getCellState(inPosition + ivec2(i, j)) - nextState) < cycleStep)
                neighborsSum++;
        }
    }
    
    if (neighborsSum >= treshold)
    {
        return nextState;
    }
    else
        return state;
}

float freeCA()
{
    return 0.0;
}

float oneOut()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkMoore(i, j))
                continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j)) * clamp(float(i * i + j * j), 0.0, 1.0);
        }
    }
    
    if (neighborsSum == 0)
        return state;
    
    float md = mod(neighborsSum, 3.0);
    if (md == 0.0)
    {
        nextState = 1.0;
    }
    else
    {
       // nextState = 0.0;
    }
    
    return nextState;
}

float lichens()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkMoore(i, j))
                continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j)) * clamp(float(i * i + j * j), 0.0, 1.0);
        }
    }
    
    if (neighborsSum == 3.0 || (neighborsSum >= 7 && neighborsSum <= 8))
    {
        nextState = 1.0;
    }
    else if (neighborsSum == 4.0)
        nextState = 0.0;
    
    return nextState;
}

float majority()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            //if (!checkMoore(i, j))
              //  continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j));
        }
    }
    
    if (neighborsSum >= 6.0 || neighborsSum == 4.0)
    {
        nextState = 1.0;
    }
    else
        nextState = 0.0;
    
    return nextState;
}

float anneal()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            //if (!checkMoore(i, j))
            //  continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j));
        }
    }
    
    if (neighborsSum >= 6.0 || neighborsSum == 4.0)
    {
        nextState = 1.0;
    }
    else
        nextState = 0.0;
    
    return nextState;
}

float banks()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkNeumann(i, j))
                continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j));
        }
    }

    if (neighborsSum == 2.0 && getCellState(inPosition + ivec2(0, -1)) != getCellState(inPosition + ivec2(0, 1)))
    {
        nextState = 0.0;
    }
    else if (neighborsSum > 2.0)
        nextState = 1.0;
    
    return nextState;
}

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float randFreq()
{
    float min = 20.0;
    float max = 22000.0;
    return pow(2.0, (log2(min) + (log2(max) - log2(min)) * rand(vec2(fract(time), 1.0))));
}

vec4 harm()
{
    float Sb = 0.0;
    
    vec4 state = getFullCellState(inPosition);
    float amp = state.x;
    float nextFreq = state.y;
    float R = -1.0;
    
    float K = 1.0;
    float r = 0.2125;
    float dR = 1.0;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkMoore(i, j))
                continue;
            
            vec4 broState = getFullCellState(inPosition + ivec2(i, j));
            
            float diff = max(state.y, broState.y) / min(state.y, broState.y);
            float B = floor(K * diff + r) - floor(K * diff - r);
            
            Sb += B;
        }
    }
    
    const float birthRange1 = 2.0;
    const float birthRange2 = 3.5;
    const float keepRange1 = 1.0;
    const float keepRange2 = 2.2;
    
    if (amp == 0.0)
    {
        if (Sb >= birthRange1 && Sb <= birthRange2)
        {
            R = 1.0;
            nextFreq = randFreq();
        }
    }
    else
    {
        if (Sb >= keepRange1 && Sb <= keepRange2)
        {
            R = 0.0;
        }
    }
    
    return vec4(clamp(amp + dR * R, 0.0, 1.0), nextFreq, 0.0, 1.0);
}

ivec4 getRule(int index)
{
    return texelFetch(rulesSampler, index);
}

vec4 customRules()
{
    int ruleAdress = 0;
    int count = 0;
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            vec4 cellState = getFullCellState(inPosition + ivec2(i, j));
            if (cellState.x > 0)
                ruleAdress = ruleAdress | (1 << count);
            
            count++;
        }
    }
    
    float nextState = float(getRule(ruleAdress).x);
    //float nextState = float(getRule(int(mod(inPosition.x * GridSize.y + inPosition.y, 512))).x);
    
    return vec4(nextState, float(ruleAdress) / 512.0, 0.0, 0.0);
}

vec4 contValues()
{
    float neighborsSum = 0;
    float state = getCellState(inPosition);
    
    int ruleNCount = (2 * ruleRadius + 1); ruleNCount *= ruleNCount; ruleNCount -= 1;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            if (!checkMoore(i, j))
                continue;
            
            neighborsSum += getCellState(inPosition + ivec2(i, j));
        }
    }
    /*
    float nextState = 0.0;
    if (state < 0.5 && neighborsSum >= 3.0 && neighborsSum < 4.0)
    {
        nextState = 1.0 - state;
    }
    else if (state > 0.5 && neighborsSum >= 2.0 && neighborsSum < 4.0)
    {
        nextState = state;
    }
    return vec4(nextState, 0.0, 0.0, 1.0);
     */
    
    const float birthCenter = 3.0;
    const float birthRadius = 0.66;
    
    const float keepCenter = 3.0;
    const float keepRadius = 0.9;
    
    float delta = -1.0;
    if (neighborsSum >= birthCenter - birthRadius && neighborsSum <= birthCenter + birthRadius)
    {
        delta = 1.0;
    }
    else if (neighborsSum >= keepCenter - keepRadius && neighborsSum <= keepCenter + keepRadius)
    {
        delta = 0.0;
    }
    float nextState = clamp(state + delta * 0.07, 0.0, 1.0);
    return vec4(nextState, 0.0, 0.0, 1.0);
}

void main()
{
    outCellState = contValues();
    //outCellState = customRules();
    //outCellState = harm();
    //outCellState = oneOut();
    
    //outCellState = lichens();
    //outCellState = majority();
    //outCellState = anneal();
    //outCellState = gameOfLife();
    //outCellState = cyclicCA();
    //outCellState = freeCA();
}