#pragma once
#include "PointLightComponent.h"

struct FSpotLightComponentInfo : public FPointLightComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FSpotLightComponentInfo);

    float Radius;
    float AttenuationFalloff;

    FSpotLightComponentInfo()
        : FPointLightComponentInfo()
        , Radius(1.0f)
        , AttenuationFalloff(0.01f)
    {
        InfoType = TEXT("FSpotLightComponentInfo");
        ComponentType = TEXT("USpotLightComponent");
    } 

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FPointLightComponentInfo::Copy(Other);
        FSpotLightComponentInfo& PointLightInfo = static_cast<FSpotLightComponentInfo&>(Other);
        PointLightInfo.Radius = Radius;
        PointLightInfo.AttenuationFalloff = AttenuationFalloff;
    }

    virtual void Serialize(FArchive& ar) const override
    {
        FPointLightComponentInfo::Serialize(ar);
        ar << Radius << AttenuationFalloff;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FPointLightComponentInfo::Deserialize(ar);
        ar >> Radius >> AttenuationFalloff;
    }
};


class USpotLightComponent : public UPointLightComponent
{
    DECLARE_CLASS(USpotLightComponent, UPointLightComponent)
public:
    USpotLightComponent();
    USpotLightComponent(const USpotLightComponent& Other);
    virtual ~USpotLightComponent() override = default;

    float InnerConeAngle;
    float OuterConeAngle;

    void SetInnerConeAngle(const float InInnerConeAngle) { InnerConeAngle = InInnerConeAngle;}
    void SetOuterConeAngle(const float InOuterConeAngle) { OuterConeAngle = InOuterConeAngle;}

    
};
