#include "Resources/Shaders/Atmos/Common.hlsl"

Texture2D       SkyLUT  : register(t0);
Texture2D Transmittance : register(t1);
SamplerState    SkySampler;

cbuffer __ : register(b1)
{
    float3 FrustumX;
    float3 FrustumY;
    float3 FrustumZ;
    float3 FrustumW;
}

cbuffer _2 : register(b2)
{
    float3 EyePosition;
    float SunRadius;
    float3 SunDirection;
    float SunIntensity;
}

struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

PSInput VSMain(uint vertexID : SV_VertexID)
{
    PSInput output;
    output.TexCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.Position = float4(output.TexCoord * float2(2, -2) + float2(-1, 1), 0.99999, 1);
    return output;
}

float3 ACES(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

	return saturate(pow(abs((x * (a * x + b)) / (x * (c * x + d) + e)), 1 / 1.7));
}

float3 FixHDR(float2 seed, float3 color)
{
    color *= 255;

    float rand = frac(sin(dot(seed, float2(12.9898, 78.233) * 2.0)) * 43758.5453);
    color = rand.xxx < (color - floor(color)) ? ceil(color) : floor(color);

    color /= 255;

    return color;
}

float3 GetSun(float3 rayDir, float3 sunDir) 
{
    float sunCos = dot(rayDir, sunDir);
    float radCos = cos(SunRadius * PI / 180.0);
    if (sunCos > radCos)
    {
        return SunIntensity;
    }

    return 0.0;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    float3 direction = normalize(
        lerp(
        lerp(normalize(FrustumX), normalize(FrustumY), input.TexCoord.x),
        lerp(normalize(FrustumZ), normalize(FrustumW), input.TexCoord.x), 
        input.TexCoord.y)
    );

    float l = asin(direction.y);
    float u = atan2(direction.z, direction.x) / (2.0 * PI);
    float v = 0.5 - 0.5 * sign(l) * sqrt(abs(l) / (0.5 * PI));

    float3 color = SkyLUT.Sample(SkySampler, float2(u, v)).rgb;
    color = ACES(color);
    color = FixHDR(input.TexCoord, color);

    color += GetSun(direction, SunDirection);

    return float4(color, 1.0);
}
