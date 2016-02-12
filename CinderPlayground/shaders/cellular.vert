#version 330 core

// Cells indices :

//          0 1 2
//          3 4 5
//          6 7 8

in float inCellState;

in mat4 inMaskTop;
in mat4 inMaskBot;

in float inNT00; in float inNT10; in float inNT20; in float inNT30;
in float inNT01; in float inNT11; in float inNT21; in float inNT31;
in float inNT02; in float inNT12; in float inNT22; in float inNT32;
in float inNT03; in float inNT13; in float inNT23; in float inNT33;

in float inNB00; in float inNB10; in float inNB20; in float inNB30;
in float inNB01; in float inNB11; in float inNB21; in float inNB31;
in float inNB02; in float inNB12; in float inNB22; in float inNB32;
in float inNB03; in float inNB13; in float inNB23; in float inNB33;

out float   outCellState;

void main()
{
    mat4 neighborsTop;
    neighborsTop[0] = vec4(inNT00, inNT10, inNT20, inNT30);
    neighborsTop[1] = vec4(inNT01, inNT11, inNT21, inNT31);
    neighborsTop[2] = vec4(inNT02, inNT12, inNT22, inNT32);
    neighborsTop[3] = vec4(inNT03, inNT13, inNT23, inNT33);
    
    mat4 neighborsBot;
    neighborsBot[0] = vec4(inNB00, inNB10, inNB20, inNB30);
    neighborsBot[1] = vec4(inNB01, inNB11, inNB21, inNB31);
    neighborsBot[2] = vec4(inNB02, inNB12, inNB22, inNB32);
    neighborsBot[3] = vec4(inNB03, inNB13, inNB23, inNB33);
    
    mat4 top = transpose(inMaskTop) * inNeighborsTop;
    mat4 bot = transpose(inMaskBot) * inNeighborsBot;
    
    vec4 cellsTop = vec4(top[0][0], top[1][1], top[2][2], top[3][3]);
    vec4 cellsBot = vec4(bot[0][0], bot[1][1], bot[2][2], bot[3][3]);
    
    outCellState = 0.0;
}