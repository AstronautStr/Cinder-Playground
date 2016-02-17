#version 330 core

uniform samplerBuffer gridSampler;
uniform ivec2 GridSize;

in ivec2 inPosition;

out float outCellState;

float getCellState(ivec2 pos)
{
    return texelFetch(gridSampler, pos.x * GridSize.y + pos.y).r;
}

void main()
{
    int aliveBroCount = 0;
    float state = getCellState(inPosition);
    float nextState = state;
    
    for (int i = -1; i <= 1; ++i)
    {
        for (int j = -1; j <= 1; ++j)
        {
            if (i != 0 || j != 0)
            {
                float bro = getCellState(inPosition + ivec2(i, j));
                if (bro > 0)
                    aliveBroCount += 1;
            }
        }
    }
    
    if (state > 0)
    {
        if (aliveBroCount == 2 || aliveBroCount == 3)
            nextState = state;
        else
            nextState = 0.0;
    }
    else
    {
        if (aliveBroCount == 3)
            nextState = 1.0;
    }
    outCellState = nextState;
}