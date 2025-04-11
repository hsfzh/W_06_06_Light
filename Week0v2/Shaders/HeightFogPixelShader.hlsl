cbuffer CameraConstant : register(b0)
{
    matrix ViewMatrix;
    matrix ProjMatrix;
    matrix InvViewMatrix;
    matrix InvProjMatrix;
    
    float3 CameraPos;
    float NearPlane;
    float3 CameraForward;
    float FarPlane;
};

cbuffer ViewportInfo : register(b1)
{
    float2 ViewportSize;
    float2 ViewportOffset;
}

cbuffer FogParams : register(b6)
{    
    float3 FogColor;
    float FogDensity;
    float FogStart;
    float FogEnd;
    int bIsFogOn;
    float FogZPosition;
    float FogBaseHeight;
    float HeightFallOff;
    int bIsHeightFog;
    float ScatteringIntensity;  // 추가: 빛 산란 강도 [4]
    float LightShaftDensity;    // 추가: 광선 밀도 [4]
    float3 Padding1;
}

Texture2D SceneTexture : register(t5);
Texture2D DepthTexture : register(t0);
SamplerState SamplerLinear : register(s0);

// Screen space to World space
float3 ReconstructWorldPosition(float2 UV, float Depth)
{
    float ndcx = UV.x * 2.0 - 1.0;
    float ndcy = 1.0f - UV.y * 2.0;
    float4 clipPos = float4(ndcx, ndcy, Depth, 1.0f);
    
    float4 viewPos = mul(clipPos, InvProjMatrix);
    viewPos /= viewPos.w;
    
    float4 worldPos = mul(viewPos, InvViewMatrix);
    worldPos /= worldPos.w;
    return worldPos.xyz;
}

struct VS_OUT
{
    float4 position : SV_POSITION; // 변환된 화면 좌표
    float2 uv : TEXCOORD0; // UV 좌표
};

// 깊이 버퍼에서 월드 좌표 계산 함수
float3 ReconstructWorldPos(float2 uv, float depth)
{
    // 1. 클립 공간 좌표 계산
    float4 clipPos = float4(uv * 2.0 - 1.0, 1.0, 1.0);

    clipPos.y *= -1.0;
    
    // 2. 뷰/프로젝션 행렬 역변환 (엔진별 행렬 전달 필요)
    float4 worldPos = mul(clipPos, InvProjMatrix);
    worldPos /= worldPos.w;
    worldPos = mul(worldPos, InvViewMatrix);
    
    return worldPos.xyz;
}

float4 mainPS(VS_OUT input) : SV_TARGET
{
    float2 viewportUV = input.uv * ViewportSize + ViewportOffset;
    float4 sceneColor = SceneTexture.Sample(SamplerLinear, viewportUV);
    float depth = DepthTexture.Sample(SamplerLinear, viewportUV).r;
    
    float3 litColor = sceneColor.rgb;

    return float4(litColor.rgb, sceneColor.a);
    
    if (bIsFogOn)  
    {        
        // 포그 혼합 (거리 + 높이)          

        float3 worldPos = ReconstructWorldPos(input.uv, depth);
        
        float dist = distance(CameraPos, worldPos);
        // 거리 기반
        float fogRange = FogEnd - FogStart;
        float disFactor = saturate((dist - FogStart) / fogRange);

        float fogFactor = disFactor;
        
        if (bIsHeightFog)  
        {  
            float FogHeight = FogZPosition + FogBaseHeight;
        
            // 높이 기반 (지수 감쇠)
            float heightDiff = worldPos.z - FogHeight;
            float heightFactor = saturate(exp(-heightDiff * HeightFallOff)); // 0~1
            fogFactor = fogFactor * heightFactor ; //factor가 클수록 fogcolor에 가까워짐
        }

        litColor = lerp(litColor, FogColor, fogFactor * FogDensity); 
    }  

    return float4(litColor, 1.0);  
}
