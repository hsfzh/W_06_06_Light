#pragma once
#include "FBaseRenderPass.h"

class FLineBatchRenderPass : public FBaseRenderPass
{
public:
    explicit FLineBatchRenderPass(const FName& InShaderName);

    void AddRenderObjectsToRenderPass(UWorld* InLevel) override;
    void Execute(std::shared_ptr<FViewportClient> InViewportClient) override;

    void Prepare(std::shared_ptr<FViewportClient> InViewportClient) override;
private:
    static void UpdateBatchResources();

    FName VBIBTopologyMappingName;
};
