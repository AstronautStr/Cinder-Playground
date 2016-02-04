#version 330 core

in float    inValue;
in float    inValue1;
in float    inValue2;

out float   outValue;

void main()
{
    outValue = inValue + inValue1 + inValue2;
}