#include "GeometryBuffersManager.h"

#include <cassert>

#include <GeometryGenerator.h>
#include <DxErrorChecker.h>
#include <MathHelper.h>

namespace Managers
{
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mCylinderBufferInfo =  nullptr;
    GeometryBuffersManager::IndexedBufferInfo* GeometryBuffersManager::mFloorBufferInfo = nullptr;
    GeometryBuffersManager::NonIndexedBufferInfo* GeometryBuffersManager::mInstancedBufferInfo = nullptr;

    void GeometryBuffersManager::initAll(ID3D11Device* device)
    {
        assert(device);

        buildCylinderBuffers(device);
        buildFloorBuffers(device);
        buildInstancedBuffer(device);
    }

    void GeometryBuffersManager::destroyAll()
    {
        mCylinderBufferInfo->mVertexBuffer->Release();
        mCylinderBufferInfo->mIndexBuffer->Release();
        delete mCylinderBufferInfo;

        mFloorBufferInfo->mVertexBuffer->Release();
        mFloorBufferInfo->mIndexBuffer->Release();
        delete mFloorBufferInfo;

        mInstancedBufferInfo->mVertexBuffer->Release();
        delete mInstancedBufferInfo;
    }

    void GeometryBuffersManager::buildInstancedBuffer(ID3D11Device * const device)
    {
        assert(device);

        // World matrices to use as instanced data
        Managers::GeometryBuffersManager::InstancedData worldMatrices[5];
        DirectX::XMMATRIX translation = DirectX::XMMatrixIdentity();        
        translation = DirectX::XMMatrixTranslation(0.0f, 25.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&worldMatrices[0].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(50.0f, 25.0f, 50.0f);
        DirectX::XMStoreFloat4x4(&worldMatrices[1].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(-50.0f, 25.0f, -50.0f);
        DirectX::XMStoreFloat4x4(&worldMatrices[2].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(50.0f, 25.0f, -50.0f);
        DirectX::XMStoreFloat4x4(&worldMatrices[3].mWorld, translation);

        translation = DirectX::XMMatrixTranslation(-50.0f, 25.0f, 50.0f);
        DirectX::XMStoreFloat4x4(&worldMatrices[4].mWorld, translation);

        //
        // Create NonIndexedBuffer and fill known data
        //
        mInstancedBufferInfo = new NonIndexedBufferInfo();
        const size_t vertexCount = 5;
        mInstancedBufferInfo->mBaseVertexLocation = 0;
        mInstancedBufferInfo->mVertexCount = vertexCount;
       
        //
        // Create vertex buffer
        //
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(InstancedData) * vertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &worldMatrices[0];

        const HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mInstancedBufferInfo->mVertexBuffer);
        DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildCylinderBuffers(ID3D11Device * const device)
    {
        assert(device);

        mCylinderBufferInfo = new IndexedBufferInfo();

        // Set base vertex location
        mCylinderBufferInfo->mBaseVertexLocation = 0;

        // Fill vertices to insert in the vertex buffer
        const float bottomRadius = 10.0f; 
        const float topRadius = 10.0f; 
        const float height = 50.0f;
        const uint32_t sliceCount = 200;
        const uint32_t stackCount = 100;
        MeshData cylinder;
        GeometryGenerator::generateCylinder(bottomRadius, topRadius, height, sliceCount, stackCount, cylinder);

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = static_cast<uint32_t> (sizeof(VertexData) * cylinder.mVertices.size());
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &cylinder.mVertices[0];

        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mCylinderBufferInfo->mVertexBuffer);
        DxErrorChecker(result);

        // Set index count
        mCylinderBufferInfo->mIndexCount = static_cast<uint32_t> (cylinder.mIndices.size());

        // Create and fill index buffer
        D3D11_BUFFER_DESC ibd;
        ibd.Usage = D3D11_USAGE_IMMUTABLE;
        ibd.ByteWidth = sizeof(uint32_t) * mCylinderBufferInfo->mIndexCount;
        ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        ibd.CPUAccessFlags = 0;
        ibd.StructureByteStride = 0;
        ibd.MiscFlags = 0;

        initData.pSysMem = &cylinder.mIndices[0];

        result = device->CreateBuffer(&ibd, &initData, &mCylinderBufferInfo->mIndexBuffer);
        DxErrorChecker(result);
    }

    void GeometryBuffersManager::buildFloorBuffers(ID3D11Device * const device)
    {
        assert(device);

        // Create IndexedBufferInfo for floor
        mFloorBufferInfo = new IndexedBufferInfo();

        // 
        // Calculate vertices and indices
        // Cache vertex offset, index count and offset
        //
        MeshData grid;
        GeometryGenerator::generateGrid(400.0f, 400.0f, 100, 100, grid);

        // Cache base vertex location
        mFloorBufferInfo->mBaseVertexLocation = 0;

        // Cache the index count
        mFloorBufferInfo->mIndexCount = static_cast<uint32_t> (grid.mIndices.size());

        // Cache the starting index
        mFloorBufferInfo->mStartIndexLocation = 0; 

        // Compute the total number of vertices
        const uint32_t totalVertexCount = static_cast<uint32_t> (grid.mVertices.size());

        // Create vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc;
        vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        vertexBufferDesc.ByteWidth = sizeof(VertexData) * totalVertexCount;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;

        D3D11_SUBRESOURCE_DATA initData;
        initData.pSysMem = &grid.mVertices[0];
        HRESULT result = device->CreateBuffer(&vertexBufferDesc, &initData, &mFloorBufferInfo->mVertexBuffer);
        DxErrorChecker(result);

        // Fill and Create index buffer
        const uint32_t totalIndexCount = mFloorBufferInfo->mIndexCount;

        D3D11_BUFFER_DESC indexBufferDesc;
        indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        indexBufferDesc.ByteWidth = sizeof(uint32_t) * totalIndexCount;
        indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        indexBufferDesc.CPUAccessFlags = 0;
        indexBufferDesc.MiscFlags = 0;

        initData.pSysMem = &grid.mIndices[0];
        result = device->CreateBuffer(&indexBufferDesc, &initData, &mFloorBufferInfo->mIndexBuffer);
        DxErrorChecker(result);
    }
}