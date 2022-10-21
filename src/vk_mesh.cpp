#include "vk_mesh.hpp"
#include "shaders/bindings.h"

VertexInputDescription Vertex::get_vertex_description()
{

    vk::VertexInputBindingDescription main_binding{.binding = 0,
                                                   .stride  = sizeof(Vertex),
                                                   .inputRate =
                                                       vk::VertexInputRate::eVertex};

    vk::VertexInputAttributeDescription position_attr{
        .location = VERTEX_ATTRIBUTE_LOCATION,
        .binding  = 0,
        .format   = vk::Format::eR32G32B32Sfloat,
        .offset   = offsetof(Vertex, position)};

    vk::VertexInputAttributeDescription normal_attr{.location = NORMAL_ATTRIBUTE_LOCATION,
                                                    .binding  = 0,
                                                    .format =
                                                        vk::Format::eR32G32B32Sfloat,
                                                    .offset = offsetof(Vertex, normal)};

    vk::VertexInputAttributeDescription colour_attr{.location = COLOUR_ATTRIBUTE_LOCATION,
                                                    .binding  = 0,
                                                    .format =
                                                        vk::Format::eR32G32B32Sfloat,
                                                    .offset = offsetof(Vertex, colour)};

    return VertexInputDescription{
        .bindings   = {main_binding},
        .attributes = {position_attr, normal_attr, colour_attr}
    };
}

bool Model::load_from_file(std::filesystem::path const& path)
{
    return load_from_file(path.string());
}

bool Model::load_from_file(std::string const& filename)
{
    static constexpr std::uint32_t flags = aiProcess_Triangulate | aiProcess_FlipUVs;

    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(filename, flags);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        fmt::print("error: unable to load {}", filename);
        fmt::print("error: {}", import.GetErrorString());
        return false;
    }

    process_node(scene->mRootNode, scene);
    return true;
}

void Model::process_node(aiNode* node, aiScene const* scene)
{
    for (std::uint32_t i{0}; i < node->mNumMeshes; ++i)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(process_mesh(mesh, scene));
    }

    for (std::uint32_t i{0}; i < node->mNumChildren; ++i)
    {
        process_node(node->mChildren[i], scene);
    }
}

Mesh Model::process_mesh(aiMesh* mesh, aiScene const*)
{
    // Ignore textures and any extra data for now. We'll worry about it later.
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;

    for (std::uint32_t i{0}; i < mesh->mNumVertices; ++i)
    {
        glm::vec3 position;
        position.x = mesh->mVertices[i].x;
        position.y = mesh->mVertices[i].y;
        position.z = mesh->mVertices[i].z;

        glm::vec3 normal;
        normal.x = mesh->mNormals[i].x;
        normal.y = mesh->mNormals[i].y;
        normal.z = mesh->mNormals[i].z;

        vertices.push_back(
            Vertex{.position = position, .normal = normal, .colour = normal});
    }

    for (std::uint32_t i{0}; i < mesh->mNumFaces; ++i)
    {
        aiFace face = mesh->mFaces[i];
        for (std::uint32_t j{0}; j < face.mNumIndices; ++j)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    return Mesh{.vertices = vertices, .indices = indices};
}
