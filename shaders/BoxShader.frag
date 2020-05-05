#version 330 core
uniform vec3 colorInput;
// You can output many things. The first vec4 type output determines the color of the fragment
out vec4 color;

void main()
{
    // Color everything a hot pink color. An alpha of 1.0f means it is not transparent.
    color = vec4(1.0f,1.0f,1.0f, 1.0f);
}