Texture2D awesomefaceTex : register(t0);
Texture2D containerTex : register(t1);

SamplerState gsamLinear : register(s0);

cbuffer transformCB : register(b0)
{
    float4x4 transform;
};

struct VertexIn
{
    float3 PosL : POSITION;
    float2 TexC : TEXCOORD;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float2 TexC : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;
	
    vout.PosH = mul(float4(vin.PosL, 1.0f), transform);
    vout.TexC = vin.TexC;
    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    //return containerTex.Sample(gsamLinear, pin.TexC);
    return lerp(containerTex.Sample(gsamLinear, pin.TexC), awesomefaceTex.Sample(gsamLinear, pin.TexC), 0.2);
}


