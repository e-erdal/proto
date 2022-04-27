#include "App.hh"

#include "Utils/Random.hh"

#include <cy/cySampleElim.h>
#include <cy/cyPoint.h>

static void GeneratePoissonDiscSamples(u32 sampleCount, eastl::vector<cy::Point2f> &output)
{
    Random::Seed();

    eastl::vector<cy::Point2f> randomPoints(sampleCount * 10);

    for (auto &point : randomPoints)
    {
        point.x = Random::Float(0, 1);
        point.y = Random::Float(0, 1);
    }

    output.resize(sampleCount);

    cy::WeightedSampleElimination<cy::Point2f, float, 2> eliminator;
    eliminator.SetTiling(true);
    eliminator.Eliminate(randomPoints.data(), randomPoints.size(), output.data(), output.size());
}

void AtmosphereApp::Init()
{
    /// CONFIG INFO
    m_Config.SkyLUTRes = XMINT2(200 * 2, 100 * 2);
    m_Config.TransmittanceLUTRes = XMINT2(256, 64);
    m_Config.MultiScatterLUTRes = XMINT2(32, 32);

    /// LUT INFO
    m_LUTData.EyePosition = XMFLOAT3(0, 0, 0);
    m_LUTData.StepCount = 48;
    m_LUTData.SunIntensity = 10;
    SetSunDirection(XMFLOAT2(90, 45));

    /// SUN INFO
    m_SunInfo.EyePosition = m_LUTData.EyePosition;
    m_SunInfo.SunDirection = m_LUTData.SunDirection;
    m_SunInfo.SunIntensity = m_LUTData.SunIntensity;
    m_SunInfo.SunRadius = 0.4;

    /// MULTI SCATTER INFO
    m_MSInfo.SampleCount = 128;
    m_MSInfo.StepCount = 256;
    m_MSInfo.TerrainAlbedo = XMFLOAT3(0.3, 0.3, 0.3);
    m_MSInfo.SunIntensity = m_SunInfo.SunIntensity;

    ///////////////////////////////////
    /// LOAD SHADERS

    ShaderDesc genericShader;
    genericShader.Type = ShaderType::Compute;
    m_TransmittanceCS.Init(&genericShader, "Atmos/Transmittance");
    m_MSCS.Init(&genericShader, "Atmos/MultiScatter");

    genericShader.Type = ShaderType::Vertex;
    m_SkyLUTVS.Init(&genericShader, "Atmos/LUT");
    m_SkyFinalVS.Init(&genericShader, "Atmos/Final");

    genericShader.Type = ShaderType::Pixel;
    m_SkyLUTPS.Init(&genericShader, "Atmos/LUT");
    m_SkyFinalPS.Init(&genericShader, "Atmos/Final");

    ///////////////////////////////////

    RenderBufferDesc genericBuffer;
    genericBuffer.Type = RenderBufferType::Constant;
    genericBuffer.MemFlags = RenderBufferMemoryFlags::Access_CPUW;
    genericBuffer.Usage = RenderBufferUsage::Dynamic;

    /// CREATE ATMOSPHERE BUFFER
    genericBuffer.DataLen = sizeof(Atmosphere);
    m_AtmosphereBuffer.Init(genericBuffer);

    genericBuffer.DataLen = sizeof(SkyLUTData);
    m_SkyLUTBuffer.Init(genericBuffer);

    genericBuffer.DataLen = sizeof(CameraFrustum);
    m_FrustumBuffer.Init(genericBuffer);

    genericBuffer.DataLen = sizeof(SunInfo);
    m_SunBuffer.Init(genericBuffer);

    genericBuffer.DataLen = sizeof(MultiScatterInfo);
    m_MSBuffer.Init(genericBuffer);

    // Sample Buffer for MS
    eastl::vector<cy::Point2f> samples;
    GeneratePoissonDiscSamples(m_MSInfo.SampleCount, samples);

    genericBuffer.Type = RenderBufferType::ShaderResource;
    genericBuffer.MemFlags = RenderBufferMemoryFlags::Structured;
    genericBuffer.Usage = RenderBufferUsage::Immutable;

    genericBuffer.ByteStride = sizeof(XMFLOAT2);
    genericBuffer.pData = &samples[0];
    genericBuffer.DataLen = samples.size() * genericBuffer.ByteStride;
    m_SampleBuffer.Init(genericBuffer);

    /// CALCULATE TRANSMITTANCE
    UpdateTransmittance();
    UpdateMS();
    UpdateSkyLut();
}

void AtmosphereApp::Tick(float deltaTime)
{
    m_LUTData.EyePosition.y =
        bx::clamp(m_Atmosphere.PlanetRadius + XMVectorGetY(m_Camera.GetPosition()), m_Atmosphere.PlanetRadius + 2.0f, m_Atmosphere.AtmosRadius);

    m_SunInfo.EyePosition = m_LUTData.EyePosition;
    m_SunInfo.SunDirection = m_LUTData.SunDirection;
}

void AtmosphereApp::Draw()
{
    /// MAP RENDER BUFFERS TO GPU
    m_API.MapBuffer(&m_AtmosphereBuffer, &m_Atmosphere, sizeof(Atmosphere));
    m_API.MapBuffer(&m_SkyLUTBuffer, &m_LUTData, sizeof(SkyLUTData));
    m_API.MapBuffer(&m_SunBuffer, &m_SunInfo, sizeof(SunInfo));

    CameraFrustum frustum;
    m_Camera.GetFrustum(frustum);
    m_API.MapBuffer(&m_FrustumBuffer, &frustum, sizeof(CameraFrustum));

    /// RENDER LUT
    m_API.SetRenderTarget(&m_SkyLUT);
    m_API.ClearRenderTarget(&m_SkyLUT);

    m_API.SetShader(&m_SkyLUTVS);
    m_API.SetShader(&m_SkyLUTPS);

    m_API.SetConstantBuffer(&m_AtmosphereBuffer, RenderBufferTarget::Pixel, 0);
    m_API.SetConstantBuffer(&m_SkyLUTBuffer, RenderBufferTarget::Pixel, 1);

    m_API.SetShaderResource(&m_TransmittanceLUT, RenderBufferTarget::Pixel, 0);
    m_API.SetShaderResource(&m_MSLUT, RenderBufferTarget::Pixel, 1);
    m_API
        .SetSamplerState(TextureFiltering::Linear, TextureAddress::Clamp, TextureAddress::Clamp, TextureAddress::Clamp, RenderBufferTarget::Pixel, 0);

    m_API.SetPrimitiveType(PrimitiveType::TriangleList);
    m_API.Draw(3);

    /// RENDER FINAL
    m_API.SetRenderTarget(nullptr);

    m_API.SetShader(&m_SkyFinalVS);
    m_API.SetShader(&m_SkyFinalPS);

    m_API.SetConstantBuffer(&m_AtmosphereBuffer, RenderBufferTarget::Pixel, 0);
    m_API.SetConstantBuffer(&m_FrustumBuffer, RenderBufferTarget::Pixel, 1);
    m_API.SetConstantBuffer(&m_SunBuffer, RenderBufferTarget::Pixel, 2);

    m_API.SetShaderResource(m_SkyLUT.m_ColorAttachments[0], RenderBufferTarget::Pixel, 0);
    m_API.SetShaderResource(&m_TransmittanceLUT, RenderBufferTarget::Pixel, 1);

    m_API.SetSamplerState(TextureFiltering::Linear, TextureAddress::Wrap, TextureAddress::Clamp, TextureAddress::Clamp, RenderBufferTarget::Pixel, 0);

    m_API.Draw(3);

    /// RENDER IMGUI
    m_Atmosphere.ToReadableUnit();

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::SetNextWindowSize(ImVec2(-FLT_MIN, 600));
    ImGui::Begin("Atmosphere Settings");
    ImGui::Text("FPS: %d", (u32)ImGui::GetIO().Framerate);
    XMVECTOR pos = m_Camera.GetPosition();
    ImGui::Text("Camera Pos = (%f, %f, %f)", XMVectorGetX(pos), XMVectorGetY(pos), XMVectorGetZ(pos));

    bool updateTLUT = false;

    ImGui::Spacing();
    ImGui::Text("Atmosphere");
    ImGui::Separator();
    updateTLUT |= ImGui::InputFloat3("(um) Rayleigh Scattering", (float *)&m_Atmosphere.RayleighScatterVal);
    updateTLUT |= ImGui::InputFloat("(km) Rayleigh Density", &m_Atmosphere.RayleighDensity);

    updateTLUT |= ImGui::InputFloat("Mie Asymmetry", &m_Atmosphere.MieAsymmetry);
    updateTLUT |= ImGui::InputFloat("(um) Mie Scattering", &m_Atmosphere.MieScatterVal);
    updateTLUT |= ImGui::InputFloat("(km) Mie Density", &m_Atmosphere.MieDensity);
    updateTLUT |= ImGui::InputFloat("(um) Mie Absorption", &m_Atmosphere.MieAbsorptionVal);

    updateTLUT |= ImGui::InputFloat3("(um) Ozone Absorption", (float *)&m_Atmosphere.OzoneAbsorption);
    updateTLUT |= ImGui::InputFloat("(km) Ozone Thickness", &m_Atmosphere.OzoneThickness);
    updateTLUT |= ImGui::InputFloat("(km) Ozone Height", &m_Atmosphere.OzoneHeight);

    ImGui::Spacing();
    ImGui::Text("Sun");
    ImGui::Separator();

    bool updateSun = false;

    updateSun |= ImGui::SliderFloat("Sun Rotation X", &m_SunRotation.x, 0, 360);
    updateSun |= ImGui::SliderFloat("Sun Rotation Y", &m_SunRotation.y, -90, 90);
    ImGui::SliderFloat("Sun Intensity", &m_LUTData.SunIntensity, 0, 90);

    ImGui::Spacing();
    ImGui::Text("Debug");
    ImGui::Separator();
    if (ImGui::CollapsingHeader("Show Textures"))
    {
        ImGui::Image(&m_TransmittanceLUT, ImVec2(m_Config.TransmittanceLUTRes.x, m_Config.TransmittanceLUTRes.y));
        ImGui::Image(m_SkyLUT.m_ColorAttachments[0], ImVec2(m_Config.SkyLUTRes.x, m_Config.SkyLUTRes.y));
        ImGui::Image(&m_MSLUT, ImVec2(m_Config.MultiScatterLUTRes.x, m_Config.MultiScatterLUTRes.y));
    }

    ImGui::End();

    m_Atmosphere.ToMeters();

    if (updateSun) SetSunDirection(m_SunRotation);
    if (updateTLUT) UpdateTransmittance();
}

void AtmosphereApp::SetSunDirection(XMFLOAT2 rotation)
{
    m_SunRotation = rotation;

    float sunRadX = XMConvertToRadians(m_SunRotation.x);
    float sunRadY = XMConvertToRadians(m_SunRotation.y);
    XMVECTOR sunDirection = XMVectorSet(cos(sunRadX) * cos(sunRadY), sin(sunRadY), sin(sunRadX) * cos(sunRadY), 0);
    sunDirection = XMVector3Normalize(sunDirection);

    XMStoreFloat3(&m_LUTData.SunDirection, sunDirection);
}

void AtmosphereApp::UpdateTransmittance()
{
    m_TransmittanceLUT.Delete();

    /// CREATE COMPUTE TEXTURE
    TextureDesc textureComputeDesc;
    textureComputeDesc.Type = TextureType::RW;

    TextureData textureComputeData;
    textureComputeData.Width = m_Config.TransmittanceLUTRes.x;
    textureComputeData.Height = m_Config.TransmittanceLUTRes.y;
    textureComputeData.Format = TextureFormat::RGBA32F;

    Texture textureCompute;
    textureCompute.Init(&textureComputeDesc, &textureComputeData);

    /// CREATE TRANSMITTANCE TEXTURE
    TextureDesc textureTransDesc;
    textureTransDesc.Type = TextureType::Default;

    TextureData textureTransData;
    textureTransData.Width = textureComputeData.Width;
    textureTransData.Height = textureComputeData.Height;
    textureTransData.Format = textureComputeData.Format;

    m_TransmittanceLUT.Init(&textureTransDesc, &textureTransData);

    /// MAP ATMOSPHERE BUFFER
    m_API.MapBuffer(&m_AtmosphereBuffer, &m_Atmosphere, sizeof(Atmosphere));

    /// PREPARE STATE
    m_API.SetShader(&m_TransmittanceCS);
    m_API.SetConstantBuffer(&m_AtmosphereBuffer, RenderBufferTarget::Compute, 0);
    m_API.SetUAVResource(&textureCompute, RenderBufferTarget::Compute, 0);
    m_API.SetSamplerState(TextureFiltering::Linear,
                          TextureAddress::Clamp,
                          TextureAddress::Clamp,
                          TextureAddress::Clamp,
                          RenderBufferTarget::Compute,
                          0);

    m_API.Dispatch(ceil(textureComputeData.Width / 16), ceil(textureComputeData.Height / 16), 1);
    m_API.GetGPUTexture(&m_TransmittanceLUT, &textureCompute);
}

void AtmosphereApp::UpdateMS()
{
    m_MSLUT.Delete();

    /// CREATE COMPUTE TEXTURE
    TextureDesc textureComputeDesc;
    textureComputeDesc.Type = TextureType::RW;

    TextureData textureComputeData;
    textureComputeData.Width = m_Config.MultiScatterLUTRes.x;
    textureComputeData.Height = m_Config.MultiScatterLUTRes.y;
    textureComputeData.Format = TextureFormat::RGBA32F;

    Texture textureCompute;
    textureCompute.Init(&textureComputeDesc, &textureComputeData);

    /// CREATE MULTISCATTER TEXTURE
    TextureDesc textureMSDesc;
    textureMSDesc.Type = TextureType::Default;

    TextureData textureMSData;
    textureMSData.Width = textureComputeData.Width;
    textureMSData.Height = textureComputeData.Height;
    textureMSData.Format = textureComputeData.Format;

    m_MSLUT.Init(&textureMSDesc, &textureMSData);

    /// MAP BUFFERS
    m_API.MapBuffer(&m_AtmosphereBuffer, &m_Atmosphere, sizeof(Atmosphere));
    m_API.MapBuffer(&m_MSBuffer, &m_MSInfo, sizeof(MultiScatterInfo));

    /// PREPARE STATE
    m_API.SetShader(&m_MSCS);
    m_API.SetConstantBuffer(&m_AtmosphereBuffer, RenderBufferTarget::Compute, 0);
    m_API.SetConstantBuffer(&m_MSBuffer, RenderBufferTarget::Compute, 1);

    m_API.SetShaderResource(&m_SampleBuffer, RenderBufferTarget::Compute, 0);
    m_API.SetShaderResource(&m_TransmittanceLUT, RenderBufferTarget::Compute, 1);
    m_API.SetUAVResource(&textureCompute, RenderBufferTarget::Compute, 0);

    m_API.SetSamplerState(TextureFiltering::Linear,
                          TextureAddress::Clamp,
                          TextureAddress::Clamp,
                          TextureAddress::Clamp,
                          RenderBufferTarget::Compute,
                          0);

    m_API.Dispatch(ceil(textureComputeData.Width / 16), ceil(textureComputeData.Height / 16), 1);
    m_API.GetGPUTexture(&m_MSLUT, &textureCompute);
}

void AtmosphereApp::UpdateSkyLut()
{
    m_SkyLUT.Reset(true, true);

    TextureDesc textureSkyDesc;
    textureSkyDesc.Type = TextureType::RenderTarget;

    TextureData textureSkyData;
    textureSkyData.Width = m_Config.SkyLUTRes.x;
    textureSkyData.Height = m_Config.SkyLUTRes.y;
    textureSkyData.Format = TextureFormat::RGBA32F;

    Texture *pSky = new Texture;
    pSky->Init(&textureSkyDesc, &textureSkyData);

    m_SkyLUT.m_Width = m_Config.SkyLUTRes.x;
    m_SkyLUT.m_Height = m_Config.SkyLUTRes.y;
    m_SkyLUT.AttachColor(pSky);
}