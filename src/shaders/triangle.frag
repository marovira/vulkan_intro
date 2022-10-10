#version 450 core

layout (location = 0) in vec3 vert_colour;

layout (location = 0) out vec4 frag_colour;

void main()
{
    frag_colour = vec4(vert_colour, 1);
}
