#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <stdio.h>
#include <assert.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_5.h>
#include <d3dcompiler.h>
#include <DirectXTex.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <algorithm>


#pragma comment(lib,"DirectXTex.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "winmm.lib")

#define SCREEN_WIDTH	(1280)
#define SCREEN_HEIGHT	(720)
#define FPS				(60.0f)

using namespace std;
using namespace DirectX;
using namespace Microsoft::WRL;
