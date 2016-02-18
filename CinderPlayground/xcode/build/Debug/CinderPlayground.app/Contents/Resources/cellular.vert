#version 330 core

uniform samplerBuffer gridSampler;
uniform ivec2 GridSize;
uniform int ruleRadius;

in ivec2 inPosition;

out float outCellState;

float getCellState(ivec2 pos)
{
    return texelFetch(gridSampler, pos.x * GridSize.y + pos.y).r;
}

void main()
{
    float aliveCount = 0.0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -ruleRadius; i <= ruleRadius; ++i)
    {
        for (int j = -ruleRadius; j <= ruleRadius; ++j)
        {
            aliveCount += getCellState(inPosition + ivec2(i, j)) * clamp(float(i * i + j * j), 0.0, 1.0);
        }
    }
    
    if (state > 0)
    {
        if (aliveCount >= 2 && aliveCount <= 3)
            nextState = state;
        else
            nextState = 0.0;
    }
    else
    {
        if (aliveCount == 3)
            nextState = 1.0;
    }
    outCellState = nextState;
}