#version 330 core

// Cells indices :

//          0 3 5
//          1 x 6
//          2 4 7

in vec4 inNClasses0;
in vec4 inNClasses1;

in vec3 inN012;
in vec3 inN3X4;
in vec3 inN567;

in vec3 inNE66F;
in vec3 inNG11H;

in float inNB;
in float inN33;
in float inNA;
in float inNC;
in float inN44;
in float inND;

uniform float AA;
uniform float BB;
uniform float CC;
uniform float DD;

out float   outCellState;

void main()
{
    /*
    mat4 neighborsT;
    neighborsT[0] = vec4(inN00, inN10, inN20, inN30);
    neighborsT[1] = vec4(inN01, inN11, inN21, inN31);
    neighborsT[2] = vec4(inN02, inN12, inN22, inN32);
    neighborsT[3] = vec4(inN03, inN13, inN23, inN33);
    
    mat4 neighborsB;
    neighborsB[0] = vec4(inN40, inN50, inN60, inN70);
    neighborsB[1] = vec4(inN41, inN51, inN61, inN71);
    neighborsB[2] = vec4(inN42, inN52, inN62, inN72);
    neighborsB[3] = vec4(inN43, inN53, inN63, inN73);
    
    mat4 maskT;
    maskT[0] = inMaskT0;
    maskT[1] = inMaskT1;
    maskT[2] = inMaskT2;
    maskT[3] = inMaskT3;
    
    mat4 maskB;
    maskB[0] = inMaskB0;
    maskB[1] = inMaskB1;
    maskB[2] = inMaskB2;
    maskB[3] = inMaskB3;
    
    mat4 top = transpose(maskT) * neighborsT;
    mat4 bot = transpose(maskB) * neighborsB;
    
    vec4 cellsTop = vec4(top[0][0], top[1][1], top[2][2], top[3][3]);
    vec4 cellsBot = vec4(bot[0][0], bot[1][1], bot[2][2], bot[3][3]);
    
    vec4 sum = cellsTop + cellsBot;*/
    outCellState = inND;
}