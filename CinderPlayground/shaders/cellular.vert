#version 330 core

in float    inValue;
out float   outValue;

void main()
{
    outValue = 99.0f + inValue;
}