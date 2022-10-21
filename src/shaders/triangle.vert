#version 450 core
#extension GL_GOOGLE_include_directive : require

#include "bindings.h"

layout (location = VERTEX_ATTRIBUTE_LOCATION) in vec3 position;
layout (location = NORMAL_ATTRIBUTE_LOCATION) in vec3 normal;
layout (location = COLOUR_ATTRIBUTE_LOCATION) in vec3 colour;

layout (location = 0) out vec3 vert_colour;

layout (push_constant) uniform constants
{
    vec4 data;
    mat4 mvp;
} PushConstants;

void main()
{
    gl_Position = PushConstants.mvp * vec4(position, 1.0f);
    vert_colour = colour;
}
