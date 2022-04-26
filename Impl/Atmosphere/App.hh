//
// Created on Sunday 24th April 2022 by e-erdal
//

#pragma once

#include "Core/BaseApp.hh"

using namespace lr;

struct Atmosphere
{
    XMFLOAT3 RayleighScatterVal = { 5.802, 13.558, 33.1 };
    float RayleighDensity = 8;

    float PlanetRadius = 6360;
    float AtmosRadius = 6460;

    float MieScatterVal = 3.996;
    float MieAbsorptionVal = 4.4;
    float MieDensity = 1.2;
    float MieAsymmetry = 0.8;

    float OzoneHeight = 25;
    float OzoneThickness = 15;
    XMFLOAT3 OzoneAbsorption = { 0.650, 1.881, 0.085 };

    float __padding;

    Atmosphere()
    {
        ToMeters();
    }

    void ToMeters()
    {
        constexpr static float kuM = 1e-6;
        constexpr static float kKM = 1e3;

        RayleighDensity *= kKM;

        PlanetRadius *= kKM;
        AtmosRadius *= kKM;

        MieDensity *= kKM;

        OzoneHeight *= kKM;
        OzoneThickness *= kKM;

        RayleighScatterVal.x *= kuM;
        RayleighScatterVal.y *= kuM;
        RayleighScatterVal.z *= kuM;

        MieScatterVal *= kuM;
        MieAbsorptionVal *= kuM;

        OzoneAbsorption.x *= kuM;
        OzoneAbsorption.y *= kuM;
        OzoneAbsorption.z *= kuM;
    }

    void ToReadableUnit()
    {
        constexpr static float kuM = 1e-6;
        constexpr static float kKM = 1e3;

        RayleighDensity /= kKM;

        PlanetRadius /= kKM;
        AtmosRadius /= kKM;

        MieDensity /= kKM;

        OzoneHeight /= kKM;
        OzoneThickness /= kKM;

        RayleighScatterVal.x /= kuM;
        RayleighScatterVal.y /= kuM;
        RayleighScatterVal.z /= kuM;

        MieScatterVal /= kuM;
        MieAbsorptionVal /= kuM;

        OzoneAbsorption.x /= kuM;
        OzoneAbsorption.y /= kuM;
        OzoneAbsorption.z /= kuM;
    }
};

class AtmosphereApp : public BaseApp
{
public:
    void Init() override;
    void Tick(float deltaTime) override;
    void Draw() override;

    void SetSunDirection(XMFLOAT2 rotation);
    
    void UpdateTransmittance();
    void UpdateSkyLut();

private:
    struct SkyConfig
    {
        XMINT2 SkyLUTRes;
        XMINT2 TransmittanceLUTRes;
    } m_Config;

    struct SkyLUTData
    {
        XMFLOAT3 EyePosition;
        float StepCount;
        XMFLOAT3 SunDirection;
        float SunIntensity;
    } m_LUTData;

    struct SunInfo
    {
        XMFLOAT3 EyePosition;
        float SunRadius;
        XMFLOAT3 SunDirection;
        float SunIntensity;
    } m_SunInfo;
 
private:
    XMFLOAT2 m_SunRotation;

    Shader m_TransmittanceCS;
    Texture m_TransmittanceLUT;

    Shader m_SkyLUTVS;
    Shader m_SkyLUTPS;
    RenderBuffer m_SkyLUTBuffer;
    RenderTargetView m_SkyLUT;

    Shader m_SkyFinalVS;
    Shader m_SkyFinalPS;

    Atmosphere m_Atmosphere;
    RenderBuffer m_AtmosphereBuffer;

    RenderBuffer m_FrustumBuffer;
    RenderBuffer m_SunBuffer;
};