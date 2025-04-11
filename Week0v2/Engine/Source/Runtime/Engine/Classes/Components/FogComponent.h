#pragma once
#include "PrimitiveComponent.h"
#include "Math/Color.h"
#include "Runtime/CoreUObject/UObject/ObjectMacros.h"

struct FFogComponentInfo : public FPrimitiveComponentInfo
{
    DECLARE_ACTORCOMPONENT_INFO(FFogComponentInfo);

    FLinearColor Color; //푸른빛
    float Start;
    float End;
    float Density;
    float HeightFallOff;
    float BaseHeight;
    float ScatteringIntensity;  // 추가: 빛 산란 강도 [4]
    float LightShaftDensity;    // 추가: 광선 밀도 [4]
    bool bIsHeight;

    FFogComponentInfo()
    : FPrimitiveComponentInfo()
    , Color(FLinearColor(0.8f, 0.9f, 1.0f))
    , Start(0.0f)
    , End(100.f)
    , Density(0.5f)
    , HeightFallOff(0.02f)
    , BaseHeight(5.f)
    , ScatteringIntensity(10.f)
    , LightShaftDensity(0.02f)
    , bIsHeight(true)
    {
        InfoType = TEXT("FHeightFogComponentInfo");
        ComponentType = TEXT("UHeightFogComponent");
    }

    virtual void Copy(FActorComponentInfo& Other) override
    {
        FPrimitiveComponentInfo::Copy(Other);
        FFogComponentInfo& FogInfo = static_cast<FFogComponentInfo&>(Other);
        FogInfo.Color = Color; //푸른빛
        FogInfo.Start = Start;
        FogInfo.End = End;
        FogInfo.Density = Density;
        FogInfo.HeightFallOff = HeightFallOff;
        FogInfo.BaseHeight = BaseHeight;
        FogInfo.ScatteringIntensity = ScatteringIntensity;  // 추가: 빛 산란 강도 [4]
        FogInfo.LightShaftDensity = LightShaftDensity;    // 추가: 광선 밀도 [4]
        FogInfo.bIsHeight = bIsHeight;

    }

    virtual void Serialize(FArchive& ar) const override
    {
        FPrimitiveComponentInfo::Serialize(ar);
        ar << Color << Start << End << Density << HeightFallOff
            << BaseHeight << ScatteringIntensity << LightShaftDensity
            << bIsHeight;
    }

    virtual void Deserialize(FArchive& ar) override
    {
        FPrimitiveComponentInfo::Deserialize(ar);
        ar >> Color >> Start >> End >> Density >> HeightFallOff
            >> BaseHeight >> ScatteringIntensity >> LightShaftDensity
            >> bIsHeight;
    }
};
class UFogComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UFogComponent, UPrimitiveComponent)
public:
    UFogComponent();
    FLinearColor Color; //푸른빛
    float Start;
    float End;
    float Density;
    float HeightFallOff;
    float BaseHeight;
    float ScatteringIntensity;  // 추가: 빛 산란 강도 [4]
    float LightShaftDensity;    // 추가: 광선 밀도 [4]
    bool bIsHeight;

public:
    virtual void LoadAndConstruct(const FActorComponentInfo& Info);
    virtual std::shared_ptr<FActorComponentInfo> GetActorComponentInfo() override;
};


