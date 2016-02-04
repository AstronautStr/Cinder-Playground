#version 330 core

// Cells indices :

//          0 1 2
//          3 4 5
//          6 7 8

in float    inCell0;
in float    inCell1;
in float    inCell2;
in float    inCell3;
in float    inCell4;
in float    inCell5;
in float    inCell6;
in float    inCell7;
in float    inCell8;

out float   outCellState;

void main()
{
    outCellState = inCell4;
}