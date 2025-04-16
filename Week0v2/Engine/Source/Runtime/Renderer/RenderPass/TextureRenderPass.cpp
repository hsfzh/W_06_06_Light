#include "TextureRenderPass.h"

#include "EditorEngine.h"
#include "Components/SceneComponent.h"
#include "Components/UParticleSubUVComp.h"
#include "D3D11RHI/CBStructDefine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Renderer/VBIBTopologyMapping.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

FTextureRenderPass::~FTextureRenderPass()
{
}

void FTextureRenderPass::AddRenderObjectsToRenderPass(UWorld* InLevel)
{
    TArray<USceneComponent*> Ss;
    for (const auto& A : InLevel->GetActors())
    {
        Ss.Add(A->GetRootComponent());
    }

    for (const auto iter : Ss)
    {
        if (UParticleSubUVComp* pBillboardComp = Cast<UParticleSubUVComp>(iter))
        {
            SubUVComponents.Add(pBillboardComp);
        }
    }
}

void FTextureRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewportClient)
{
    FBaseRenderPass::Prepare(InViewportClient);
        
    FRenderer& Renderer = GEngine->renderer;
    FGraphicsDevice& Graphics = GEngine->graphicDevice;

    Graphics.DeviceContext->RSSetState(Renderer.GetCurrentRasterizerState());
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
}

void FTextureRenderPass::Execute(std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngine->renderer;
    FGraphicsDevice& Graphics = GEngine->graphicDevice;
    FRenderResourceManager* renderResourceManager = GEngine->renderer.GetResourceManager();

    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;

    const std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }

    for (UParticleSubUVComp* BillboardComponent : SubUVComponents)
    {
        FMatrix Model = BillboardComponent->CreateBillboardMatrix();
        FMatrix VP = View * Proj;
        
        FConstants Constant;
        Constant.MVP = Model * VP;
        renderResourceManager->UpdateConstantBuffer(TEXT("FConstants"), &Constant);
     
        FSubUVConstant SubUVConstant;
        SubUVConstant.indexU = BillboardComponent->GetUVOffset().x;
        SubUVConstant.indexV = BillboardComponent->GetUVOffset().y;
        SubUVConstant.ScaleU = BillboardComponent->GetUVScale().x;
        SubUVConstant.ScaleV = BillboardComponent->GetUVScale().y;
        
        renderResourceManager->UpdateConstantBuffer(TEXT("FSubUVConstant"), &SubUVConstant);
 
        const std::shared_ptr<FVBIBTopologyMapping> VBIBTopMappingInfo = Renderer.GetVBIBTopologyMapping(BillboardComponent->GetVBIBTopologyMappingName());
        VBIBTopMappingInfo->Bind();

        Graphics.DeviceContext->PSSetShaderResources(0, 1, &(BillboardComponent->Texture->TextureSRV));
        
        Graphics.DeviceContext->DrawIndexed(VBIBTopMappingInfo->GetNumIndices(), 0, 0);
    }
}

void FTextureRenderPass::ClearRenderObjects()
{
    FBaseRenderPass::ClearRenderObjects();
    SubUVComponents.Empty();
}
