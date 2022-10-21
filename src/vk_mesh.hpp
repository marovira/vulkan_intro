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
    std::vector<std::uint32_t> indices;
    vk_types::AllocatedBuffer vertex_buffer;
    vk_types::AllocatedBuffer index_buffer;
};

class Model
{
public:
    bool load_from_file(std::filesystem::path const& path);
    bool load_from_file(std::string const& filename);

    std::vector<Mesh> meshes;

private:
    void process_node(aiNode* node, aiScene const* scene);
    Mesh process_mesh(aiMesh* mesh, aiScene const* scene);
};
