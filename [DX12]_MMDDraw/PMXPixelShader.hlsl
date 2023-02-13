#include "BasicShaderHeader.hlsli"


float4 main(Model input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1)); //���̌������x�N�g��(���s����)

	//�f�B�t���[�Y�v�Z
    float diffuseB = saturate(-dot(light, input.normal.rgb));
    float4 toonDif = toon.Sample(smptoon, float2(1, 1.0 - diffuseB));

	//���̔��˃x�N�g��
    float3 refLight = normalize(reflect(light, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
	
	//�X�t�B�A�}�b�v�pUV
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);
    
    float4 texColor = tex.Sample(smp, input.uv); //�e�N�X�`���J���[
    float4 subtexcolor = subtex.Sample(smp, input.exuv.xy);
    float4 ambCol = float4(ambient * 0.01, 1); //�A���r�G���g

    
    float4 outDif = saturate(toonDif //�P�x(�g�D�[��)
		* diffuse //�f�B�t���[�Y�F
		* texColor //�e�N�X�`���J���[
		* (sph.Sample(smp, sphereMapUV))) //�X�t�B�A�}�b�v(��Z) 
		+ saturate(spa.Sample(smp, sphereMapUV) //�X�t�B�A�}�b�v(���Z)
        * subtexcolor
		+ float4(specularB * specular * 0.1)) //�X�y�L�����[
		+ float4(texColor * ambCol);
    
    outDif.a = (texColor.a * diffuse.a) ;
    //�G�b�W����
    float d = (-dot(input.vnormal.xyz, input.ray));
    
    if (d <= edgesize * 0.1f)
    {
        outDif.xyz *= 0.0f;
        outDif.xyz += edgecolor.xyz;
    }
    
    return outDif;
    

}