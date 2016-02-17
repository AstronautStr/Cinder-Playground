#version 330 core
layout(origin_upper_left) in vec4 gl_FragCoord;

uniform vec2 gridSize;
uniform vec2 screenSize;
uniform samplerBuffer gridSampler;

out vec4 color;

void main()
{
    ivec2 cellCoord = ivec2(gl_FragCoord.xy / screenSize * gridSize);
    vec4 cell = texelFetch(gridSampler, int(cellCoord.x * gridSize.x) + cellCoord.y);
    
    float alive = cell.w;
    float freq = cell.y / 20000.0;
    float energy = cell.x;
    
    color = vec4(energy, energy * freq, 0.0, alive);
}