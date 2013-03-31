#pragma once

struct ID3D11Device;
struct ID3D11BlendState;
struct ID3D11RasterizerState;
struct ID3D11SamplerState;

namespace Managers
{
    class PipelineStatesManager
    {
    public:
        static void initAll(ID3D11Device* device);
        static void destroyAll();

        static ID3D11SamplerState* mLinearSS;

        static ID3D11RasterizerState* mWireframeRS;

        static ID3D11BlendState* mAlphaToCoverageBS;
        static ID3D11BlendState* mTransparentBS;

    private:
        PipelineStatesManager();
        ~PipelineStatesManager();
        PipelineStatesManager(const PipelineStatesManager& oipelineStatesManager);
        const PipelineStatesManager& operator=(const PipelineStatesManager& pipelineStatesManager);
    };
}