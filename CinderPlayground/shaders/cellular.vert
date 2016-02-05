#version 330 core

// Cells indices :

//          0 1 2
//          3 4 5
//          6 7 8

in float    inCell0_0;
in float    inCell0_1;
in float    inCell0_2;
in float    inCell0_3;
in float    inCell0_4;
in float    inCell0_5;
in float    inCell0_6;
in float    inCell0_7;
in float    inCell0_8;



out float   outCellState;

void main()
{
    outCellState = inCell0 + inCell1 + inCell2 + inCell3 + inCell4 + inCell5 + inCell6 + inCell7 + inCell8;
}