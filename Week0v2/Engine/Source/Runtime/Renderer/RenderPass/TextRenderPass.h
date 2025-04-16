#pragma once
#include "FBaseRenderPass.h"
#include "Container/Array.h"

class UTextComponent;

class FTextRenderPass : public FBaseRenderPass
{
public:
    explicit FTextRenderPass(const FName& InShaderName)
        : FBaseRenderPass(InShaderName) {}
    
    ~FTextRenderPass() override;
    
    void AddRenderObjectsToRenderPass(UWorld* InLevel) override;
    void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
    void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;
    void ClearRenderObjects() override;
private:
    TArray<UTextComponent*> TextComponents;
};
