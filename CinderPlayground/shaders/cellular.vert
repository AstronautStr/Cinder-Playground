#version 330 core

#define EPS 0.01

uniform samplerBuffer gridSampler;
uniform ivec2 GridSize;

uniform int ruleRadius;
uniform int cycleN;
uniform float cycleStep;

in ivec2 inPosition;

out float outCellState;

float getCellState(ivec2 pos)
{
    return texelFetch(gridSampler, int(mod(pos.x, GridSize.x)) * GridSize.y + int(mod(pos.y, GridSize.y))).r;
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

void main()
{
    //outCellState = gameOfLife();
    outCellState = cyclicCA();
}