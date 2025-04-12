#pragma once
#include "Define.h"
#include "Components/SceneComponent.h"
#include "UObject/ObjectMacros.h"

class UPrimitiveComponent;
struct FTexture;
class UBillboardComponent;

struct FLightComponentBaseInfo : public FSceneComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FSceneComponentInfo);

    FVector4 Color;

    float Intensity;

    // ctor
    FLightComponentBaseInfo()
        : FSceneComponentInfo()
        , Color(FVector4(1, 1, 1, 1))
        , Intensity(1.0f)
    {
        InfoType = TEXT("FLightComponentBaseInfo");
        ComponentType = TEXT("ULightComponentBase");
    }

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FSceneComponentInfo::Copy(Other);
        FLightComponentBaseInfo& LightInfo = static_cast<FLightComponentBaseInfo&>(Other);
        LightInfo.Color = Color;
        LightInfo.Intensity = Intensity;
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FSceneComponentInfo::Serialize(ar);
        ar << Color << Intensity;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FSceneComponentInfo::Deserialize(ar);
        ar >> Color >> Intensity;
    }

};

class ULightComponentBase : public USceneComponent
{
    DECLARE_CLASS(ULightComponentBase, USceneComponent)

public:
    ULightComponentBase();
    ULightComponentBase(const ULightComponentBase& Other);
    virtual ~ULightComponentBase() override;

    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    virtual void InitializeComponent() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void InitializeLight();
    void SetColor(FVector4 newColor);
    FVector4 GetColor() const;

protected:
    FVector4 LightColor = { 1, 1, 1, 1 }; // RGBA
    float Intensity = 1.0f;
public:
    FVector4 GetLightColor() const {return LightColor;}
    float GetIntensity() const { return Intensity; }
    void SetIntensity(const float InIntensity) { Intensity = InIntensity; }
    void SetLightColor(const FVector4 InColor) { LightColor = InColor; }

public:
    // duplictae
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

public:
    virtual std::shared_ptr<FActorComponentInfo> GetActorComponentInfo() override;
    virtual void LoadAndConstruct(const FActorComponentInfo& Info) override;

#pragma region OnlyEditor
    std::shared_ptr<FTexture> StaticEditorTexture;
};

struct FLightComponentInfo : public FLightComponentBaseInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FLightComponentInfo);

    float Temperature;
    float MaxDrawDistance;
    float MaxDistancceFadeRange;

    bool bUseTemperature;

    float SpecularScale;
    float DiffuseScale;
    
    // ctor
    FLightComponentInfo()
        : FLightComponentBaseInfo()
        , Temperature(0.f), MaxDrawDistance(0.f), MaxDistancceFadeRange(0.f), bUseTemperature(false), SpecularScale(0.f), DiffuseScale(0.f)
    {
        InfoType = TEXT("FLightComponentInfo");
        ComponentType = TEXT("ULightComponent");
    }

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FLightComponentBaseInfo::Copy(Other);
        FLightComponentInfo& LightInfo = static_cast<FLightComponentInfo&>(Other);
        LightInfo.Temperature = Temperature;
        LightInfo.MaxDrawDistance = MaxDrawDistance;
        LightInfo.MaxDistancceFadeRange = MaxDistancceFadeRange;
        LightInfo.bUseTemperature = bUseTemperature;
        LightInfo.SpecularScale = SpecularScale;
        LightInfo.DiffuseScale = DiffuseScale;
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FLightComponentBaseInfo::Serialize(ar);
        ar << Temperature << MaxDrawDistance << MaxDistancceFadeRange << bUseTemperature << bUseTemperature << SpecularScale << DiffuseScale;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FLightComponentBaseInfo::Deserialize(ar);
        ar >> Temperature >> MaxDrawDistance >> MaxDistancceFadeRange >> bUseTemperature >> bUseTemperature >> SpecularScale >> DiffuseScale;
    }

};

class ULightComponent : public ULightComponentBase
{
    DECLARE_CLASS(ULightComponent, ULightComponentBase)

public:
    ULightComponent();
    ULightComponent(const ULightComponent& Other);
    virtual ~ULightComponent() override;

    bool AffectsPrimitive(UPrimitiveComponent* Primitive);
    virtual bool AffectsBounds(FBoundingBox InBounds);

    void SetTemperature(const float InTemperature) { Temperature = InTemperature; }
    void SetSpecularScale(float InSpecularScale) { SpecularScale = InSpecularScale; }
    void SetDiffuseScale(float InDiffuseScale) { DiffuseScale = InDiffuseScale; }

    FVector GetDirection();

    float Temperature;
    float MaxDrawDistance;
    float MaxDistancceFadeRange;

    bool bUseTemperature;

    float SpecularScale;
    float DiffuseScale;
};
