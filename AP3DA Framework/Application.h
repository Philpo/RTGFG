#pragma once

#include <windows.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include "DDSTextureLoader.h"
#include "resource.h"
#include "Camera.h"
#include "Terrain.h"
#include <vector>
#include <time.h>
#include "GameObject.h"
#include <iostream>
#include <sstream>
#include "TypeDefs.h"
#include "RapidXML\rapidxml.hpp"
#include "RapidXML\rapidxml_utils.hpp"
#include "KeyFrame.h"
#include "Skeleton.h"

using namespace DirectX;
using namespace rapidxml;

struct sphere {
  XMFLOAT3 centre;
  float radius;
};

class Application {
public:
  Application();
  ~Application();

  HRESULT initialise(HINSTANCE hInstance, int nCmdShow);

  bool handleKeyboard(MSG msg);
  void handleMouseMovement(WPARAM buttonStates, int x, int y);
  void handleMouseClick(WPARAM buttonStates, int x, int y);

  void update();
  void shadowPass(ConstantBuffer& cb);
  void renderToTextures(ConstantBuffer& cb);
  void shadowMapping();
  void deferredRendering();
  void draw();
  inline void setWindowCaption(const std::wostringstream& caption) const { SetWindowText(hWnd, caption.str().c_str()); }

private:
  HINSTANCE hInst;
  HWND hWnd;
  D3D_DRIVER_TYPE driverType;
  D3D_FEATURE_LEVEL featureLevel;
  ID3D11Device* d3dDevice;
  ID3D11DeviceContext* immediateContext;
  IDXGISwapChain* swapChain;
  ID3D11RenderTargetView* renderTargetView;
  ID3D11Texture2D* deferredTextures[DEFERRED_BUFFERS];
  ID3D11RenderTargetView* deferredRenderTargets[DEFERRED_BUFFERS];
  ID3D11ShaderResourceView* deferredResourceViews[DEFERRED_BUFFERS];
  ID3D11VertexShader* vertexShader;
  ID3D11VertexShader* instanceVertexShader;
  ID3D11PixelShader* shadowPixelShader;
  ID3D11PixelShader* shadowTerrainPixelShader;
  ID3D11PixelShader* deferredPixelShader;
  ID3D11PixelShader* deferredTerrainPixelShader;
  ID3D11PixelShader* lightingPixelShader;
  ID3D11HullShader* controlPointHullShader;
  ID3D11DomainShader* domainShader;
  ID3D11InputLayout* vertexLayout;
  ID3D11InputLayout* instanceLayout;

  ID3D11Buffer* vertexBuffer;
  ID3D11Buffer* skeletonVertexBuffer;
  ID3D11Buffer* indexBuffer;

  ID3D11Buffer* planeVertexBuffer;

  ID3D11Buffer* fullScreenVertexBuffer;
  ID3D11Buffer* fullScreenIndexBuffer;

  ID3D11Buffer* constantBuffer;

  ID3D11DepthStencilView* depthStencilView = nullptr;
  ID3D11Texture2D* depthStencilBuffer = nullptr;
  ID3D11DepthStencilView* lightDepthBuffer = nullptr;
  ID3D11RenderTargetView* null = nullptr;

  ID3D11ShaderResourceView* textureRV = nullptr;
  ID3D11ShaderResourceView* terrainTextureRV1 = nullptr;
  ID3D11ShaderResourceView* terrainTextureRV2 = nullptr;
  ID3D11ShaderResourceView* terrainTextureRV3 = nullptr;
  ID3D11ShaderResourceView* terrainTextureRV4 = nullptr;
  ID3D11ShaderResourceView* lightDepthView = nullptr;

  D3D11_VIEWPORT vp, lightVP;

  ID3D11SamplerState* samplerAnistropic = nullptr;
  ID3D11SamplerState* shadowSampler;

  Light basicLight;

  vector<GameObject*> bones;
  vector<KeyFrame> animation;
  Skeleton* skeleton;
  GameObject* bone1;

  vector<Camera*> cameras;
  float cameraSpeed = 2.0f;

  UINT windowHeight;
  UINT windowWidth;

  // Render dimensions - Change here to alter screen resolution
  UINT renderHeight = 1080;
  UINT renderWidth = 1920;

  ID3D11DepthStencilState* dsLessEqual;
  ID3D11RasterizerState *rsCullNone, *rsCullNoneWireFrame;

  ID3D11BlendState* transparency;
  ID3D11RasterizerState* ccwCullMode;
  ID3D11RasterizerState* cwCullMode;

  ID3D11RasterizerState* wireframe;

  bool wireFrame = false;
  bool deferredPipeline = false;
  bool perlinNoise = true;
  Material noSpecMaterial;
  Geometry planeGeometry;
  Terrain* terrain;
  const float TERRAIN_WIDTH = 512.0f;
  const float TERRAIN_DEPTH = 512.0f;
  const float CELL_WIDTH = 1.0f;
  const float CELL_DEPTH = 1.0f;
  int lastMousePosX, lastMousePosY;
  int selectedCamera = 0;

  HRESULT initWindow(HINSTANCE hInstance, int nCmdShow);
  HRESULT initDevice();
  void cleanup();
  HRESULT compileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
  HRESULT initShadersAndInputLayout();
  HRESULT initVertexBuffer();
  HRESULT initTerrainVertexBuffer();
  HRESULT initIndexBuffer();
};