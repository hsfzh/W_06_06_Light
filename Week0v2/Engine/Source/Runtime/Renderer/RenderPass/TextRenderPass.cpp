#include "TextRenderPass.h"

#include "EditorEngine.h"
#include "LaunchEngineLoop.h"
#include "Components/SceneComponent.h"
#include "Components/UTextComponent.h"
#include "D3D11RHI/CBStructDefine.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"
#include "Renderer/Renderer.h"
#include "Renderer/VBIBTopologyMapping.h"
#include "UnrealEd/EditorViewportClient.h"

FTextRenderPass::~FTextRenderPass()
{
}

void FTextRenderPass::AddRenderObjectsToRenderPass(UWorld* InLevel)
{
    TArray<USceneComponent*> Ss;
    for (const auto& A : InLevel->GetActors())
    {
        Ss.Add(A->GetRootComponent());
    }

    for (const auto iter : Ss)
    {
        if (UTextComponent* TextRenderComp = Cast<UTextComponent>(iter))
        {
            TextComponents.Add(TextRenderComp);
        }
    }
}

void FTextRenderPass::Prepare(std::shared_ptr<FViewportClient> InViewportClient)
{
    FBaseRenderPass::Prepare(InViewportClient);
    
    const FRenderer& Renderer = GEngine->renderer;
    const FGraphicsDevice& Graphics = GEngine->graphicDevice;
    
    Graphics.DeviceContext->RSSetState(Renderer.GetCurrentRasterizerState());
    ID3D11SamplerState* linearSampler = Renderer.GetSamplerState(ESamplerType::Linear);
    Graphics.DeviceContext->PSSetSamplers(static_cast<uint32>(ESamplerType::Linear), 1, &linearSampler);
}

void FTextRenderPass::Execute(std::shared_ptr<FViewportClient> InViewportClient)
{
    FRenderer& Renderer = GEngine->renderer;
    FGraphicsDevice& Graphics = GEngine->graphicDevice;
    FRenderResourceManager* renderResourceManager = GEngine->renderer.GetResourceManager();

    FMatrix View = FMatrix::Identity;
    FMatrix Proj = FMatrix::Identity;

    std::shared_ptr<FEditorViewportClient> curEditorViewportClient = std::dynamic_pointer_cast<FEditorViewportClient>(InViewportClient);
    if (curEditorViewportClient != nullptr)
    {
        View = curEditorViewportClient->GetViewMatrix();
        Proj = curEditorViewportClient->GetProjectionMatrix();
    }

    for (const auto TextComponent : TextComponents)
    {
        FSubUVConstant SubUVConstant;
        SubUVConstant.indexU = TextComponent->GetFinalIndexU();
        SubUVConstant.indexV = TextComponent->GetFinalIndexV();
        renderResourceManager->UpdateConstantBuffer(TEXT("FSubUVConstant"), &SubUVConstant);

        FMatrix Model;
        if (TextComponent->IsBillboard())
        {
            Model = TextComponent->CreateBillboardMatrix();
        }
        else
        {
            Model = JungleMath::CreateModelMatrix(TextComponent->GetComponentLocation(), TextComponent->GetComponentRotation(), TextComponent->GetComponentScale());
        }

        FMatrix VP = View * Proj;
        
        FConstants Constant;
        Constant.MVP = Model * VP;
        renderResourceManager->UpdateConstantBuffer(TEXT("FConstants"), &Constant);

        const std::shared_ptr<FVBIBTopologyMapping> VBIBTopMappingInfo = Renderer.GetVBIBTopologyMapping(TextComponent->GetVBIBTopologyMappingName());
        VBIBTopMappingInfo->Bind();

        Graphics.DeviceContext->PSSetShaderResources(0,1, &TextComponent->GetTexture()->TextureSRV);
        Graphics.DeviceContext->OMSetDepthStencilState(Renderer.GetDepthStencilState(EDepthStencilState::DepthNone), 0);

        Graphics.DeviceContext->Draw(VBIBTopMappingInfo->GetNumVertices(), 0);
    }
}

void FTextRenderPass::ClearRenderObjects()
{
    FBaseRenderPass::ClearRenderObjects();
    TextComponents.Empty();
}
