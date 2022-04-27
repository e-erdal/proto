_OverrideSettings_

EntryCS = "CSMain"

_OverrideSettings_

#include "Resources/Shaders/Atmos/Common.hlsl"

cbuffer ___ : register(b1)
{
    float3 TerrainAlbedo;
    float StepCount;
    uint SampleCount;
    float SunIntensity;
}

RWTexture2D<float4>      MultiScattering    : register(u0);
StructuredBuffer<float2> SampleBuffer       : register(t0);
Texture2D<float4>        TransmittanceLut   : register(t1);
SamplerState             LinearSampler      : register(s0);

// https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere
float3 GetUniformSphereSample(float2 dirSample)
{
    float dirX = 1.0 - 2.0 * dirSample.x;  // new direction from 1 to -1
    float radius = sqrt(max(0, 1.0 - dirX * dirX));
    float phi = 2.0 * PI * dirSample.y;

    return float3(radius * cos(phi), radius * sin(phi), dirX);
}

float3 getSphericalDir(float theta, float phi) {
     float cosPhi = cos(phi);
     float sinPhi = sin(phi);
     float cosTheta = cos(theta);
     float sinTheta = sin(theta);
     return float3(sinPhi*sinTheta, cosPhi, sinPhi*cosTheta);
}

float3 ComputeSample(float3 rayPos, float3 sunDir)
{
    // Start of equation 5
    float3 totalL2 = float3(0.0, 0.0, 0.0);  
    float3 totalFms = float3(0.0, 0.0, 0.0);

    // We already calculated samples using Cem Yuksel's library
    // So just integrate the multiscatter, sample generation is not needed
    for (uint si = 0; si < SampleCount / 2; si++)
    {
        float3 sampleDir = GetUniformSphereSample(SampleBuffer[si]);  // w_s

        // At this point, evaluation is very similar to LUT evaluation
        // So this is mostly copy-paste from LUT.hlsl

        float maxDist = 0.0;
        bool planetHit = GetQuadraticIntersection3D(rayPos, sampleDir, PlanetRadius, maxDist);
        if (!planetHit)
            GetQuadraticIntersection3D(rayPos, sampleDir, AtmosRadius, maxDist);

        // Get Rayleigh + Mie phase
        float cosTheta = dot(sampleDir, sunDir);
        float rayleighPhase = GetRayleighPhase(-cosTheta);
        float miePhase = GetMiePhase(cosTheta);

        // Ray marching
        float3 L2 = float3(0.0, 0.0, 0.0);
        float3 Fms = float3(0.0, 0.0, 0.0);
        float3 transmittance = float3(1.0, 1.0, 1.0);

        float stepSize = maxDist / StepCount;
        float t = 0.0;
        for (float i = 0.0; i < StepCount; i += 1.0)
        {
            float nextT = stepSize * i;
            float deltaT = nextT - t;
            t = nextT;
            
            float3 stepPosition = rayPos + t * sampleDir;  // w_s
            
            float h = length(stepPosition);

            // Altitude from ground to top atmosphere
            float altitude = h - PlanetRadius;
            float3 extinction = GetExtinctionSum(altitude);
            float3 altitudeTrans = exp(-deltaT * extinction);  // T(x, x - tv)

            // Shadowing factor --- S(x, x - tw_s)
            float sunTheta = dot(sunDir, stepPosition / h);
            // This function on paper seems to be wrong?
            // Its getting sample direction instead of step direction
            // So it becomes S(x, x - tw_s) like extinction function
            float3 sunTrans = SampleLUT(TransmittanceLut, LinearSampler, altitude, sunTheta);

            // Equation 6
            // Molecules scattered on ray's position
            float3 rayleighScat;
            float mieScat;
            GetScattering(altitude, rayleighScat, mieScat);
        
            // Molecules scattered on ray's position
            float3 rayleighInScat = rayleighScat * rayleighPhase;
            float3 mieInScat = mieScat * miePhase;
            float3 scatteringPhase = (rayleighInScat + mieInScat) * sunTrans;

            float3 integralLum = (scatteringPhase - scatteringPhase * altitudeTrans) / extinction;
            L2 += integralLum * transmittance;

            // Equation 7, a bit different, cancels phase function
            float3 scatteringNoPhase = rayleighScat + mieScat;
            float3 integralFactor = (scatteringNoPhase - scatteringNoPhase * altitudeTrans) / extinction;
            Fms += integralFactor * transmittance;
            
            transmittance *= altitudeTrans;
        }

        // Calculate ground albedo
        if (planetHit)
        {
            float3 intersectPos = rayPos + maxDist * sampleDir;

            float h = length(intersectPos);
            float3 up = intersectPos / h;

            float altitude = h - PlanetRadius;
            float theta = dot(sunDir, up);

            float3 sunTrans = SampleLUT(TransmittanceLut, LinearSampler, altitude, theta);

            // magic function, actually called Normal.Light
            float lightTheta = saturate(dot(up, sunDir));

            L2 += sunTrans * transmittance * lightTheta * TerrainAlbedo / PI;
        }

        totalL2 += L2;
        totalFms += Fms;
    }

    float3 L2 = totalL2 / SampleCount;
    float3 Fms = totalFms / SampleCount;

    return L2 / (1.0 - Fms);
}

[numthreads(16, 16, 1)]
void CSMain(int3 threadID : SV_DISPATCHTHREADID)
{
    int width, height;
    MultiScattering.GetDimensions(width, height);

    float u = float(threadID.x) / width;
    float v = float(threadID.y) / height;

    float h = lerp(PlanetRadius, AtmosRadius, v);
    float3 rayPosition = float3(0.0, h, 0.0);
    
    // https://www.desmos.com/calculator/guspypmdaa
    float sunCosTheta = 2.0 * u - 1.0;  // [0, 1] -> [-1, 1]
    float3 sunDirection = float3(0.0, sunCosTheta, sin(acos(sunCosTheta)));

    MultiScattering[threadID.xy] = float4(ComputeSample(rayPosition, sunDirection), 1.0);
}