#version 330 core

#define M_PI 3.14159265358979323846
#define M_2PI (2.0 * M_PI)

in int samplePosition;
uniform int samplesElapsed;
uniform int sampleRate;
uniform int cellsCount;

uniform samplerBuffer cellsSampler;

out float sampleValue;

void main()
{
    float value = 0.0;
    int sampleIndex = samplesElapsed + samplePosition;
    float time2pi = float(sampleIndex) / float(sampleRate) * M_2PI;
    for (int i = 0; i < cellsCount; ++i)
    {
        vec4 cell = texelFetch(cellsSampler, i);
        value += cell.x * sin(time2pi * cell.y + cell.z);
    }
    sampleValue = value / float(cellsCount);
}