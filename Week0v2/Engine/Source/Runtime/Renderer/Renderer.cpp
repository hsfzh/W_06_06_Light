#include "Renderer.h"
#include <d3dcompiler.h>

#include "RenderResourceManager.h"
#include "VBIBTopologyMapping.h"
#include "Engine/World.h"
#include "D3D11RHI/GraphicDevice.h"
#include "Launch/EditorEngine.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UnrealEd/PrimitiveBatch.h"
#include "PropertyEditor/ShowFlags.h"
#include "UObject/UObjectIterator.h"
#include "D3D11RHI/FShaderProgram.h"
#include "RenderPass/GizmoRenderPass.h"
#include "RenderPass/LineBatchRenderPass.h"
#include "RenderPass/StaticMeshRenderPass.h"

void FRenderer::Initialize(FGraphicsDevice* graphics)
{
    Graphics = graphics;
    RenderResourceManager = new FRenderResourceManager(graphics);
    RenderResourceManager->Initialize();
    CreateStaticMeshShader();
    CreateLineBatchShader();
}

void FRenderer::PrepareShader(const FName InShaderName)
{
    ShaderPrograms[InShaderName]->Bind();

    BindConstantBuffers(InShaderName);
}

void FRenderer::BindConstantBuffers(const FName InShaderName)
{
    TMap<FShaderConstantKey, uint32> curShaderBindedConstant = ShaderConstantNameAndSlots[InShaderName];
    for (const auto item : curShaderBindedConstant)
    {
        auto curConstantBuffer = RenderResourceManager->GetConstantBuffer(item.Key.ConstantName);
        if (item.Key.ShaderType == EShaderStage::VS)
        {
            if (curConstantBuffer)
                Graphics->DeviceContext->VSSetConstantBuffers(item.Value, 1, &curConstantBuffer);
        }
        else if (item.Key.ShaderType == EShaderStage::PS)
        {
            if (curConstantBuffer)
                Graphics->DeviceContext->PSSetConstantBuffers(item.Value, 1, &curConstantBuffer);
        }
    }
}

void FRenderer::Release()
{
    RenderResourceManager->ReleaseResources();
    
    delete RenderResourceManager;
    RenderResourceManager = nullptr;
    
    for (const auto item : ShaderPrograms)
    {
        item.Value->Release();
    }
}

#pragma region Shader
void FRenderer::CreateStaticMeshShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    VertexShader = RenderResourceManager->GetVertexShader(TEXT("StaticMeshVS"));
    if (VertexShader == nullptr)
    {
        Graphics->CreateVertexShader(TEXT("StaticMeshVertexShader.hlsl"), nullptr, &VSBlob_StaticMesh, &VertexShader);
    }
    else
    {
        FGraphicsDevice::CompileVertexShader(TEXT("StaticMeshVertexShader.hlsl"), nullptr,  &VSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetVertexShader(TEXT("StaticMeshVS"), VertexShader);
    
    PixelShader = RenderResourceManager->GetPixelShader(TEXT("StaticMeshPS"));
    if (PixelShader == nullptr)
    {
        Graphics->CreatePixelShader(TEXT("StaticMeshPixelShader.hlsl"), nullptr, &PSBlob_StaticMesh, &PixelShader);
    }
    else
    {
        FGraphicsDevice::CompilePixelShader(TEXT("StaticMeshPixelShader.hlsl"), nullptr, &PSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetPixelShader(TEXT("StaticMeshPS"), PixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    
    Graphics->Device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_StaticMesh);
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_StaticMesh);
    
    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const FConstantBufferInfo item : VertexStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    for (const FConstantBufferInfo item :PixelStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    MappingVSPSInputLayout(TEXT("StaticMesh"), TEXT("StaticMeshVS"), TEXT("StaticMeshPS"), InputLayout);
    MappingVSPSCBSlot(TEXT("StaticMesh"), ShaderStageToCB);


    StaticMeshRenderPass = std::make_shared<FStaticMeshRenderPass>(TEXT("StaticMesh"));
    GizmoRenderPass = std::make_shared<FGizmoRenderPass>(TEXT("StaticMesh"));
    // TODO : Create RenderPass


    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh)
}

void FRenderer::CreateTextureShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    VertexShader = RenderResourceManager->GetVertexShader(TEXT("TextureVS"));
    if (VertexShader == nullptr)
    {
        Graphics->CreateVertexShader(TEXT("TextureVertexShader.hlsl"), nullptr, &VSBlob_StaticMesh, &VertexShader);
    }
    else
    {
        FGraphicsDevice::CompileVertexShader(TEXT("TextureVertexShader.hlsl"), nullptr,  &VSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetVertexShader(TEXT("TextureVS"), VertexShader);
    
    PixelShader = RenderResourceManager->GetPixelShader(TEXT("TexturePS"));
    if (PixelShader == nullptr)
    {
        Graphics->CreatePixelShader(TEXT("TexturePixelShader.hlsl"), nullptr, &PSBlob_StaticMesh, &PixelShader);
    }
    else
    {
        FGraphicsDevice::CompilePixelShader(TEXT("TexturePixelShader.hlsl"), nullptr, &PSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetPixelShader(TEXT("TexturePS"), PixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    
    Graphics->Device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_StaticMesh);
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_StaticMesh);
    
    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const FConstantBufferInfo item : VertexStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    for (const FConstantBufferInfo item :PixelStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    MappingVSPSInputLayout(TEXT("Texture"), TEXT("TextureVS"), TEXT("TexturePS"), InputLayout);
    MappingVSPSCBSlot(TEXT("Texture"), ShaderStageToCB);

    // TODO : Create RenderPass

    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh)
}

void FRenderer::CreateLineBatchShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    VertexShader = RenderResourceManager->GetVertexShader(TEXT("LineVS"));
    if (VertexShader == nullptr)
    {
        Graphics->CreateVertexShader(TEXT("LineVertexShader.hlsl"), nullptr, &VSBlob_StaticMesh, &VertexShader);
    }
    else
    {
        FGraphicsDevice::CompileVertexShader(TEXT("LineVertexShader.hlsl"), nullptr,  &VSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetVertexShader(TEXT("LineVS"), VertexShader);
    
    PixelShader = RenderResourceManager->GetPixelShader(TEXT("LinePS"));
    if (PixelShader == nullptr)
    {
        Graphics->CreatePixelShader(TEXT("LinePixelShader.hlsl"), nullptr, &PSBlob_StaticMesh, &PixelShader);
    }
    else
    {
        FGraphicsDevice::CompilePixelShader(TEXT("LinePixelShader.hlsl"), nullptr, &PSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetPixelShader(TEXT("LinePS"), PixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        // 정점 ID: 32비트 부호 없는 정수, 입력 슬로트 0, Per-Vertex 데이터
        { "SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    
        // 인스턴스 ID: 32비트 부호 없는 정수, 입력 슬로트 1, Per-Instance 데이터
        { "SV_InstanceID", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };
    
    Graphics->Device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_StaticMesh);
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_StaticMesh);
    
    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const FConstantBufferInfo item : VertexStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    for (const FConstantBufferInfo item :PixelStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    MappingVSPSInputLayout(TEXT("LineBatch"), TEXT("LineVS"), TEXT("LinePS"), InputLayout);
    MappingVSPSCBSlot(TEXT("LineBatch"), ShaderStageToCB);
    
    LineBatchRenderPass = std::make_shared<FLineBatchRenderPass>(TEXT("LineBatch"));

    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh)
}

void FRenderer::CreateFogShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    VertexShader = RenderResourceManager->GetVertexShader(TEXT("FogVS"));
    if (VertexShader == nullptr)
    {
        Graphics->CreateVertexShader(TEXT("HeightFogVertexShader.hlsl"), nullptr, &VSBlob_StaticMesh, &VertexShader);
    }
    else
    {
        FGraphicsDevice::CompileVertexShader(TEXT("HeightFogVertexShader.hlsl"), nullptr,  &VSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetVertexShader(TEXT("FogVS"), VertexShader);
    
    PixelShader = RenderResourceManager->GetPixelShader(TEXT("FogPS"));
    if (PixelShader == nullptr)
    {
        Graphics->CreatePixelShader(TEXT("HeightFogPixelShader.hlsl"), nullptr, &PSBlob_StaticMesh, &PixelShader);
    }
    else
    {
        FGraphicsDevice::CompilePixelShader(TEXT("HeightFogPixelShader.hlsl"), nullptr, &PSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetPixelShader(TEXT("FogPS"), PixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        // 정점 ID: 32비트 부호 없는 정수, 입력 슬로트 0, Per-Vertex 데이터
        { "SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    Graphics->Device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_StaticMesh);
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_StaticMesh);
    
    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const FConstantBufferInfo item : VertexStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    for (const FConstantBufferInfo item :PixelStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    MappingVSPSInputLayout(TEXT("Fog"), TEXT("FogVS"), TEXT("FogPS"), InputLayout);
    MappingVSPSCBSlot(TEXT("Fog"), ShaderStageToCB);

    // TODO : Create RenderPass

    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh)
}

void FRenderer::CreateDebugDepthShader()
{
    ID3DBlob* VSBlob_StaticMesh = nullptr;
    ID3DBlob* PSBlob_StaticMesh = nullptr;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;
    
    VertexShader = RenderResourceManager->GetVertexShader(TEXT("DebugDepthVS"));
    if (VertexShader == nullptr)
    {
        Graphics->CreateVertexShader(TEXT("DebugDepthVertexShader.hlsl"), nullptr, &VSBlob_StaticMesh, &VertexShader);
    }
    else
    {
        FGraphicsDevice::CompileVertexShader(TEXT("DebugDepthVertexShader.hlsl"), nullptr,  &VSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetVertexShader(TEXT("DebugDepthVS"), VertexShader);
    
    PixelShader = RenderResourceManager->GetPixelShader(TEXT("DebugDepthPS"));
    if (PixelShader == nullptr)
    {
        Graphics->CreatePixelShader(TEXT("DebugDepthPixelShader.hlsl"), nullptr, &PSBlob_StaticMesh, &PixelShader);
    }
    else
    {
        FGraphicsDevice::CompilePixelShader(TEXT("DebugDepthPixelShader.hlsl"), nullptr, &PSBlob_StaticMesh);
    }
    RenderResourceManager->AddOrSetPixelShader(TEXT("DebugDepthPS"), PixelShader);

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] =
    {
        // 정점 ID: 32비트 부호 없는 정수, 입력 슬로트 0, Per-Vertex 데이터
        { "SV_VertexID", 0, DXGI_FORMAT_R32_UINT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    Graphics->Device->CreateInputLayout(
        layoutDesc, ARRAYSIZE(layoutDesc), VSBlob_StaticMesh->GetBufferPointer(), VSBlob_StaticMesh->GetBufferSize(), &InputLayout
    );
    
    const TArray<FConstantBufferInfo> VertexStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(VSBlob_StaticMesh);
    const TArray<FConstantBufferInfo> PixelStaticMeshConstant = FGraphicsDevice::ExtractConstantBufferNames(PSBlob_StaticMesh);
    
    TMap<FShaderConstantKey, uint32> ShaderStageToCB;

    for (const FConstantBufferInfo item : VertexStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::VS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    for (const FConstantBufferInfo item :PixelStaticMeshConstant)
    {
        ShaderStageToCB[{EShaderStage::PS, item.Name}] = item.BindSlot;
        if (RenderResourceManager->GetConstantBuffer(item.Name) == nullptr)
        {
            ID3D11Buffer* ConstantBuffer = RenderResourceManager->CreateConstantBuffer(item.ByteWidth);
            RenderResourceManager->AddOrSetConstantBuffer(item.Name, ConstantBuffer);
        }
    }

    MappingVSPSInputLayout(TEXT("DebugDepth"), TEXT("DebugDepthVS"), TEXT("DebugDepthPS"), InputLayout);
    MappingVSPSCBSlot(TEXT("DebugDepth"), ShaderStageToCB);

    // TODO : Create RenderPass

    SAFE_RELEASE(VSBlob_StaticMesh)
    SAFE_RELEASE(PSBlob_StaticMesh) 
}

ID3D11SamplerState* FRenderer::GetSamplerState(const ESamplerType InType) const
{
    return RenderResourceManager->GetSamplerState(InType);
}

ID3D11RasterizerState* FRenderer::GetRasterizerState(const ERasterizerState InState) const
{
    return RenderResourceManager->GetRasterizerState(InState);
}

ID3D11BlendState* FRenderer::GetBlendState(const EBlendState InState) const
{
    return RenderResourceManager->GetBlendState(InState);
}

ID3D11DepthStencilState* FRenderer::GetDepthStencilState(const EDepthStencilState InState) const
{
    return RenderResourceManager->GetDepthStencilState(InState);
}

ID3D11RasterizerState* FRenderer::GetCurrentRasterizerState() const
{
    return RenderResourceManager->GetRasterizerState(CurrentRasterizerState);
}
void FRenderer::ReleaseShader()
{
    // RenderResourceManager->ReleaseShader(InputLayout, VertexShader, PixelShader);
    // ShaderManager.ReleaseShader(TextureInputLayout, VertexTextureShader, PixelTextureShader);
    // ShaderManager.ReleaseShader(nullptr, VertexLineShader, PixelLineShader);
    // ShaderManager.ReleaseShader(nullptr, DebugDepthVertexShader, DebugDepthPixelShader);
    // ShaderManager.ReleaseShader(nullptr, HeightFogVertexShader, HeightFogPixelShader);
}
#pragma endregion Shader
// void FRenderer::   PrepareRender()
// {
//     if (GEngine->GetWorld()->WorldType == EWorldType::Editor)
//     {
//         for (const auto iter : TObjectRange<USceneComponent>())
//         {
//                 UE_LOG(LogLevel::Display, "%d", GUObjectArray.GetObjectItemArrayUnsafe().Num());
//                 if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter))
//                 {
//                     if (!Cast<UGizmoBaseComponent>(iter))
//                         StaticMeshObjs.Add(pStaticMeshComp);
//                 }
//                 if (UGizmoBaseComponent* pGizmoComp = Cast<UGizmoBaseComponent>(iter))
//                 {
//                     GizmoObjs.Add(pGizmoComp);
//                 }
//                 if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter))
//                 {
//                     BillboardObjs.Add(pBillboardComp);
//                 }
//                 if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter))
//                 {
//                     LightObjs.Add(pLightComp);
//                 }
//         }
//     }
//     else if (GEngine->GetWorld()->WorldType == EWorldType::PIE)
//     {
//         for (const auto iter : GEngine->GetWorld()->GetActors())
//         {
//             
//             for (const auto iter2 : iter->GetComponents())
//             {
//                 if (UStaticMeshComponent* pStaticMeshComp = Cast<UStaticMeshComponent>(iter2))
//                 {
//                     if (!Cast<UGizmoBaseComponent>(iter2))
//                         StaticMeshObjs.Add(pStaticMeshComp);
//                 }
//                 if (UBillboardComponent* pBillboardComp = Cast<UBillboardComponent>(iter2))
//                 {
//                     BillboardObjs.Add(pBillboardComp);
//                 }
//                 if (ULightComponentBase* pLightComp = Cast<ULightComponentBase>(iter2))
//                 {
//                     LightObjs.Add(pLightComp);
//                 }
//             }
//         }
//     }
// }

void FRenderer::Render(UWorld* World, const std::shared_ptr<FEditorViewportClient>& ActiveViewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &ActiveViewport->GetD3DViewport());
    
    if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
    {
        StaticMeshRenderPass->Prepare(ActiveViewport);
        StaticMeshRenderPass->Execute(ActiveViewport);
    }

    LineBatchRenderPass->Prepare(ActiveViewport);
    LineBatchRenderPass->Execute(ActiveViewport);

    if (World->GetSelectedActor() != nullptr)
    {
        GizmoRenderPass->Prepare(ActiveViewport);
        GizmoRenderPass->Execute(ActiveViewport);
    }
    
    //Graphics->ChangeRasterizer(ActiveViewport->GetViewMode());
    //ChangeViewMode(ActiveViewport->GetViewMode());

    //RenderPostProcess(World, ActiveViewport);
    // 0. 광원 렌더
    //RenderLight(World, ActiveViewport);

    // 2. 스태틱 메시 렌더

    //if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Primitives))
        //RenderStaticMeshes(World, ActiveViewport);

    // 3. 빌보드 렌더(빌보드, 텍스트)
    //if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))
        //RenderBillboards(World, ActiveViewport);
    
    // 6. 포스트 프로세스
    //RenderPostProcess(World, ActiveViewport, ActiveViewport);

    //ClearRenderArr();
}

// void FRenderer::RenderTexturePrimitive(
//     ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11Buffer* pIndexBuffer, UINT numIndices, ID3D11ShaderResourceView* _TextureSRV,
//     ID3D11SamplerState* _SamplerState
// ) const
//{
    // if (!_TextureSRV || !_SamplerState)
    // {
    //     Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    // }
    // if (numIndices <= 0)
    // {
    //     Console::GetInstance().AddLog(LogLevel::Warning, "numIndices Error");
    // }
    // UINT offset = 0;
    // Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    // Graphics->DeviceContext->IASetIndexBuffer(pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    // Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    // Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    // Graphics->DeviceContext->DrawIndexed(numIndices, 0, 0);
//}

// void FRenderer::RenderTextPrimitive(
//     ID3D11Buffer* pVertexBuffer, UINT numVertices, ID3D11ShaderResourceView* _TextureSRV, ID3D11SamplerState* _SamplerState
// ) const
//{
    // if (!_TextureSRV || !_SamplerState)
    // {
    //     Console::GetInstance().AddLog(LogLevel::Warning, "SRV, Sampler Error");
    // }
    // UINT offset = 0;
    // Graphics->DeviceContext->IASetVertexBuffers(0, 1, &pVertexBuffer, &TextureStride, &offset);
    // Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // Graphics->DeviceContext->PSSetShaderResources(0, 1, &_TextureSRV);
    // Graphics->DeviceContext->PSSetSamplers(0, 1, &_SamplerState);
    // Graphics->DeviceContext->Draw(numVertices, 0);
//}


// void FRenderer::RenderBillboards(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
// {
  //   PrepareTextureShader();
  //   PrepareSubUVConstant();
  //   for (auto BillboardComp : BillboardObjs)
  //   {
  //       ConstantBufferUpdater.UpdateSubUVConstant(SubUVConstantBuffer, BillboardComp->finalIndexU, BillboardComp->finalIndexV);
  //
  //       FMatrix Model = BillboardComp->CreateBillboardMatrix();
  //
  //       // 최종 MVP 행렬
		// FMatrix ViewProj = ActiveViewport->GetViewMatrix() * ActiveViewport->GetProjectionMatrix();
  //
  //       FMatrix NormalMatrix = FMatrix::Transpose(FMatrix::Inverse(Model));
  //       FVector4 UUIDColor = BillboardComp->EncodeUUID() / 255.0f;
  //       if (BillboardComp == World->GetPickingGizmo())
  //           ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, ViewProj, NormalMatrix, UUIDColor, true);
  //       else
  //           ConstantBufferUpdater.UpdateConstant(ConstantBuffer, Model, ViewProj, NormalMatrix, UUIDColor, false);
  //
  //       if (UParticleSubUVComp* SubUVParticle = Cast<UParticleSubUVComp>(BillboardComp))
  //       {
  //           RenderTexturePrimitive(
  //               SubUVParticle->vertexSubUVBuffer, SubUVParticle->numTextVertices,
  //               SubUVParticle->indexTextureBuffer, SubUVParticle->numIndices, SubUVParticle->Texture->TextureSRV, SubUVParticle->Texture->SamplerState
  //           );
  //       }
  //       else if (UText* Text = Cast<UText>(BillboardComp))
  //       {
  //           UEditorEngine::renderer.RenderTextPrimitive(
  //               Text->vertexTextBuffer, Text->numTextVertices,
  //               Text->Texture->TextureSRV, Text->Texture->SamplerState
  //           );
  //       }
  //       else
  //       {
  //           RenderTexturePrimitive(
  //               BillboardComp->vertexTextureBuffer, BillboardComp->numVertices,
  //               BillboardComp->indexTextureBuffer, BillboardComp->numIndices, BillboardComp->Texture->TextureSRV, BillboardComp->Texture->SamplerState
  //           );
  //       }
  //   }
  //   PrepareShader();
//}

// void FRenderer::RenderPostProcess(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport, std::shared_ptr<FEditorViewportClient> CurrentViewport)
// {
    //
    // RenderDebugDepth(ActiveViewport);
    // if (ActiveViewport->GetShowFlag() & static_cast<uint64>(EEngineShowFlags::SF_Fog)) 
    // {
    //     RenderHeightFog(ActiveViewport, CurrentViewport);
    // }
//}

//void FRenderer::RenderDebugDepth(std::shared_ptr<FEditorViewportClient> ActiveViewport)
//{
    // // 현재 뷰포트의 뷰모드가 Depth 인지 확인
    // if (ActiveViewport->GetViewMode() != EViewModeIndex::VMI_Depth)
    // {
    //     return;
    // }
    //
    // Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // Graphics->DeviceContext->CopyResource(Graphics->DepthCopyTexture, Graphics->DepthStencilBuffer);
    //
    //
    // Graphics->DeviceContext->PSSetSamplers(0, 1, &DebugDepthSRVSampler);
    // Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->DepthCopySRV);
    //
    // Graphics->DeviceContext->VSSetShader(DebugDepthVertexShader, nullptr, 0);
    // Graphics->DeviceContext->PSSetShader(DebugDepthPixelShader, nullptr, 0);
    //
    // FViewportConstants ViewportConstants;
    // ViewportConstants.ViewportWidth = ActiveViewport->Viewport->GetViewport().Width / Graphics->screenWidth;
    // ViewportConstants.ViewportHeight = ActiveViewport->Viewport->GetViewport().Height/ Graphics->screenHeight;
    // ViewportConstants.ViewportOffsetX = ActiveViewport->Viewport->GetViewport().TopLeftX/ Graphics->screenWidth;
    // ViewportConstants.ViewportOffsetY = ActiveViewport->Viewport->GetViewport().TopLeftY/ Graphics->screenHeight;
    //
    // D3D11_MAPPED_SUBRESOURCE mappedResource;
    // Graphics->DeviceContext->Map(ViewportConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    // memcpy(mappedResource.pData, &ViewportConstants, sizeof(FViewportConstants));
    // Graphics->DeviceContext->Unmap(ViewportConstantBuffer, 0);
    //
    // // 렌더링 시 샘플러 설정
    // Graphics->DeviceContext->VSSetConstantBuffers(0, 1, &CameraConstantBuffer);
    // Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &CameraConstantBuffer);
    // Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &ViewportConstantBuffer);
    //
    // Graphics->DeviceContext->Draw(4, 0);
//}

// void FRenderer::RenderLight(UWorld* World, std::shared_ptr<FEditorViewportClient> ActiveViewport)
// {
//     // 라이트 오브젝트들 모아서 Constant 업데이트 해야함
//     ConstantBufferUpdater.UpdateLightConstant(LightingBuffer, LightObjs);
// }

#pragma endregion Render

void FRenderer::SetViewMode(const EViewModeIndex evi)
{
    switch (evi)
    {
    case EViewModeIndex::VMI_Lit:
        CurrentRasterizerState = ERasterizerState::SolidBack;
        //TODO : Light 받는 거 
        //ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 1);
        break;
    case EViewModeIndex::VMI_Wireframe:
        CurrentRasterizerState = ERasterizerState::WireFrame;
    case EViewModeIndex::VMI_Unlit:
        CurrentRasterizerState = ERasterizerState::SolidBack;
        //TODO : Light 안 받는 거 
        //ConstantBufferUpdater.UpdateLitUnlitConstant(FlagBuffer, 0);
        break;
    default:
        CurrentRasterizerState = ERasterizerState::SolidBack;
        break;
    }
}

void FRenderer::AddRenderObjectsToRenderPass(UWorld* InWorld) const
{
    StaticMeshRenderPass->AddRenderObjectsToRenderPass(InWorld);
    GizmoRenderPass->AddRenderObjectsToRenderPass(InWorld);
}

//void FRenderer::RenderHeightFog(std::shared_ptr<FEditorViewportClient> ActiveViewport, std::shared_ptr<FEditorViewportClient> CurrentViewport)
//{
    // // 활성화된 Height Fog 컴포넌트 찾기
    // UHeightFogComponent* HeightFogComp = nullptr;
    // for (const auto& comp: TObjectRange<UHeightFogComponent>() )
    // {
    //     HeightFogComp = comp;
    // }
    //
    // if (!HeightFogComp) return;
    // if (!HeightFogComp->bIsActive) return;
    //
    // Graphics->DeviceContext->VSSetShader(HeightFogVertexShader, nullptr, 0);
    // Graphics->DeviceContext->PSSetShader(HeightFogPixelShader, nullptr, 0);
    //
    // Graphics->DeviceContext->PSSetSamplers(0, 1, &DebugDepthSRVSampler);
    //
    // // Fog Constant buffer update
    // FHeightFogConstants fogParams;
    // fogParams.FogDensity = HeightFogComp->FogDensity;
    // fogParams.HeightFogStart = HeightFogComp->HeightFogStart;
    // fogParams.HeightFogEnd = HeightFogComp->HeightFogEnd;
    // fogParams.DistanceFogNear = HeightFogComp->DistanceFogNear;
    // fogParams.DistanceFogFar = HeightFogComp->DistanceFogFar;
    // fogParams.MaxOpacity = HeightFogComp->FogMaxOpacity;
    // fogParams.InscatteringColor = FLinearColor(
    //     HeightFogComp->FogInscatteringColor.R,
    //     HeightFogComp->FogInscatteringColor.G,
    //     HeightFogComp->FogInscatteringColor.B,
    //     HeightFogComp->FogInscatteringColor.A
    // );
    // fogParams.DirectionalInscatteringColor = FLinearColor(
    //     HeightFogComp->DirectionalInscatteringColor.R,
    //     HeightFogComp->DirectionalInscatteringColor.G,
    //     HeightFogComp->DirectionalInscatteringColor.B,
    //     HeightFogComp->DirectionalInscatteringColor.A
    // );
    // // 현재 0, 0, -1 방향의 가상의 Directional light로 고정함
    // // Unreal의 경우 Default Directional light(Sun)의 값을 따라가는 것으로 보임
    // // 실제 Directional light를 가져오도록 할 지 고려, 또한 Directional light이 2개 이상일 때는 어떻게 처리할지?
    // fogParams.DirectionalLightDirection = HeightFogComp->DirectionalLightDirection;
    // fogParams.DirectionalInscatteringExponent = HeightFogComp->DirectionalInscatteringExponent;
    // fogParams.DirectionalInscatteringStartDistance = HeightFogComp->DirectionalInscatteringStartDistance;
    // fogParams.IsExponential = HeightFogComp->bIsExponential ? 1 : 0;
    //
    // D3D11_MAPPED_SUBRESOURCE mappedResource;
    // Graphics->DeviceContext->Map(FogConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    // memcpy(mappedResource.pData, &fogParams, sizeof(FHeightFogConstants));
    // Graphics->DeviceContext->Unmap(FogConstantBuffer, 0);
    //
    // FViewportConstants ViewportConstants;
    // ViewportConstants.ViewportWidth = CurrentViewport->Viewport->GetViewport().Width / Graphics->screenWidth;
    // ViewportConstants.ViewportHeight = CurrentViewport->Viewport->GetViewport().Height / Graphics->screenHeight;
    // ViewportConstants.ViewportOffsetX = CurrentViewport->Viewport->GetViewport().TopLeftX / Graphics->screenWidth;
    // ViewportConstants.ViewportOffsetY = CurrentViewport->Viewport->GetViewport().TopLeftY / Graphics->screenHeight;
    //
    // D3D11_MAPPED_SUBRESOURCE viewportMappedResource;
    // Graphics->DeviceContext->Map(ViewportConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &viewportMappedResource);
    // memcpy(viewportMappedResource.pData, &ViewportConstants, sizeof(FViewportConstants));
    // Graphics->DeviceContext->Unmap(ViewportConstantBuffer, 0);
    //
    // // 백퍼버 복사
    // Graphics->CreateSceneColorResources();
    // Graphics->DeviceContext->CopyResource(Graphics->DepthCopyTexture, Graphics->DepthStencilBuffer);
    // // 겹치지 않도록 하기 위해 임시로 Slot를 현재 사용하지 않는 슬롯으로 지정함
    // Graphics->DeviceContext->PSSetConstantBuffers(0, 1, &CameraConstantBuffer);
    // Graphics->DeviceContext->PSSetConstantBuffers(6, 1, &FogConstantBuffer);
    // Graphics->DeviceContext->PSSetConstantBuffers(1, 1, &ViewportConstantBuffer);
    // Graphics->DeviceContext->PSSetShaderResources(5, 1, &Graphics->SceneColorSRV);
    // Graphics->DeviceContext->PSSetShaderResources(0, 1, &Graphics->DepthCopySRV);
    //
    // Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    // Graphics->DeviceContext->Draw(4, 0);
//}

void FRenderer::MappingVSPSInputLayout(const FName InShaderProgramName, FName VSName, FName PSName, ID3D11InputLayout* InInputLayout)
{
    ShaderPrograms.Add(InShaderProgramName, std::make_shared<FShaderProgram>(VSName, PSName, InInputLayout));
}

void FRenderer::MappingVSPSCBSlot(const FName InShaderName, const TMap<FShaderConstantKey, uint32>& MappedConstants)
{
    ShaderConstantNameAndSlots.Add(InShaderName, MappedConstants);
}

void FRenderer::MappingVBTopology(const FName InObjectName, const FName InVBName, const uint32 InStride, const uint32 InNumVertices, D3D11_PRIMITIVE_TOPOLOGY InTopology)
{
    if (VBIBTopologyMappings.Contains(InObjectName) == false)
    {
        VBIBTopologyMappings[InObjectName] = std::make_shared<FVBIBTopologyMapping>();
    }
    VBIBTopologyMappings[InObjectName]->MappingVertexBuffer(InVBName, InStride, InNumVertices, InTopology);
}

void FRenderer::MappingIB(const FName InObjectName, const FName InIBName, const uint32 InNumIndices)
{
    if (VBIBTopologyMappings.Contains(InObjectName) == false)
    {
        VBIBTopologyMappings[InObjectName] = std::make_shared<FVBIBTopologyMapping>();
    }
    VBIBTopologyMappings[InObjectName]->MappingIndexBuffer(InIBName, InNumIndices);
}
