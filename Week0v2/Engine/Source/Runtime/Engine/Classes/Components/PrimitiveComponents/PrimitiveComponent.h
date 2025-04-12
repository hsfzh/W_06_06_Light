#pragma once
#include "Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h"
#include "Serialization/Archive.h"

struct FPrimitiveComponentInfo : FSceneComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FPrimitiveComponentInfo);

    FVector ComponentVelocity;
    FName VBIBTopologyMappingName;

    FPrimitiveComponentInfo()
        : FSceneComponentInfo()
        , ComponentVelocity(0.0f)
        , VBIBTopologyMappingName(TEXT("None"))
    {
        InfoType = TEXT("FPrimitiveComponentInfo");
        ComponentType = TEXT("UPrimitiveComponent");
    }

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FSceneComponentInfo::Copy(Other);
        FPrimitiveComponentInfo& OtherPrimitive = static_cast<FPrimitiveComponentInfo&>(Other);
        OtherPrimitive.ComponentVelocity = ComponentVelocity;
        OtherPrimitive.VBIBTopologyMappingName = VBIBTopologyMappingName;
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FSceneComponentInfo::Serialize(ar);
        ar << ComponentVelocity << VBIBTopologyMappingName;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FSceneComponentInfo::Deserialize(ar);
        ar >> ComponentVelocity >> VBIBTopologyMappingName;
    }
};

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent();
    UPrimitiveComponent(const UPrimitiveComponent& Other);
    virtual ~UPrimitiveComponent() override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance);
    bool IntersectRayTriangle(
        const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance
    );
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

    bool MoveComponent(const FVector& Delta) override;
    FVector ComponentVelocity;

public:
    virtual std::shared_ptr<FActorComponentInfo> GetActorComponentInfo() override;
    virtual void LoadAndConstruct(const FActorComponentInfo& Info);

public:
    FName GetVBIBTopologyMappingName() const { return VBIBTopologyMappingName; }
protected:
    FName VBIBTopologyMappingName;
};

