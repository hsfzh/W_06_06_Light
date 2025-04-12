#pragma once
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

#include "EngineBaseTypes.h"
#include "Define.h"
#include "Container/Map.h"

class FRenderResourceManager;
class FShaderProgram;
class FVBIBTopologyMapping;
class FGraphicsDevice;
class FGizmoRenderPass;
class FLineBatchRenderPass;
class FStaticMeshRenderPass;
class FEditorViewportClient;
class UWorld;

class FRenderer 
{
private:
    void CreateStaticMeshShader();
    void CreateTextureShader();
    void CreateLineBatchShader();
    void CreateFogShader();
    void CreateDebugDepthShader();
public:
    FGraphicsDevice* Graphics;
public:
    ID3D11SamplerState* GetSamplerState(const ESamplerType InType) const;
    ID3D11RasterizerState* GetRasterizerState(const ERasterizerState InState) const;
    ID3D11BlendState* GetBlendState(const EBlendState InState) const;
    ID3D11DepthStencilState* GetDepthStencilState(const EDepthStencilState InState) const;

    ID3D11RasterizerState* GetCurrentRasterizerState() const;
    void SetCurrentRasterizerState(const ERasterizerState InState) { CurrentRasterizerState = InState; }

    std::shared_ptr<FVBIBTopologyMapping> GetVBIBTopologyMapping(const FName InName) { return VBIBTopologyMappings[InName]; }

public:
    void Initialize(FGraphicsDevice* graphics);
    
    //Release
    void Release();
    void ReleaseShader();
    
    void SetViewMode(EViewModeIndex evi);

public://텍스쳐용 기능 추가
    
    // void PrepareTextureShader() const;
    //
    // void RenderTexturePrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    //     ID3D11Buffer* pIndexBuffer, UINT numIndices,
    //     ID3D11ShaderResourceView* _TextureSRV,
    //     ID3D11SamplerState* _SamplerState) const;
    // void RenderTextPrimitive(ID3D11Buffer* pVertexBuffer, UINT numVertices,
    //     ID3D11ShaderResourceView* _TextureSRV,
    //     ID3D11SamplerState* _SamplerState) const;
    //
    // void PrepareSubUVConstant() const;

public:
    //Render Pass Demo
    void AddRenderObjectsToRenderPass(UWorld* InWorld) const;
    void Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    //void RenderGizmos(const UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport);
    //void RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport);
    //void RenderBillboards(UWorld* World,std::shared_ptr<FEditorViewportClient> ActiveViewport);

    // post process
    //void RenderPostProcess(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport, std::shared_ptr<FEditorViewportClient> CurrentViewport);
    //void RenderDebugDepth(std::shared_ptr<FEditorViewportClient> ActiveViewport);
    //void RenderHeightFog(std::shared_ptr<FEditorViewportClient> ActiveViewport, std::shared_ptr<FEditorViewportClient> CurrentViewport);
public:
    void PrepareShader(FName InShaderName);
    void BindConstantBuffers(FName InShaderName);
    
public:
    void MappingVSPSInputLayout(FName InShaderProgramName, FName VSName, FName PSName, ID3D11InputLayout* InInputLayout);
    void MappingVSPSCBSlot(FName InShaderName, const TMap<FShaderConstantKey, uint32>& MappedConstants);
    void MappingVBTopology(FName InObjectName, FName InVBName, uint32 InStride, uint32 InNumVertices, D3D11_PRIMITIVE_TOPOLOGY InTopology= D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    void MappingIB(FName InObjectName, FName InIBName, uint32 InNumIndices);
private: 
    TMap<FName, std::shared_ptr<FShaderProgram>> ShaderPrograms;
    TMap<FName, TMap<FShaderConstantKey, uint32>> ShaderConstantNameAndSlots;
    TMap<FName, std::shared_ptr<FVBIBTopologyMapping>> VBIBTopologyMappings;
public:
    FRenderResourceManager* GetResourceManager() const { return RenderResourceManager; }
private:
    FRenderResourceManager* RenderResourceManager = nullptr;

    std::shared_ptr<FStaticMeshRenderPass> StaticMeshRenderPass;
    std::shared_ptr<FLineBatchRenderPass> LineBatchRenderPass;
    std::shared_ptr<FGizmoRenderPass> GizmoRenderPass;

    ERasterizerState CurrentRasterizerState = ERasterizerState::SolidBack;
};

