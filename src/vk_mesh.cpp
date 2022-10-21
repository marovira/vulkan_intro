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
