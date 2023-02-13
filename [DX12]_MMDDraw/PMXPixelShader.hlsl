#include "BasicShaderHeader.hlsli"


float4 main(Model input) : SV_TARGET
{
    float3 light = normalize(float3(1, -1, 1)); //光の向かうベクトル(平行光線)

	//ディフューズ計算
    float diffuseB = saturate(-dot(light, input.normal.rgb));
    float4 toonDif = toon.Sample(smptoon, float2(1, 1.0 - diffuseB));

	//光の反射ベクトル
    float3 refLight = normalize(reflect(light, input.normal.xyz));
    float specularB = pow(saturate(dot(refLight, -input.ray)), specular.a);
	
	//スフィアマップ用UV
    float2 sphereMapUV = input.vnormal.xy;
    sphereMapUV = (sphereMapUV + float2(1, -1)) * float2(0.5, -0.5);
    
    float4 texColor = tex.Sample(smp, input.uv); //テクスチャカラー
    float4 subtexcolor = subtex.Sample(smp, input.exuv.xy);
    float4 ambCol = float4(ambient * 0.01, 1); //アンビエント

    
    float4 outDif = saturate(toonDif //輝度(トゥーン)
		* diffuse //ディフューズ色
		* texColor //テクスチャカラー
		* (sph.Sample(smp, sphereMapUV))) //スフィアマップ(乗算) 
		+ saturate(spa.Sample(smp, sphereMapUV) //スフィアマップ(加算)
        * subtexcolor
		+ float4(specularB * specular * 0.1)) //スペキュラー
		+ float4(texColor * ambCol);
    
    outDif.a = (texColor.a * diffuse.a) ;
    //エッジ処理
    float d = (-dot(input.vnormal.xyz, input.ray));
    
    if (d <= edgesize * 0.1f)
    {
        outDif.xyz *= 0.0f;
        outDif.xyz += edgecolor.xyz;
    }
    
    return outDif;
    

}