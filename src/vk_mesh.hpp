#pragma once

#include "vk_types.hpp"

struct VertexInputDescription
{
    std::vector<vk::VertexInputBindingDescription> bindings;
    std::vector<vk::VertexInputAttributeDescription> attributes;

    vk::PipelineVertexInputStateCreateFlags flags;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 colour;

    static VertexInputDescription get_vertex_description();
};

struct MeshPushConstants
{
    glm::vec4 data;
    glm::mat4 mvp;
};

struct Mesh
{
    std::vector<Vertex> vertices;
    vk_types::AllocatedBuffer vertex_buffer;
};
