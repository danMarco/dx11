#pragma once

struct ID3D11ComputeShader;
struct ID3D11Device;
struct ID3D11DomainShader;
struct ID3D11HullShader;
struct ID3D11InputLayout;
struct ID3D11PixelShader;
struct ID3D11VertexShader;

struct Shaders
{
    Shaders()
        : mTerrainVS(nullptr)
        , mTerrainIL(nullptr)
        , mTerrainPS(nullptr)
        , mTerrainHS(nullptr)
        , mTerrainDS(nullptr)
        , mTerrainCS(nullptr) 
    {

    }

    ID3D11VertexShader* mTerrainVS;
    ID3D11InputLayout* mTerrainIL;
    ID3D11PixelShader* mTerrainPS;
    ID3D11HullShader* mTerrainHS;
    ID3D11DomainShader* mTerrainDS;
    ID3D11ComputeShader* mTerrainCS;
};

namespace ShadersUtils
{
    void init(ID3D11Device& device, Shaders& shaders);
    void destroy(Shaders& shaders);
}
