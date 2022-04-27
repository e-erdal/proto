#include "Resources/Shaders/Atmos/Common.hlsl"

Texture2D TransmittanceLut  : register(t0);
Texture2D MultiScatterLut   : register(t1);
SamplerState LinearSampler  : register(s0);

cbuffer __ : register(b1)
{
    float3 EyePosition;
    float StepCount;
    float3 SunDirection;
    float SunIntensity;
}

struct VSLayout
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VSLayout VSMain(uint vertexID : SV_VertexID)
{
    VSLayout output;
    output.TexCoord = float2((vertexID << 1) & 2, vertexID & 2);
    output.Position = float4(output.TexCoord * float2(2, -2) + float2(-1, 1), 0.5, 1);
    return output;
}

float4 PSMain(VSLayout input) : SV_TARGET
{
    float u = input.TexCoord.x;
    float v = input.TexCoord.y;

    // Non-linear parameterization
    if (v < 0.5) 
    {
        float coord = 1.0 - 2.0 * v;
        v = coord * coord;
    } 
    else 
    {
        float coord = v * 2.0 - 1.0;
        v = -coord * coord;
    }

    float h = length(EyePosition);
    
    // https://en.wikipedia.org/wiki/Azimuth#/media/File:Azimuth-Altitude_schematic.svg
    float azimuthAngle = 2.0 * PI * u;  // Consider 360 degrees.
    float horizonAngle = acos(sqrt(h * h - PlanetRadius * PlanetRadius) / h) - 0.5 * PI;
    float altitudeAngle = v * 0.5 * PI - horizonAngle;
    
    float cosAltitude = cos(altitudeAngle);
    float3 rayDirection = float3(
        cosAltitude * cos(azimuthAngle),
        sin(altitudeAngle),
        cosAltitude * sin(azimuthAngle)
    );
    
    // Get Rayleigh + Mie phase
    float cosTheta = dot(rayDirection, SunDirection);
    float rayleighPhase = GetRayleighPhase(-cosTheta);
    float miePhase = GetMiePhase(cosTheta);
    
    float3 luminance = float3(0.0, 0.0, 0.0);
    float3 transmittance = float3(1.0, 1.0, 1.0);
    
    // Raymarching
    float maxDist = 0.0;
    if (!GetQuadraticIntersection3D(EyePosition, rayDirection, PlanetRadius, maxDist))
        GetQuadraticIntersection3D(EyePosition, rayDirection, AtmosRadius, maxDist);
    
    float stepSize = maxDist / StepCount;
    float t = 0.0;
    for (float i = 0.0; i < StepCount; i += 1.0)
    {
        float nextT = stepSize * i;
        float deltaT = nextT - t;
        t = nextT;
        
        float3 stepPosition = EyePosition + t * rayDirection;
        
        h = length(stepPosition);
        
        // Altitude from ground to top atmosphere
        float altitude = h - PlanetRadius;
        float3 extinction = GetExtinctionSum(altitude);
        float3 altitudeTrans = exp(-deltaT * extinction);
        
        // Shadowing factor
        float sunTheta = dot(SunDirection, stepPosition / h);
        float3 sunTrans = SampleLUT(TransmittanceLut, LinearSampler, altitude, sunTheta);
        float3 MS = SampleLUT(MultiScatterLut, LinearSampler, altitude, sunTheta);
        
        // Get scattering coefficient
        float3 rayleighScat;
        float mieScat;
        GetScattering(altitude, rayleighScat, mieScat);
        
        // Molecules scattered on ray's position
        float3 rayleighInScat = rayleighScat * (rayleighPhase + MS);
        float3 mieInScat = mieScat * (miePhase + MS);
        float3 scatteringPhase = (rayleighInScat + mieInScat) * sunTrans;

        // https://www.ea.com/frostbite/news/physically-based-unified-volumetric-rendering-in-frostbite
        // slide 28
        float3 integral = (scatteringPhase - scatteringPhase * altitudeTrans) / extinction;

        luminance += SunIntensity * (integral * transmittance);
        transmittance *= altitudeTrans;
    }

    return float4(luminance, 1.0);
}