#include "GPUAcceleratedInterlockingTilesAlgorithmNaive.h"

#include "Managers/GeometryBuffersManager.h"
#include "Managers/PipelineStatesManager.h"
#include "Managers/ResourcesManager.h"
#include "Managers/ShadersManager.h"

#include <DirectXColors.h>
#include <DirectXMath.h>
#include <DxErrorChecker.h>
#include <GeometryGenerator.h>
#include <MathHelper.h>

namespace Framework
{
    GPUAcceleratedInterlockingTilesAlgorithmNaive::GPUAcceleratedInterlockingTilesAlgorithmNaive(HINSTANCE hInstance)
        : D3DApplication(hInstance)
        , mWireframeMode(false)
    {
        mMainWindowCaption = L"GPU Accelerated Interlocking Tiles Algorithm Naive Demo";

        mCamera.setPosition(0.0f, 2.0f, -15.0f);

        mLastMousePosition.x = 0;
        mLastMousePosition.y = 0;

        // Shapes world matrices
        DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();
        DirectX::XMStoreFloat4x4(&mWorldMatrix, translation);

        // Directional lights.
        mDirectionalLight[0].mAmbient = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[0].mDiffuse = DirectX::XMFLOAT4(0.7f, 0.7f, 0.6f, 1.0f);
        mDirectionalLight[0].mSpecular = DirectX::XMFLOAT4(0.8f, 0.8f, 0.7f, 1.0f);
        mDirectionalLight[0].mDirection = DirectX::XMFLOAT3(0.707f, 0.0f, 0.707f);

        mDirectionalLight[1].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        mDirectionalLight[1].mDiffuse = DirectX::XMFLOAT4(0.40f, 0.40f, 0.40f, 1.0f);
        mDirectionalLight[1].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[1].mDirection = DirectX::XMFLOAT3(0.0f, -0.707f, 0.707f);

        mDirectionalLight[2].mAmbient = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
        mDirectionalLight[2].mDiffuse = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[2].mSpecular = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
        mDirectionalLight[2].mDirection = DirectX::XMFLOAT3(-0.57735f, -0.57735f, -0.57735f);

        // Initialize material
        mMaterial.mAmbient  = DirectX::XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
        mMaterial.mDiffuse  = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        mMaterial.mSpecular = DirectX::XMFLOAT4(0.4f, 0.4f, 0.4f, 16.0f);
        mMaterial.mReflect  = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    GPUAcceleratedInterlockingTilesAlgorithmNaive::~GPUAcceleratedInterlockingTilesAlgorithmNaive()
    {
        Managers::ResourcesManager::destroyAll();
        Managers::ShadersManager::destroyAll();
        Managers::PipelineStatesManager::destroyAll();
        Managers::GeometryBuffersManager::destroyAll();
    }

    bool GPUAcceleratedInterlockingTilesAlgorithmNaive::init()
    {
        if(!D3DApplication::init())
            return false;

        assert(mDevice);
        mGridPSPerFrameBuffer.initialize(*mDevice);
        mGridHSPerFrameBuffer.initialize(*mDevice);
        mGridDSPerFrameBuffer.initialize(*mDevice);
        mGridVSPerObjectBuffer.initialize(*mDevice);
        
        assert(mImmediateContext);
        Managers::ShadersManager::initAll(*mDevice);   
        Managers::ResourcesManager::initAll(*mDevice, *mImmediateContext);
        Managers::PipelineStatesManager::initAll(*mDevice);
        Managers::GeometryBuffersManager::initAll(*mDevice);

        return true;
    }

    void GPUAcceleratedInterlockingTilesAlgorithmNaive::updateScene(const float dt)
    {
        //
        // Control the camera.
        //
        const float offset = 50.0f;
        if (GetAsyncKeyState('W') & 0x8000)
        {
            mCamera.walk(offset * dt);
        }

        if (GetAsyncKeyState('S') & 0x8000)
        {
            mCamera.walk(-offset * dt);
        }

        if (GetAsyncKeyState('A') & 0x8000)
        {
            mCamera.strafe(-offset * dt);
        }

        if (GetAsyncKeyState('D') & 0x8000)
        {
            mCamera.strafe(offset * dt);
        }

        if (GetAsyncKeyState('T') & 0x8000) 
        {
            mWireframeMode = true;
        }

        if (GetAsyncKeyState('Y') & 0x8000) 
        {
            mWireframeMode = false;
        }
    }

    void GPUAcceleratedInterlockingTilesAlgorithmNaive::drawScene()
    {
        mImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&DirectX::Colors::Silver));
        mImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        mImmediateContext->RSSetState(mWireframeMode ? Managers::PipelineStatesManager::mWireframeRS : nullptr);

        mCamera.updateViewMatrix();
       
        drawGrid();

        // Present results
        const HRESULT result = mSwapChain->Present(0, 0);
        DebugUtils::DxErrorChecker(result);
    }

    void GPUAcceleratedInterlockingTilesAlgorithmNaive::onMouseMove(WPARAM btnState,  const int32_t x, const int32_t y)
    {
        if ((btnState & MK_LBUTTON) != 0)
        {
            // Make each pixel correspond to a quarter of a degree.
            const float dx = DirectX::XMConvertToRadians(0.15f * static_cast<float>(x - mLastMousePosition.x));
            const float dy = DirectX::XMConvertToRadians(0.15f * static_cast<float>(y - mLastMousePosition.y));

            mCamera.pitch(dy);
            mCamera.rotateY(dx);
        }

        mLastMousePosition.x = x;
        mLastMousePosition.y = y;
    }

    void GPUAcceleratedInterlockingTilesAlgorithmNaive::drawGrid()
    {
        //
        // Input Assembler Stage
        //

        // Input Layout 
        ID3D11InputLayout * const inputLayout = Managers::ShadersManager::mTerrainIL;
        mImmediateContext->IASetInputLayout(inputLayout);

        // Primitive Topology
        mImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST);

        // Vertex Buffer
        ID3D11Buffer* vertexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mVertexBuffer;
        uint32_t stride = sizeof(Geometry::GeometryGenerator::Vertex);
        uint32_t offset = 0;
        mImmediateContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        // Index Buffer
        ID3D11Buffer* indexBuffer = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexBuffer;
        mImmediateContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

        //
        // Vertex Shader Stage
        //
        
        // Shader
        ID3D11VertexShader * const vertexShader = Managers::ShadersManager::mTerrainVS;
        mImmediateContext->VSSetShader(vertexShader, nullptr, 0);

        // Per Frame Constant Buffer
        DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&mWorldMatrix);
        DirectX::XMMATRIX worldInverseTranspose = Utils::MathHelper::inverseTranspose(world);

        DirectX::XMStoreFloat4x4(&mGridVSPerObjectBuffer.mData.mWorld, DirectX::XMMatrixTranspose(world));
        DirectX::XMStoreFloat4x4(&mGridVSPerObjectBuffer.mData.mWorldInverseTranspose, DirectX::XMMatrixTranspose(worldInverseTranspose));
        mGridVSPerObjectBuffer.applyChanges(*mImmediateContext);

        // Set Constant Buffers
        ID3D11Buffer* const vertexShaderBuffers[] = { &mGridVSPerObjectBuffer.buffer() };
        mImmediateContext->VSSetConstantBuffers(0, 1, vertexShaderBuffers);

        //
        // Hull Shader Stage
        //

        // Shader
        ID3D11HullShader * const hullShader = Managers::ShadersManager::mTerrainHS;
        mImmediateContext->HSSetShader(hullShader, nullptr, 0);

        // Per Frame Constant Buffer
        mGridHSPerFrameBuffer.mData.mEyePositionW = mCamera.position();
        mGridHSPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set Constant Buffers
        ID3D11Buffer* const hullShaderBuffers = { &mGridHSPerFrameBuffer.buffer() };
        mImmediateContext->HSSetConstantBuffers(0, 1, &hullShaderBuffers);

        //
        // Domain Shader Stage
        //

        // Shader
        ID3D11DomainShader * const domainShader = Managers::ShadersManager::mTerrainDS;
        mImmediateContext->DSSetShader(domainShader, nullptr, 0);

        // Per Frame Constant Buffer
        const DirectX::XMMATRIX viewProjection = mCamera.viewProjection();
        DirectX::XMStoreFloat4x4(&mGridDSPerFrameBuffer.mData.mViewProjection, DirectX::XMMatrixTranspose(viewProjection));
        mGridDSPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set Constant Buffers
        ID3D11Buffer* const domainShaderBuffers = { &mGridDSPerFrameBuffer.buffer() };
        mImmediateContext->DSSetConstantBuffers(0, 1, &domainShaderBuffers);

        // Resources
        ID3D11ShaderResourceView * const domainShaderResources[] = { Managers::ResourcesManager::mHeightMapSRV };
        mImmediateContext->DSSetShaderResources(0, 1, domainShaderResources);

        // Sampler state
        mImmediateContext->DSSetSamplers(0, 1, &Managers::PipelineStatesManager::mLinearSS);

        //
        // Pixel Shader Stage
        //

        // Shader
        ID3D11PixelShader* const pixelShader = Managers::ShadersManager::mTerrainPS;
        mImmediateContext->PSSetShader(pixelShader, nullptr, 0);

        // Per Frame Constant Buffer
        memcpy(&mGridPSPerFrameBuffer.mData.mDirectionalLight, &mDirectionalLight, sizeof(mDirectionalLight));
        mGridPSPerFrameBuffer.mData.mEyePositionW = mCamera.position();
        mGridPSPerFrameBuffer.mData.mMaterial = mMaterial;
        mGridPSPerFrameBuffer.applyChanges(*mImmediateContext);

        // Set constant buffers
        ID3D11Buffer * const pixelShaderBuffers[] = { &mGridPSPerFrameBuffer.buffer() };
        mImmediateContext->PSSetConstantBuffers(0, 1, pixelShaderBuffers);
                
        // Resources
        ID3D11ShaderResourceView * const pixelShaderResources[] = { Managers::ResourcesManager::mFloorDiffuseMapSRV, Managers::ResourcesManager::mFloorNormalMapSRV };
        mImmediateContext->PSSetShaderResources(0, 2, pixelShaderResources);

        // Sampler state
        mImmediateContext->PSSetSamplers(0, 1, &Managers::PipelineStatesManager::mLinearSS);

        //
        // Draw
        // 
        const uint32_t baseVertexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mBaseVertexLocation;
        const uint32_t startIndexLocation = Managers::GeometryBuffersManager::mFloorBufferInfo->mStartIndexLocation;
        const uint32_t indexCount = Managers::GeometryBuffersManager::mFloorBufferInfo->mIndexCount;
        mImmediateContext->DrawIndexed(indexCount, startIndexLocation, baseVertexLocation);
    }    
}