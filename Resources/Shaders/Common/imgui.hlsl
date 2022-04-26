cbuffer VP : register(b0) 
{ 
    matrix prMat; 
}

cbuffer PCbuf : register(b0) 
{ 
    float lodLevel; 
}

struct VS_Input 
{
    float2 Pos      : POSITION;
    float2 UV       : TEXCOORD0;
    float4 Color    : COLOR0;
};

struct PS_Input 
{
    float4 Pos      : SV_POSITION;
    float2 UV       : TEXCOORD0;
    float4 Color    : COLOR0;
};

Texture2D texture0    : register(t0);
SamplerState sampler0 : register(s0);

PS_Input VSMain(VS_Input input) {
    PS_Input output;
    output.Pos = mul(prMat, float4(input.Pos.xy, 0.f, 1.f));
    output.Color = input.Color;
    output.UV = input.UV;

    return output;
}

float4 PSMain(PS_Input input) : SV_TARGET 
{
    return input.Color * texture0.SampleLevel(sampler0, input.UV, lodLevel);
}