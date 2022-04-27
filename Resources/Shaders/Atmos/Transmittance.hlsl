_OverrideSettings_

EntryCS = "CSMain"

_OverrideSettings_

#define STEP_COUNT 1000.0

#include "Resources/Shaders/Atmos/Common.hlsl"

RWTexture2D<float4> Transmittance : register(u0);

// https://cs.dartmouth.edu/wjarosz/publications/novak14residual.pdf
float3 CalculateTransmittance(float3 rayPosition, float3 sunDirection)
{
    // We need to check if ray hits planet first. 
    if (SolveQuadratic(rayPosition, sunDirection, PlanetRadius))
    {
        return float3(0.0, 0.0, 0.0);
    }

    float distance = 0.0;
    GetQuadraticIntersection3D(rayPosition, sunDirection, AtmosRadius, distance);
    float distancePerStep = distance / STEP_COUNT;

    float3 transmittance = float3(0.0, 0.0, 0.0);
    for (float i = 0.0; i < STEP_COUNT; i += 1.0)
    {
        rayPosition += distancePerStep * sunDirection + 0.3;

        // classic sphere function, in this case its used as altitude from ground
        float altitude = length(rayPosition) - PlanetRadius;
        transmittance += GetExtinctionSum(altitude);
    }

    // transmittance = extinction coefficient
    return exp(-distancePerStep * transmittance);  // equation 1 from jnovak
}

[numthreads(16, 16, 1)]
void CSMain(int3 threadID : SV_DISPATCHTHREADID)
{
    int width, height;
    Transmittance.GetDimensions(width, height);

    float u = float(threadID.x) / width;
    float v = float(threadID.y) / height;

    float h = lerp(PlanetRadius, AtmosRadius, v);
    float3 rayPosition = float3(0.0, h, 0.0);
    
    // https://www.desmos.com/calculator/guspypmdaa
    float sunCosTheta = 2.0 * u - 1.0;  // [0, 1] -> [-1, 1]
    float3 sunDirection = float3(0.0, sunCosTheta, sin(acos(sunCosTheta)));

    Transmittance[threadID.xy] = float4(CalculateTransmittance(rayPosition, sunDirection), 1);
}