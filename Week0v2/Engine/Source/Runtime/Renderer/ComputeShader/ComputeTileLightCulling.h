#pragma once
#include "Container/Array.h"
#include "Renderer/RenderPass/FBaseRenderPass.h"
#include "D3D11RHI/GraphicDevice.h"

class ULightComponentBase;
class USkySphereComponent;
struct FObjMaterialInfo;
struct FMatrix;
class UStaticMeshComponent;

class FComputeTileLightCulling
{
public:
    explicit FComputeTileLightCulling(const FName& InShaderName)
    {}

    virtual ~FComputeTileLightCulling() {}
    void AddRenderObjectsToRenderPass(UWorld* InWorld);
    void Dispatch(std::shared_ptr<FViewportClient> InViewportClient);
    void UpdateLightConstants();

    inline static int XTileCount = 16;
    inline static int YTileCount = 16;
    inline static int MaxPointLightCount = 16;
    inline static int MaxSpotLightCount = 8;
    
private:
    void UpdateComputeConstants(const std::shared_ptr<FViewportClient> InViewportClient);

    int PreviousLightCount = 0;
    TArray<ULightComponentBase*> LightComponents;
};