#version 450 core

layout (location = 0) out vec3 vert_colour;

void main()
{
    const vec3 positions[3] = vec3[3](
            vec3( 1.0f,  1.0f, 0.0f),
            vec3(-1.0f,  1.0f, 0.0f),
            vec3( 0.0f, -1.0f, 0.0f)
            );

    const vec3 colours[3] = vec3[3](
            vec3(1, 0, 0),
            vec3(0, 1, 0),
            vec3(0, 0, 1)
            );

    gl_Position = vec4(positions[gl_VertexIndex], 1.0f);
    vert_colour = colours[gl_VertexIndex];
}
