#include "FogComponent.h"

UFogComponent::UFogComponent()
{
    Color = FLinearColor(0.8f, 0.9f, 1.0f); //푸른빛
    Start = 0.0f;
    End = 100.f;
    Density = 0.5f;
    HeightFallOff = 0.02f;
    BaseHeight = 5.f;
    ScatteringIntensity = 10.f; // 추가: 빛 산란 강도 [4]
    LightShaftDensity = 0.02f; // 추가: 광선 밀도 [4]
    bIsHeight = true;
}

void UFogComponent::LoadAndConstruct(const FActorComponentInfo& Info)
{
    Super::LoadAndConstruct(Info);
    const auto& HeightFogInfo = static_cast<const FFogComponentInfo&>(Info);
    Color = HeightFogInfo.Color;
    Start = HeightFogInfo.Start;
    End = HeightFogInfo.End;
    Density = HeightFogInfo.Density;
    HeightFallOff = HeightFogInfo.HeightFallOff;
    BaseHeight = HeightFogInfo.BaseHeight;
    ScatteringIntensity = HeightFogInfo.ScatteringIntensity;
    LightShaftDensity = HeightFogInfo.LightShaftDensity;
    bIsHeight = HeightFogInfo.bIsHeight;
}

std::shared_ptr<FActorComponentInfo> UFogComponent::GetActorComponentInfo()
{
    auto Info = std::make_shared<FFogComponentInfo>();
    Super::GetActorComponentInfo()->Copy(*Info);
    Info->Color = Color;
    Info->Start = Start;
    Info->End = End;
    Info->Density = Density;
    Info->HeightFallOff = HeightFallOff;
    Info->BaseHeight = BaseHeight;
    Info->ScatteringIntensity = ScatteringIntensity;
    Info->LightShaftDensity = LightShaftDensity;
    Info->bIsHeight = bIsHeight;
    return Info;
}
