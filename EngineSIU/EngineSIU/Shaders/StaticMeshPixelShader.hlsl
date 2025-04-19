
#include "ShaderRegisters.hlsl"

SamplerState DiffuseSampler : register(s0);
SamplerState NormalSampler : register(s1);
SamplerComparisonState ShadowPCF: register(s2);

Texture2D DiffuseTexture : register(t0);
Texture2D NormalTexture : register(t1);
// TextureCube<float> ShadowMap : register(t2);
Texture2D ShadowMap : register(t2);


cbuffer MaterialConstants : register(b1)
{
    FMaterial Material;
}

cbuffer FlagConstants : register(b2)
{
    bool IsLit;
    float3 flagPad0;
}

cbuffer SubMeshConstants : register(b3)
{
    bool IsSelectedSubMesh;
    float3 SubMeshPad0;
}

cbuffer TextureConstants : register(b4)
{
    float2 UVOffset;
    float2 TexturePad0;
}
cbuffer LightMatrix : register(b5)
{
    row_major matrix LightViewMat;
    row_major matrix LightProjectMat;
};

#include "Light.hlsl"
struct FSputLightInfo
{
    float4 LightColor;

    float3 Position;
    float Radius;
    
    int Type;
    float Intensity;
    float Attenuation;
    float Padding;
};

float ShadowOcclusion(float3 worldPos) {
    float3 toLight = worldPos - PointLights[0].Position;
    float currDist = length(toLight);
    float3 dir = normalize(toLight);
    // PCF 비교 샘플
    return ShadowMap.SampleCmpLevelZero(ShadowPCF, dir, currDist - 0.001);
}
static const float SHADOW_BIAS = 0.001;
float ShadowOcclusion_Spot(float3 worldPos)
{
    // 1) 월드 좌표 → 라이트 뷰 공간
    float4 lightViewPos = mul(float4(worldPos, 1.0), LightViewMat);
    
    // 2) 라이트 뷰 공간 → 라이트 클립 공간
    float4 lightClipPos = mul(lightViewPos, LightProjectMat);
    
    // 3) 퍼스펙티브 분할
    lightClipPos.xyz /= lightClipPos.w;
    
    // 4) NDC [-1,1] → UV [0,1]
    float2 uv = lightClipPos.xy * 0.5 + 0.5;
    
    // 5) UV 범위 밖은 그림자 없음 처리
    if (uv.x < 0.0 || uv.x > 1.0 ||
        uv.y < 0.0 || uv.y > 1.0)
    {
        return 1.0;
    }
    
    // 6) 깊이 비교 (PCF)
    float shadow = ShadowMap.SampleCmpLevelZero(
        ShadowPCF,
        uv,
        lightClipPos.z - SHADOW_BIAS
    );
    
    return shadow;
}
float4 mainPS(PS_INPUT_StaticMesh Input) : SV_Target
{
    float4 FinalColor = float4(0.f, 0.f, 0.f, 1.f);

    // Diffuse
    float3 DiffuseColor = Material.DiffuseColor;
    if (Material.TextureFlag & (1 << 1))
    {
        DiffuseColor = DiffuseTexture.Sample(DiffuseSampler, Input.UV).rgb;
        DiffuseColor = SRGBToLinear(DiffuseColor);
    }

    // Normal
    float3 WorldNormal = Input.WorldNormal;
    if (Material.TextureFlag & (1 << 2))
    {
        float3 Normal = NormalTexture.Sample(NormalSampler, Input.UV).rgb;
        Normal = normalize(2.f * Normal - 1.f);
        WorldNormal = normalize(mul(mul(Normal, Input.TBN), (float3x3) InverseTransposedWorld));
    }

    // (1) 현재 픽셀이 속한 타일 계산 (input.position = 화면 픽셀좌표계)
    uint2 pixelCoord = uint2(Input.Position.xy);
    uint2 tileCoord = pixelCoord / TileSize; // 각 성분별 나눔
    uint tilesX = ScreenSize.x / TileSize.x; // 한 행에 존재하는 타일 수
    uint flatTileIndex = tileCoord.x + tileCoord.y * tilesX;
    
    // (2) 현재 타일의 조명 정보 읽기
    LightPerTiles tileLights = gLightPerTiles[flatTileIndex];
    
    // 조명 기여 누적 (예시: 단순히 조명 색상을 더함)
    float3 lightingAccum = float3(0, 0, 0);
    for (uint i = 0; i < tileLights.NumLights; ++i)
    {
        // tileLights.Indices[i] 는 전역 조명 인덱스
        uint gPointLightIndex = tileLights.Indices[i];
        //FPointLightInfo light = gPointLights[gPointLightIndex];
        
        float4 lightContribution = PointLight(gPointLightIndex, Input.WorldPosition, 
            normalize(Input.WorldNormal),
            Input.WorldViewPosition, DiffuseColor.rgb
        );
        lightingAccum += lightContribution.rgb;
    }
    //lightingAccum += Ambient[0].AmbientColor.rgb;
    float NdotL = saturate(dot(normalize(Input.WorldNormal), normalize(PointLights[0].Position - Input.WorldPosition)));
    float shadow = ShadowOcclusion_Spot(Input.WorldPosition);
    // Lighting
    if (IsLit)
    {
#ifdef LIGHTING_MODEL_GOURAUD
        FinalColor = float4(Input.Color.rgb * DiffuseColor, 1.0);
#else
        float3 LitColor = Lighting(Input.WorldPosition, WorldNormal, Input.WorldViewPosition, DiffuseColor).rgb;
        LitColor = LitColor * NdotL * shadow;
        // 디버깅용 ---- PointLight 전역 배열에 대한 라이팅 테스팅
        //LitColor = float3(0, 0, 0);
        //LitColor += lightingAccum;
        // ------------------------------
        
        FinalColor = float4(LitColor, 1);
#endif
    }
    else
    {
        FinalColor = float4(DiffuseColor, 1);
    }
    
    if (bIsSelected)
    {
        FinalColor += float4(0.01, 0.01, 0.0, 1);
    }
    
    return FinalColor;
}
