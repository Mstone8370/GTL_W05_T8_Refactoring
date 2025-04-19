#include "PointLightComponent.h"

#include "Math/JungleMath.h"
#include "UObject/Casts.h"

UPointLightComponent::UPointLightComponent()
{
    PointLightInfo.Position = GetWorldLocation();
    PointLightInfo.Radius = 30.f;

    PointLightInfo.LightColor = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);

    PointLightInfo.Intensity = 1000.f;
    PointLightInfo.Type = ELightType::POINT_LIGHT;
    PointLightInfo.Attenuation = 20.0f;
    CreateShadowMapResources();
}

UPointLightComponent::~UPointLightComponent()
{
}

UObject* UPointLightComponent::Duplicate(UObject* InOuter)
{

    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    if (NewComponent)
    {
        NewComponent->PointLightInfo = PointLightInfo;
    }
    return NewComponent;
}

void UPointLightComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("Radius"), FString::Printf(TEXT("%f"), PointLightInfo.Radius));
    OutProperties.Add(TEXT("LightColor"), FString::Printf(TEXT("%s"), *PointLightInfo.LightColor.ToString()));
    OutProperties.Add(TEXT("Intensity"), FString::Printf(TEXT("%f"), PointLightInfo.Intensity));
    OutProperties.Add(TEXT("Type"), FString::Printf(TEXT("%d"), PointLightInfo.Type));
    OutProperties.Add(TEXT("Attenuation"), FString::Printf(TEXT("%f"), PointLightInfo.Attenuation));
    OutProperties.Add(TEXT("Position"), FString::Printf(TEXT("%s"), *PointLightInfo.Position.ToString()));
}

void UPointLightComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Radius"));
    if (TempStr)
    {
        PointLightInfo.Radius = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("LightColor"));
    if (TempStr)
    {
        PointLightInfo.LightColor.InitFromString(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Intensity"));
    if (TempStr)
    {
        PointLightInfo.Intensity = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Type"));
    if (TempStr)
    {
        PointLightInfo.Type = FString::ToInt(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Attenuation"));
    if (TempStr)
    {
        PointLightInfo.Attenuation = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("Position"));
    if (TempStr)
    {
        PointLightInfo.Position.InitFromString(*TempStr);
    }
    
}

const FPointLightInfo& UPointLightComponent::GetPointLightInfo() const
{
    return PointLightInfo;
}

void UPointLightComponent::SetPointLightInfo(const FPointLightInfo& InPointLightInfo)
{
    PointLightInfo = InPointLightInfo;
}


float UPointLightComponent::GetRadius() const
{
    return PointLightInfo.Radius;
}

void UPointLightComponent::SetRadius(float InRadius)
{
    PointLightInfo.Radius = InRadius;
}

FLinearColor UPointLightComponent::GetLightColor() const
{
    return PointLightInfo.LightColor;
}

void UPointLightComponent::SetLightColor(const FLinearColor& InColor)
{
    PointLightInfo.LightColor = InColor;
}


float UPointLightComponent::GetIntensity() const
{
    return PointLightInfo.Intensity;
}

void UPointLightComponent::SetIntensity(float InIntensity)
{
    PointLightInfo.Intensity = InIntensity;
}

int UPointLightComponent::GetType() const
{
    return PointLightInfo.Type;
}

void UPointLightComponent::SetType(int InType)
{
    PointLightInfo.Type = InType;
}

void UPointLightComponent::CreateShadowMapResources()
{
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width              = ShadowMapWidth;
    texDesc.Height             = ShadowMapHeight;
    texDesc.MipLevels          = 1;
    texDesc.ArraySize          = 6; // 큐브맵의 6면
    texDesc.Format             = DXGI_FORMAT_R32_TYPELESS;
    texDesc.SampleDesc.Count   = 1;
    texDesc.Usage              = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags          = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    GEngineLoop.GraphicDevice.Device->CreateTexture2D(&texDesc, nullptr, &PointDepthCubeTex);

    for(int i = 0; i < 6; ++i) {
        D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
        dsvDesc.Format               = DXGI_FORMAT_D32_FLOAT;
        dsvDesc.ViewDimension        = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
        dsvDesc.Texture2DArray.FirstArraySlice = i;
        dsvDesc.Texture2DArray.ArraySize       = 1;
        GEngineLoop.GraphicDevice.Device->CreateDepthStencilView(PointDepthCubeTex, &dsvDesc, &PointShadowDSV[i]);
    }
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = DXGI_FORMAT_R32_FLOAT;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MipLevels     = 1;
    GEngineLoop.GraphicDevice.Device->CreateShaderResourceView(PointDepthCubeTex, &srvDesc, &PointShadowSRV);

    for(int face = 0; face < 6; ++face) {
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format                    = DXGI_FORMAT_R32_FLOAT;              // R32_TYPELESS -> R32_FLOAT
        srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        srvDesc.Texture2DArray.MostDetailedMip = 0;
        srvDesc.Texture2DArray.MipLevels       = 1;
        srvDesc.Texture2DArray.FirstArraySlice = face;                           // 각 페이스
        srvDesc.Texture2DArray.ArraySize       = 1;
        GEngineLoop.GraphicDevice.Device->CreateShaderResourceView(
        PointDepthCubeTex, &srvDesc, &faceSRVs[face]);
    }
    D3D11_SAMPLER_DESC sampDesc = {};
    // 비교 샘플링 필터: MIN/MAG은 Linear, MIP은 Point (PCF용)
    sampDesc.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    // 텍스처 좌표가 [0,1] 범위를 벗어날 때 경계색(깊이=1)으로 처리
    sampDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
    sampDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
    // 테두리 색상: 최대조명(깊이 비교 시 항상 lit 되게)
    sampDesc.BorderColor[0] = 1.0f;
    sampDesc.BorderColor[1] = 1.0f;
    sampDesc.BorderColor[2] = 1.0f;
    sampDesc.BorderColor[3] = 1.0f;
    // 비교 함수: “현재 샘플 깊이 <= 맵에 저장된 깊이” 면 lit
    sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    // MIP 레벨 범위
    sampDesc.MinLOD         = 0;
    sampDesc.MaxLOD         = D3D11_FLOAT32_MAX;

    // 2) SamplerState 생성
    HRESULT hr = GEngineLoop.GraphicDevice.Device->CreateSamplerState(&sampDesc, &PointShadowComparisonSampler);
    projection = JungleMath::CreateProjectionMatrix(60.0f, 1.0f, 0.01f, GetRadius());
}

void UPointLightComponent::UpdateViewMatrix()
{
    for (int i=0; i < 6; i++)
    {
        FVector target = GetWorldLocation() + dirs[i];
        FVector up = ups[i];
        view[i] = JungleMath::CreateViewMatrix(GetWorldLocation(),target, up);
    }
}
