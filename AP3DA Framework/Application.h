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
#include <noise\noise.h>
#include <iostream>
#include "noiseutils.h"
#include <sstream>
#include "Chunk.h"
#include "TypeDefs.h"

using namespace noise;
using namespace DirectX;

struct sphere {
  XMFLOAT3 centre;
  float radius;
};

class Application {
private:
  HINSTANCE               _hInst;
  HWND                    _hWnd;
  D3D_DRIVER_TYPE         _driverType;
  D3D_FEATURE_LEVEL       _featureLevel;
  ID3D11Device*           _pd3dDevice;
  ID3D11DeviceContext*    _pImmediateContext;
  IDXGISwapChain*         _pSwapChain;
  ID3D11RenderTargetView* _pRenderTargetView;
  ID3D11Texture2D* deferredTextures[DEFERRED_BUFFERS];
  ID3D11RenderTargetView* deferredRenderTargets[DEFERRED_BUFFERS];
  ID3D11ShaderResourceView* deferredResourceViews[DEFERRED_BUFFERS];
  ID3D11VertexShader*     _pVertexShader;
  ID3D11VertexShader* instanceVertexShader;
  ID3D11PixelShader*      shadowPixelShader;
  ID3D11PixelShader*      shadowTerrainPixelShader;
  ID3D11PixelShader*      deferredPixelShader;
  ID3D11PixelShader*      deferredTerrainPixelShader;
  ID3D11PixelShader*      lightingPixelShader;
  ID3D11HullShader* controlPointHullShader;
  ID3D11DomainShader* domainShader;
  ID3D11InputLayout*      _pVertexLayout;
  ID3D11InputLayout*      instanceLayout;

  ID3D11Buffer*           _pVertexBuffer;
  ID3D11Buffer*           _pIndexBuffer;

  ID3D11Buffer*           _pPlaneVertexBuffer;
  ID3D11Buffer*           _pPlaneIndexBuffer;

  ID3D11Buffer*           fullScreenVertexBuffer;
  ID3D11Buffer*           fullScreenIndexBuffer;

  ID3D11Buffer*           _pConstantBuffer;

  ID3D11DepthStencilView* _depthStencilView = nullptr;
  ID3D11Texture2D* _depthStencilBuffer = nullptr;
  ID3D11DepthStencilView* lightDepthBuffer = nullptr;
  ID3D11RenderTargetView* null = nullptr;

  ID3D11ShaderResourceView * _pTextureRV = nullptr;
  ID3D11ShaderResourceView * terrainTextureRV1 = nullptr;
  ID3D11ShaderResourceView * terrainTextureRV2 = nullptr;
  ID3D11ShaderResourceView * terrainTextureRV3 = nullptr;
  ID3D11ShaderResourceView * terrainTextureRV4 = nullptr;
  ID3D11ShaderResourceView * terrainTextureRV5 = nullptr;
  ID3D11ShaderResourceView * blendMap = nullptr;
  ID3D11ShaderResourceView* lightDepthView = nullptr;

  D3D11_VIEWPORT vp, lightVP;

  ID3D11SamplerState * _pSamplerLinear = nullptr;
  ID3D11SamplerState* shadowSampler;

  Light basicLight;

  vector<Chunk*> chunks;
  vector<GameObject *> _gameObjects;

  vector<Camera*> cameras;
  float _cameraOrbitRadius = 7.0f;
  float _cameraOrbitRadiusMin = 2.0f;
  float _cameraOrbitRadiusMax = 50.0f;
  float _cameraOrbitAngleXZ = 0.0f;
  float _cameraSpeed = 2.0f;

  UINT _WindowHeight;
  UINT _WindowWidth;

  // Render dimensions - Change here to alter screen resolution
  UINT _renderHeight = 1080;
  UINT _renderWidth = 1920;

  ID3D11DepthStencilState* DSLessEqual;
  ID3D11RasterizerState* RSCullNone, *RSCullNoneWireFrame;

  ID3D11BlendState* Transparency;
  ID3D11RasterizerState* CCWcullMode;
  ID3D11RasterizerState* CWcullMode;

  ID3D11RasterizerState* wireframe;

  bool _wireFrame = false;
  Terrain *terrain;
  const float TERRAIN_WIDTH = 512.0f;
  const float TERRAIN_DEPTH = 512.0f;
  const float CELL_WIDTH = 1.0f;
  const float CELL_DEPTH = 1.0f;
  int lastMousePosX, lastMousePosY;
  int selectedCamera = 0;
private:
  HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
  HRESULT InitDevice();
  void Cleanup();
  HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
  HRESULT InitShadersAndInputLayout();
  HRESULT InitVertexBuffer();
  HRESULT initTerrainVertexBuffer();
  HRESULT InitIndexBuffer();

public:
  Application();
  ~Application();

  HRESULT Initialise(HINSTANCE hInstance, int nCmdShow);

  bool HandleKeyboard(MSG msg);
  void handleMouseMovement(WPARAM buttonStates, int x, int y);
  void handleMouseClick(WPARAM buttonStates, int x, int y);

  void Update();
  void shadowPass(ConstantBuffer& cb);
  void renderToTextures(ConstantBuffer& cb);
  void shadowMapping();
  void deferredRendering();
  void Draw();
  inline void setWindowCaption(const std::wostringstream& caption) const { SetWindowText(_hWnd, caption.str().c_str()); }
};

