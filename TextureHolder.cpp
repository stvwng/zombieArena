#include "TextureHolder.h"
#include <assert.h>

TextureHolder* TextureHolder::m_s_Instance = nullptr;

TextureHolder::TextureHolder()
{
    assert(m_s_Instance == nullptr);
    m_s_Instance = this;
}

Texture& TextureHolder::GetTexture(string const& filename)
{
    // get reference to m_Textures using m_s_Instance
    auto& m = m_s_Instance->m_Textures;

    auto keyValuePair = m.find(filename);

    if (keyValuePair != m.end())
    {
        return keyValuePair->second;
    }
    else
    {
        // file not found
        // create a new key-value pair using the filename
        auto& texture = m[filename];
        texture.loadFromFile(filename);
        return texture;
    }
}