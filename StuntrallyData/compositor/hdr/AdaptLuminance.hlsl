sampler s0 : register(s0);//lum
sampler s1 : register(s1);//oldlum

static const float TauCone = 0.01;
static const float TauRod = 0.04;

struct PS_IN
{
	float2 TexC : TEXCOORD0;
};

float4 PS(PS_IN i,uniform float AdaptationScale, uniform float dTime) : COLOR
{
    float Lum = tex2D(s0,float2(0.5f, 0.5f)).r;
    float oldLum = tex2D(s1,float2(0.5f, 0.5f)).r;

    //determin if rods or cones are active
    //Perceptual Effects in Real-time Tone Mapping: Equ(7)    
    float sigma = saturate(0.4/(0.04+Lum));

    //interpolate tau from taurod and taucone depending on lum
    //Perceptual Effects in Real-time Tone Mapping: Equ(12)
    float Tau = lerp(TauCone,TauRod,sigma) * AdaptationScale;

    //calculate adaption
    //Perceptual Effects in Real-time Tone Mapping: Equ(5)

	float result = oldLum + (Lum - oldLum) * (1 - exp(-(dTime)/Tau));
    return float4(result,result,result,result);
}
