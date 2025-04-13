#include "ShaderHeaders/GSamplers.hlsli"
Texture2D Texture : register(t0);
Texture2D NormalTexture : register(t1);
Texture2D BumpTexture : register(t2);

cbuffer FMaterialConstants : register(b0)
{
    float3 DiffuseColor;
    float TransparencyScalar;
    float3 MatAmbientColor;
    float DensityScalar;
    float3 SpecularColor;
    float SpecularScalar;
    float3 EmissiveColor;
    float MaterialPad0;
}

struct PS_INPUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float3 worldPos : POSITION;
    float4 color : COLOR; // 전달할 색상
    float2 texcoord : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float3x3 TBN: TEXCOORD2;
};

struct PS_OUTPUT
{
    float4 color : SV_Target0;
};

PS_OUTPUT mainPS(PS_INPUT input)
{
    PS_OUTPUT output;
    // 기본 색상 추출  
    float4 baseColor = Texture.Sample(linearSampler, input.texcoord) + float4(DiffuseColor, 1.0);
    // 기본 텍스쳐 색상 추출
    // 고로 셰이딩에서는 색상 계산을 정점 셰이더에서 완료했기 때문에 
    // 픽셀 셰이더에서는 보간된 색상을 그대로 사용
    output.color = float4(input.color.rgb * baseColor.rgb, 1.0f); // 보간된 색상

    return output;
    
    
}