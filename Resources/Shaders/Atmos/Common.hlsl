#define PI 3.14159265358

cbuffer Atmosphere : register(b0)
{
    float3 RayleighScatterVal;
    float RayleighDensity;

    float PlanetRadius;
    float AtmosRadius;

    float MieScatterVal;
    float MieAbsorptionVal;

    float MieDensity;
    float MieAsymmetry;

    float OzoneHeight;
    float OzoneThickness;
    float3 OzoneAbsorption;
}

float3 SampleLUT(Texture2D targetLUT, SamplerState linearSampler, float altitude, float theta)
{
    float2 uv = float2(0.5 + 0.5 * theta, max(0.0, min(altitude / (AtmosRadius - PlanetRadius), 1.0)));
    return targetLUT.SampleLevel(linearSampler, uv, 0.0).xyz;
}

float3 GetExtinctionSum(float altitude)
{
    float3 rayleigh = RayleighScatterVal * exp(-altitude / RayleighDensity);
    float mie = (MieScatterVal + MieAbsorptionVal) * exp(-altitude / MieDensity);
    float3 ozone = OzoneAbsorption * max(0.0, 1.0 - abs(altitude - OzoneHeight) / OzoneThickness);

    return rayleigh + mie + ozone;
}

void GetScattering(float altitude, out float3 rayleigh, out float mie)
{
    rayleigh = RayleighScatterVal * exp(-altitude / RayleighDensity);
    mie = (MieScatterVal + MieAbsorptionVal) * exp(-altitude / MieDensity);
}

float GetRayleighPhase(float altitude)
{
    const float k = 3.0 / (16.0 * PI);
    return k * (1.0 + altitude * altitude);
}

float GetMiePhase(float altitude)
{
    const float g = MieAsymmetry;
    const float g2 = g * g;
    const float scale = 3.0 / (8.0 * PI);

    float num = (1.0 - g2) * (1.0 + altitude * altitude);
    float denom = (2.0 + g2) * pow(abs(1.0 + g2 - 2.0 * g * altitude), 1.5);
    
    return scale * num / denom;
}

// Finds if a point intersects with a circle, returns false if ray hits ground
bool SolveQuadratic(float2 origin, float2 direction, float radius)
{
    float a = dot(direction, direction);
    float b = 2.0 * dot(origin, direction);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    return (discriminant >= 0.0) && (b <= 0.0);
}

bool SolveQuadratic(float3 origin, float3 direction, float radius)
{
    float a = dot(direction, direction);
    float b = 2.0 * dot(origin, direction);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    return (discriminant >= 0.0) && (b <= 0.0);
}

// Out T = distance from origin
bool GetQuadraticIntersection(float2 origin, float2 direction, float radius, out float t)
{
    float a = dot(direction, direction);
    float b = 2.0 * dot(origin, direction);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
        return false;

    if (c <= 0.0)
        t = (-b + sqrt(discriminant)) / (a * 2.0);
    else 
        t = (-b + -sqrt(discriminant)) / (a * 2.0);
    
    return (b <= 0.0);
}

// Out T = distance from origin
bool GetQuadraticIntersection3D(float3 origin, float3 direction, float radius, out float t)
{
    float a = dot(direction, direction);
    float b = 2.0 * dot(origin, direction);
    float c = dot(origin, origin) - radius * radius;
    float discriminant = b * b - 4.0 * a * c;

    if (discriminant < 0.0)
        return false;

    if (c <= 0.0)
        t = (-b + sqrt(discriminant)) / (a * 2.0);
    else 
        t = (-b + -sqrt(discriminant)) / (a * 2.0);
    
    return (b <= 0.0);
}