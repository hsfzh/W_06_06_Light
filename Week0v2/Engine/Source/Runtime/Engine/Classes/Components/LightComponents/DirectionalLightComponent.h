#pragma once
#include "LightComponent.h"

struct FDirectionalLightComponentInfo : public FLightComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FDirectionalLightComponentInfo);

    FVector Direction;
    float LigthSourceAngle;
    float LigthSourceSoftAngle;

    FDirectionalLightComponentInfo()
        : FLightComponentInfo()
        , Direction(FVector(0.0f, 0.0f, -1.0f))
        , LigthSourceAngle(0.5357f)
        , LigthSourceSoftAngle(0.0f)
    {
        InfoType = TEXT("FDirectionalLightComponentInfo");
        ComponentType = TEXT("UDirectionalLightComponent");
    }

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FLightComponentInfo::Copy(Other);
        FDirectionalLightComponentInfo& DirectionalLightInfo = static_cast<FDirectionalLightComponentInfo&>(Other);
        DirectionalLightInfo.Direction = Direction;
    }
    virtual void Serialize(FArchive& ar) const override
    {
        FLightComponentInfo::Serialize(ar);
        ar << Direction;
    }
    virtual void Deserialize(FArchive& ar) override
    {
        FLightComponentInfo::Deserialize(ar);
        ar >> Direction;
    }
};

class UDirectionalLightComponent : public ULightComponent
{
    DECLARE_CLASS(UDirectionalLightComponent, ULightComponent)
public:
    UDirectionalLightComponent();
    UDirectionalLightComponent(const UDirectionalLightComponent& Other);
    virtual ~UDirectionalLightComponent() override = default;
private:
    FVector Direction = FVector(0.0f, 0.0f, -1.0f);
    float LigthSourceAngle = 0.5357f;
    float LigthSourceSoftAngle = 0.0f;
public:
    FVector GetDirection() const { return Direction; }
    void SetDirection(const FVector InDirection) { Direction = InDirection; }

public:
    virtual UObject* Duplicate() const override;
    virtual void DuplicateSubObjects(const UObject* Source) override;
    virtual void PostDuplicate() override;

public:
    virtual std::shared_ptr<FActorComponentInfo> GetActorComponentInfo() override;
    virtual void LoadAndConstruct(const FActorComponentInfo& Info) override;
};


