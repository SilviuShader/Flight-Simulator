#pragma once

#include <glm/glm.hpp>

struct VertexPosition
{
public:

    VertexPosition()                                 = default;

    VertexPosition(const VertexPosition&)            = default;
    VertexPosition& operator=(const VertexPosition&) = default;

    VertexPosition(VertexPosition&&)                 = default;
    VertexPosition& operator=(VertexPosition&&)      = default;

    VertexPosition(glm::vec3 const&)                  noexcept;

    static void SetLayout();
    static void ResetLayout();

public:

    glm::vec3 Position;
};

struct VertexPositionColor
{
public:

    VertexPositionColor()                                      = default;

    VertexPositionColor(const VertexPositionColor&)            = default;
    VertexPositionColor& operator=(const VertexPositionColor&) = default;

    VertexPositionColor(VertexPositionColor&&)                 = default;
    VertexPositionColor& operator=(VertexPositionColor&&)      = default;

    VertexPositionColor(glm::vec3 const&, glm::vec3 const&)     noexcept;

    static void SetLayout();
    static void ResetLayout();

public:

    glm::vec3 Position;
    glm::vec3 Color;
};

struct VertexPositionTexture
{
public:

    VertexPositionTexture()                                        = default;

    VertexPositionTexture(const VertexPositionTexture&)            = default;
    VertexPositionTexture& operator=(const VertexPositionTexture&) = default;

    VertexPositionTexture(VertexPositionTexture&&)                 = default;
    VertexPositionTexture& operator=(VertexPositionTexture&&)      = default;

    VertexPositionTexture(glm::vec3 const&, glm::vec2 const&)       noexcept;

    static void SetLayout();
    static void ResetLayout();

public:

    glm::vec3 Position;
    glm::vec2 TexCoords;
};

struct VertexNormalTexture
{
public:

    VertexNormalTexture()                                                    = default;
    VertexNormalTexture(const VertexNormalTexture&)                          = default;
    VertexNormalTexture& operator=(const VertexNormalTexture&)               = default;
                                                                             
    VertexNormalTexture(VertexNormalTexture&&)                               = default;
    VertexNormalTexture& operator=(VertexNormalTexture&&)                    = default;

    VertexNormalTexture(glm::vec3 const&, glm::vec3 const&, glm::vec2 const&) noexcept;

    static void SetLayout();
    static void ResetLayout();

public:

    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
};