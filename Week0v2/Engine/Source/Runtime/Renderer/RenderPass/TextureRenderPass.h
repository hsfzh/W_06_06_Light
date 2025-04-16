#pragma once
#include "FBaseRenderPass.h"
#include "Container/Array.h"

class UParticleSubUVComp;

class FTextureRenderPass : public FBaseRenderPass
{
public:
    FTextureRenderPass(const FName& InShaderName)
        : FBaseRenderPass(InShaderName) {}
    ~FTextureRenderPass() override;
    
    void AddRenderObjectsToRenderPass(UWorld* InLevel) override;
    void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;
    void ClearRenderObjects() override;

private:
    TArray<UParticleSubUVComp*> SubUVComponents;
};
 