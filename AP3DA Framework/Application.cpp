#include "Application.h"

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  PAINTSTRUCT ps;
  HDC hdc;

  switch (message) {
    case WM_PAINT:
      hdc = BeginPaint(hWnd, &ps);
      EndPaint(hWnd, &ps);
      break;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }

  return 0;
}

bool Application::HandleKeyboard(MSG msg) {
  switch (msg.wParam) {
    case VK_UP:
      cameras[selectedCamera]->move(_cameraSpeed * 0.2f, *terrain);
      return true;
    case VK_DOWN:
      cameras[selectedCamera]->move(_cameraSpeed * -0.2f, *terrain);
      return true;
    case VK_RIGHT:
      cameras[selectedCamera]->strafe(_cameraSpeed * 0.2f, *terrain);
      return true;
    case VK_LEFT:
      cameras[selectedCamera]->strafe(_cameraSpeed * -0.2f, *terrain);
      return true;
    case VK_SHIFT:
      if (msg.message == WM_KEYDOWN) {
        _wireFrame = !_wireFrame;
      }
      return true;
    case VK_SPACE:
      cameras[selectedCamera]->moveY(_cameraSpeed * 0.2f);
      return true;
    case VK_TAB:
      if (msg.message == WM_KEYDOWN) {
        selectedCamera = selectedCamera == cameras.size() - 1 ? 0 : selectedCamera + 1;
      }
      return true;
    case 0x57:
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->GetRotation();
        rotation.x += XMConvertToRadians(-10.0f);
        bones[1]->SetRotation(rotation);
      }
      return true;
    case 0x53:
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->GetRotation();
        rotation.x += XMConvertToRadians(10.0f);
        bones[1]->SetRotation(rotation);
      }
      return true;
    case 0x41:
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->GetRotation();
        rotation.y += XMConvertToRadians(-10.0f);
        bones[1]->SetRotation(rotation);
      }
      return true;
    case 0x44:
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->GetRotation();
        rotation.y += XMConvertToRadians(10.0f);
        bones[1]->SetRotation(rotation);
      }
      return true;
    case 0x45:
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[0]->GetRotation();
        rotation.x += XMConvertToRadians(-10.0f);
        bones[0]->SetRotation(rotation);;
      }
      return true;
    case 0x51:
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[0]->GetRotation();
        rotation.x += XMConvertToRadians(10.0f);
        bones[0]->SetRotation(rotation);
      }
      return true;
  }

  return false;
}

void Application::handleMouseClick(WPARAM buttonStates, int x, int y) {
  if ((buttonStates & MK_LBUTTON) != 0) {
    lastMousePosX = x;
    lastMousePosY = y;
  }
}

void Application::handleMouseMovement(WPARAM buttonStates, int x, int y) {
  if ((buttonStates & MK_LBUTTON) != 0) {
    float xRotation = XMConvertToRadians(x - lastMousePosX);
    float yRotation = XMConvertToRadians(y - lastMousePosY);

    cameras[selectedCamera]->rotatePitch(yRotation);
    cameras[selectedCamera]->rotateYaw(xRotation);

    lastMousePosX = x;
    lastMousePosY = y;
  }
}

Application::Application() : lastMousePosX(0.0f), lastMousePosY(0.0f) {
  _hInst = nullptr;
  _hWnd = nullptr;
  _driverType = D3D_DRIVER_TYPE_NULL;
  _featureLevel = D3D_FEATURE_LEVEL_11_0;
  _pd3dDevice = nullptr;
  _pImmediateContext = nullptr;
  _pSwapChain = nullptr;
  _pRenderTargetView = nullptr;
  _pVertexShader = nullptr;
  shadowPixelShader = nullptr;
  _pVertexLayout = nullptr;
  _pVertexBuffer = nullptr;
  _pIndexBuffer = nullptr;
  _pConstantBuffer = nullptr;

  DSLessEqual = nullptr;
  RSCullNone = nullptr;
}

Application::~Application() {
  Cleanup();
}

HRESULT Application::Initialise(HINSTANCE hInstance, int nCmdShow) {
  if (FAILED(InitWindow(hInstance, nCmdShow))) {
    return E_FAIL;
  }

  AllocConsole();
  freopen("CONOUT$", "wb", stdout);

  //module::RidgedMulti mountainTerrain;
  //module::Billow baseFlatTerrain;
  //baseFlatTerrain.SetFrequency(2.0);
  //module::ScaleBias flatTerrain;
  //flatTerrain.SetSourceModule(0, baseFlatTerrain);
  //flatTerrain.SetScale(0.125);
  //flatTerrain.SetBias(-0.75);
  //module::Perlin terrainType;
  //terrainType.SetFrequency(0.5);
  //terrainType.SetPersistence(0.25); 
  //module::Select terrainSelector;
  //terrainSelector.SetSourceModule(0, flatTerrain);
  //terrainSelector.SetSourceModule(1, mountainTerrain);
  //terrainSelector.SetControlModule(terrainType);
  //terrainSelector.SetBounds(0.0, 1000.0);
  //terrainSelector.SetEdgeFalloff(0.125); 
  //module::Turbulence finalTerrain;
  //finalTerrain.SetSourceModule(0, terrainSelector);
  //finalTerrain.SetFrequency(4.0);
  //finalTerrain.SetPower(0.125);
  //utils::NoiseMap heightMap;
  //utils::NoiseMapBuilderPlane heightMapBuilder;
  //heightMapBuilder.SetSourceModule(finalTerrain);
  ////heightMapBuilder.SetSourceModule(mountainTerrain);
  //heightMapBuilder.SetDestNoiseMap(heightMap);
  //heightMapBuilder.SetDestSize(TERRAIN_WIDTH, TERRAIN_DEPTH);
  //heightMapBuilder.SetBounds(6.0, 10.0, 1.0, 5.0);
  //heightMapBuilder.Build();

  //utils::RendererImage renderer;
  //utils::Image image;
  //renderer.SetSourceNoiseMap(heightMap);
  //renderer.SetDestImage(image);
  //renderer.ClearGradient();
  //renderer.AddGradientPoint(-1.00, utils::Color(32, 160, 0, 255)); // grass
  //renderer.AddGradientPoint(-0.25, utils::Color(224, 224, 0, 255)); // dirt
  //renderer.AddGradientPoint(0.25, utils::Color(128, 128, 128, 255)); // rock
  //renderer.AddGradientPoint(1.00, utils::Color(255, 255, 255, 255)); // snow
  //renderer.EnableLight();
  //renderer.SetLightContrast(3.0);
  //renderer.SetLightBrightness(2.0);
  //renderer.Render();

  //utils::WriterBMP writer;
  //writer.SetSourceImage(image);
  //writer.SetDestFilename("tutorial.bmp");
  //writer.WriteDestFile();

  RECT rc;
  GetClientRect(_hWnd, &rc);
  _WindowWidth = rc.right - rc.left;
  _WindowHeight = rc.bottom - rc.top;

  Material noSpecMaterial;
  noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
  noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  noSpecMaterial.specularPower = 0.0f;

  Material oceanMaterial;
  oceanMaterial.ambient = XMFLOAT4(0.0f, 0.0f, 0.1f, 0.1f);
  oceanMaterial.diffuse = XMFLOAT4(0.0f, 0.0f, 1.0f, 0.1f);
  oceanMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  oceanMaterial.specularPower = 0.0f;

  Material voxelMaterial;
  voxelMaterial.ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
  voxelMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  voxelMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  voxelMaterial.specularPower = 0.0f;

  if (FAILED(InitDevice())) {
    Cleanup();

    return E_FAIL;
  }

  terrain = new Terrain(noSpecMaterial);
  //terrain->loadHeightMap(TERRAIN_DEPTH, TERRAIN_WIDTH, "Resources\\coastMountain513.raw");
  terrain->diamondSquare(TERRAIN_DEPTH, time(0), 1.0f, 255.0f);
  //terrain->circleHill(TERRAIN_DEPTH, TERRAIN_WIDTH, time(0), 20000, 2, 2);
  double seed = time(0);
  double seed2 = time(0);
  //terrain->perlinNoise(TERRAIN_DEPTH, TERRAIN_WIDTH, 6.0, 10.0, 1.0, 5.0);
  terrain->generateGeometry(TERRAIN_DEPTH, TERRAIN_WIDTH, 1.0f, 1.0f * tan(XM_PIDIV4), 16, _renderHeight, _pd3dDevice, _pImmediateContext, CELL_WIDTH, CELL_DEPTH);

  if (FAILED(initTerrainVertexBuffer())) {
    Cleanup();
    return E_FAIL;
  }

  CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\Crate_COLOR.dds", nullptr, &_pTextureRV);
  //CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\grass.dds", nullptr, &terrainTextureRV1);
  CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\darkdirt.dds", nullptr, &terrainTextureRV2);
  CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\stone.dds", nullptr, &terrainTextureRV3);
  CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\lightdirt.dds", nullptr, &terrainTextureRV1);
  CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\snow.dds", nullptr, &terrainTextureRV4);
  CreateDDSTextureFromFile(_pd3dDevice, L"Resources\\blend.dds", nullptr, &blendMap);

  _pImmediateContext->PSSetShaderResources(1, 1, &terrainTextureRV1);
  _pImmediateContext->PSSetShaderResources(2, 1, &terrainTextureRV2);
  _pImmediateContext->PSSetShaderResources(3, 1, &terrainTextureRV3);
  _pImmediateContext->PSSetShaderResources(4, 1, &terrainTextureRV4);
  //_pImmediateContext->PSSetShaderResources(5, 1, &terrainTextureRV5);
  //_pImmediateContext->PSSetShaderResources(6, 1, &blendMap);

  // Setup Camera
  XMFLOAT3 eye = XMFLOAT3(0.0f, 2.0f, -10.0f);
  XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 1.0f);
  XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

  cameras.push_back(new Camera(true, true, eye, at, up, (float) _renderWidth, (float) _renderHeight, 1.0f, 1000.0f));
  cameras.push_back(new Camera(true, false, eye, at, up, (float) _renderWidth, (float) _renderHeight, 1.0f, 1000.0f));

  // Setup the scene's light
  basicLight.AmbientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
  basicLight.DiffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  basicLight.SpecularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
  basicLight.SpecularPower = 20.0f;
  basicLight.LightVecW = XMFLOAT3(-0.57735f, 0.57735f, -0.57735f);

  Geometry cubeGeometry;
  cubeGeometry.indexBuffer = _pIndexBuffer;
  cubeGeometry.vertexBuffer = _pVertexBuffer;
  cubeGeometry.numberOfIndices = 36;
  cubeGeometry.vertexBufferOffset = 0;
  cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

  Geometry skeletonGeometry;
  skeletonGeometry.indexBuffer = _pIndexBuffer;
  skeletonGeometry.vertexBuffer = skeletonVertexBuffer;
  skeletonGeometry.numberOfIndices = 36;
  skeletonGeometry.vertexBufferOffset = 0;
  skeletonGeometry.vertexBufferStride = sizeof(SimpleVertex);

  Geometry planeGeometry;
  planeGeometry.indexBuffer = _pPlaneIndexBuffer;
  planeGeometry.vertexBuffer = _pPlaneVertexBuffer;
  planeGeometry.numberOfIndices = terrain->getNumIndices();
  planeGeometry.vertexBufferOffset = 0;
  planeGeometry.vertexBufferStride = sizeof(SimpleVertex);

  Material shinyMaterial;
  shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
  shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
  shinyMaterial.specularPower = 10.0f;

  terrain->setGeometry(planeGeometry);
  terrain->SetPosition(0.0f, 0.0f, 0.0f);

  GameObject * gameObject = new GameObject("Cube 1", cubeGeometry, shinyMaterial);
  gameObject->SetPosition(0.0f, 2.0f, -2.0f);
  gameObject->SetTextureRV(_pTextureRV);

  //_gameObjects.push_back(gameObject);

  GameObject* root = new GameObject("root", skeletonGeometry, shinyMaterial);
  root->SetPosition(0.0f, 2.0f, -2.0f);
  root->SetScale(0.25f, 0.25f, 2.0f);
  root->SetRotation(0.0f, 0.0f, 0.0f);
  bones.push_back(root);

  GameObject* node1 = new GameObject("node1", skeletonGeometry, shinyMaterial);
  node1->SetParent(root);
  node1->SetPosition(0.0f, 0.0f, 4.0f);
  node1->SetRotation(0.0f, 0.0f, 0.0f);
  node1->SetScale(0.25f, 0.25f, 2.0f);
  bones.push_back(node1);

  skeleton = new Skeleton(2, _pd3dDevice, _pImmediateContext);

  GameObject* bone1 = new GameObject("root", skeletonGeometry, shinyMaterial);
  bone1->SetPosition(-4.0f, 2.0f, -2.0f);
  bone1->SetScale(0.25f, 0.25f, 2.0f);
  bone1->SetRotation(0.0f, 0.0f, 0.0f);
  skeleton->addBone(bone1);

  GameObject* bone2 = new GameObject("node1", skeletonGeometry, shinyMaterial);
  bone2->SetParent(bone1);
  bone2->SetPosition(0.0f, 0.0f, 4.0f);
  bone2->SetRotation(0.0f, 0.0f, 0.0f);
  bone2->SetScale(0.25f, 0.25f, 2.0f);
  skeleton->addBone(bone2);

  file<> file("test_animation.xml");
  xml_document<> doc;
  doc.parse<0>(file.data());
  xml_node<>* rootNode = doc.first_node();

  for (xml_node<>* frameNode = rootNode->first_node()->first_node(); frameNode; frameNode = frameNode->next_sibling()) {
    KeyFrame frame;
    frame.loadFrame(frameNode);
    animation.push_back(frame);
  }

  //gameObject = new GameObject("Cube 2", cubeGeometry, shinyMaterial);
  //gameObject->SetScale(0.5f, 0.5f, 0.5f);
  //gameObject->SetPosition(-203.0f, -74.0f, -200.0f);
  //gameObject->SetTextureRV(_pTextureRV);

  //_gameObjects.push_back(gameObject);

  //Chunk* chunk = new Chunk(16, 8, 8, cubeGeometry, voxelMaterial, _pd3dDevice, _pImmediateContext);
  //chunk->SetPosition(0.0, 0.0, 0.0);
  //chunk->perlinNoise2D(time(0), 8, 8, 4, 0.5, 4);
  //chunk->perlinNoise3D(time(0), 8, 8, 16, 4, 0.5, 1);
  //chunk->Update(0);
  //chunks.push_back(chunk);

  //chunk = new Chunk(16, 8, 8, cubeGeometry, voxelMaterial, _pd3dDevice, _pImmediateContext);
  //chunk->SetPosition(-16.0, 0.0, 0.0);
  //chunk->Update(0);
  //chunks.push_back(chunk);

  //chunk = new Chunk(16, 8, 8, cubeGeometry, voxelMaterial, _pd3dDevice, _pImmediateContext);
  //chunk->SetPosition(16.0, 0.0, 0.0);
  //chunk->Update(0);
  //chunks.push_back(chunk);

  return S_OK;
}

HRESULT Application::InitShadersAndInputLayout() {
  HRESULT hr;

  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  hr = CompileShaderFromFile(L"GeneralShaders.fx", "VS", "vs_4_0", &pVSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = _pd3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &_pVertexShader);

  if (FAILED(hr)) {
    pVSBlob->Release();
    return hr;
  }

  ID3DBlob* instanceVSBlob = nullptr;
  hr = CompileShaderFromFile(L"GeneralShaders.fx", "INSTANCE_VS", "vs_4_0", &instanceVSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = _pd3dDevice->CreateVertexShader(instanceVSBlob->GetBufferPointer(), instanceVSBlob->GetBufferSize(), nullptr, &instanceVertexShader);

  if (FAILED(hr)) {
    instanceVSBlob->Release();
    return hr;
  }

  // Compile the hull shader
  ID3DBlob* pHSBlob = nullptr;

  hr = CompileShaderFromFile(L"GeneralShaders.fx", "HS", "hs_5_0", &pHSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the control point hull shader
  hr = _pd3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &controlPointHullShader);

  if (FAILED(hr)) {
    pHSBlob->Release();
    return hr;
  }

  // Compile the domain shader
  ID3DBlob* pDSBlob = nullptr;
  hr = CompileShaderFromFile(L"GeneralShaders.fx", "DS", "ds_5_0", &pDSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the domain shader
  hr = _pd3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &domainShader);

  if (FAILED(hr)) {
    pDSBlob->Release();
    return hr;
  }

  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromFile(L"ShadowMapping.fx", "SHADOW_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shadowPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"ShadowMapping.fx", "SHADOW_TERRAIN_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shadowTerrainPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"DeferredRendering.fx", "DEFERRED_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &deferredPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"DeferredRendering.fx", "DEFERRED_TERRAIN_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &deferredTerrainPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = CompileShaderFromFile(L"DeferredRendering.fx", "LIGHTING_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = _pd3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &lightingPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  // Define the input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
  };

  UINT numElements = ARRAYSIZE(layout);

  // Create the input layout
  hr = _pd3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), &_pVertexLayout);
  pVSBlob->Release();

  if (FAILED(hr))
    return hr;

  D3D11_INPUT_ELEMENT_DESC instanceDesc[] =
  {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
  };

  numElements = ARRAYSIZE(instanceDesc);

  // Create the input layout
  hr = _pd3dDevice->CreateInputLayout(instanceDesc, numElements, instanceVSBlob->GetBufferPointer(),
    instanceVSBlob->GetBufferSize(), &instanceLayout);
  instanceVSBlob->Release();

  if (FAILED(hr))
    return hr;

  // Set the input layout
  _pImmediateContext->IASetInputLayout(_pVertexLayout);

  D3D11_SAMPLER_DESC sampDesc;
  ZeroMemory(&sampDesc, sizeof(sampDesc));
  sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
  sampDesc.MaxAnisotropy = 16;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  sampDesc.MinLOD = 0;
  sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
  hr = _pd3dDevice->CreateSamplerState(&sampDesc, &_pSamplerLinear);

  //sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  //sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  //sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  //sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  //sampDesc.MipLODBias = 0.0f;
  //sampDesc.MaxAnisotropy = 1;
  //sampDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
  //sampDesc.BorderColor[0] = 0;
  //sampDesc.BorderColor[1] = 0;
  //sampDesc.BorderColor[2] = 0;
  //sampDesc.BorderColor[3] = 0;
  //sampDesc.MinLOD = 0;
  //sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

  //hr = _pd3dDevice->CreateSamplerState(&sampDesc, &s);

  sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

  hr = _pd3dDevice->CreateSamplerState(&sampDesc, &shadowSampler);

  return hr;
}

HRESULT Application::InitVertexBuffer() {
  HRESULT hr;

  // Create vertex buffer
  SimpleVertex vertices[] =
  {
    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
  };

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(SimpleVertex) * 24;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = vertices;

  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pVertexBuffer);

  if (FAILED(hr))
    return hr;

  SimpleVertex skeletonVertices[] =
  {
    { XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 2.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(1.0f, -1.0f, 2.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, 2.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 2.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 2.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },

    { XMFLOAT3(1.0f, -1.0f, 2.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(1.0f, 1.0f, 2.0f), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },

    { XMFLOAT3(-1.0f, -1.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(1.0f, -1.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
    { XMFLOAT3(1.0f, 1.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(-1.0f, 1.0f, 2.0f), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
  };

  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = skeletonVertices;

  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &skeletonVertexBuffer);

  if (FAILED(hr))
    return hr;

  float left = (float) ((1920.0f / 2.0f) * -1.0f);

  // Calculate the screen coordinates of the right side of the window.
  float right = left + (float) 1920.0f;

  // Calculate the screen coordinates of the top of the window.
  float top = (float) (1080.0f / 2);

  // Calculate the screen coordinates of the bottom of the window.
  float bottom = top - (float) 1080.0f;

  SimpleVertex quadVertices[] = 
  {
    { XMFLOAT3(left, top, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(right, bottom, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
    { XMFLOAT3(left, bottom, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) },

    { XMFLOAT3(left, top, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT2(0.0f, 0.0f) },
    { XMFLOAT3(right, top, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) },
    { XMFLOAT3(right, bottom, 0.0f), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT2(1.0f, 1.0f) },
  };

  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(SimpleVertex) * 6;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = quadVertices;

  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &fullScreenVertexBuffer);

  if (FAILED(hr))
    return hr;

  return S_OK;
}

HRESULT Application::initTerrainVertexBuffer() {
  // Create vertex buffer
  const SimpleVertex* planeVertices = terrain->getVertices();

  D3D11_BUFFER_DESC bd;
  D3D11_SUBRESOURCE_DATA InitData;

  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(SimpleVertex) * terrain->getNumVertices();
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;

  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = planeVertices;

  HRESULT hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneVertexBuffer);

  if (FAILED(hr))
    return hr;
}

HRESULT Application::InitIndexBuffer() {
  HRESULT hr;

  // Create index buffer
  UINT indices[] =
  {
    3, 1, 0,
    2, 1, 3,

    6, 4, 5,
    7, 4, 6,

    11, 9, 8,
    10, 9, 11,

    14, 12, 13,
    15, 12, 14,

    19, 17, 16,
    18, 17, 19,

    22, 20, 21,
    23, 20, 22
  };

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));

  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(UINT) * 36;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = indices;
  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pIndexBuffer);

  if (FAILED(hr))
    return hr;

  // Create plane index buffer
//  const UINT* planeIndices = terrain->getIndices();
//
//  ZeroMemory(&bd, sizeof(bd));
//  bd.Usage = D3D11_USAGE_DEFAULT;
//  bd.ByteWidth = sizeof(UINT) * terrain->getNumIndices();
//  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//  bd.CPUAccessFlags = 0;
//
//  ZeroMemory(&InitData, sizeof(InitData));
//  InitData.pSysMem = planeIndices;
//  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pPlaneIndexBuffer);
//
//  if (FAILED(hr))
//    return hr;
//
//  // Create plane index buffer
//  planeIndices = ocean->getIndices();
//
//  ZeroMemory(&bd, sizeof(bd));
//  bd.Usage = D3D11_USAGE_DEFAULT;
//  bd.ByteWidth = sizeof(UINT) * ocean->getNumIndices();
//  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
//  bd.CPUAccessFlags = 0;
//
//  ZeroMemory(&InitData, sizeof(InitData));
//  InitData.pSysMem = planeIndices;
////  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &_pOceanPlaneIndexBuffer);
//
//  if (FAILED(hr))
//    return hr;

  UINT quadIndices[] =
  {
    0, 1, 2,
    3, 4, 5
  };

  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(UINT) * 6;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;

  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = quadIndices;
  hr = _pd3dDevice->CreateBuffer(&bd, &InitData, &fullScreenIndexBuffer);

  if (FAILED(hr))
    return hr;

  return S_OK;
}

HRESULT Application::InitWindow(HINSTANCE hInstance, int nCmdShow) {
  // Register class
  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, (LPCTSTR) IDI_TUTORIAL1);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
  wcex.lpszMenuName = nullptr;
  wcex.lpszClassName = L"TutorialWindowClass";
  wcex.hIconSm = LoadIcon(wcex.hInstance, (LPCTSTR) IDI_TUTORIAL1);
  if (!RegisterClassEx(&wcex))
    return E_FAIL;

  // Create window
  _hInst = hInstance;
  RECT rc = { 0, 0, 960, 540 };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  _hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
    nullptr);
  if (!_hWnd)
    return E_FAIL;

  ShowWindow(_hWnd, nCmdShow);

  return S_OK;
}

HRESULT Application::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) {
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

  ID3DBlob* pErrorBlob;
  hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
    dwShaderFlags, 0, ppBlobOut, &pErrorBlob);

  if (FAILED(hr)) {
    if (pErrorBlob != nullptr)
      OutputDebugStringA((char*) pErrorBlob->GetBufferPointer());

    if (pErrorBlob) pErrorBlob->Release();

    return hr;
  }

  if (pErrorBlob) pErrorBlob->Release();

  return S_OK;
}

HRESULT Application::InitDevice() {
  HRESULT hr = S_OK;

  UINT createDeviceFlags = 0;

#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverTypes[] =
  {
    D3D_DRIVER_TYPE_HARDWARE,
    D3D_DRIVER_TYPE_WARP,
    D3D_DRIVER_TYPE_REFERENCE,
  };

  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  D3D_FEATURE_LEVEL featureLevels[] =
  {
    D3D_FEATURE_LEVEL_11_0,
    D3D_FEATURE_LEVEL_10_1,
    D3D_FEATURE_LEVEL_10_0,
  };

  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  UINT sampleCount = 1;

  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 1;
  sd.BufferDesc.Width = _renderWidth;
  sd.BufferDesc.Height = _renderHeight;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = _hWnd;
  sd.SampleDesc.Count = sampleCount;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
    _driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDeviceAndSwapChain(nullptr, _driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION, &sd, &_pSwapChain, &_pd3dDevice, &_featureLevel, &_pImmediateContext);
    if (SUCCEEDED(hr))
      break;
  }

  if (FAILED(hr))
    return hr;

  // Create a render target view
  ID3D11Texture2D* pBackBuffer = nullptr;
  hr = _pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &pBackBuffer);

  if (FAILED(hr))
    return hr;

  hr = _pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &_pRenderTargetView);
  pBackBuffer->Release();

  if (FAILED(hr))
    return hr;

  // create deferred rendering resources

  D3D11_TEXTURE2D_DESC textureDesc;
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

  ZeroMemory(&textureDesc, sizeof(textureDesc));
  textureDesc.Width = _renderWidth;
  textureDesc.Height = _renderHeight;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    hr = _pd3dDevice->CreateTexture2D(&textureDesc, NULL, &deferredTextures[i]);
    if (FAILED(hr)) {
      return hr;
    }
  }

  // Setup the description of the render target view.
  renderTargetViewDesc.Format = textureDesc.Format;
  renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
  renderTargetViewDesc.Texture2D.MipSlice = 0;

  // Create the render target views.
  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    hr = _pd3dDevice->CreateRenderTargetView(deferredTextures[i], &renderTargetViewDesc, &deferredRenderTargets[i]);
    if (FAILED(hr)) {
      return hr;
    }
  }

  // Setup the description of the shader resource view.
  shaderResourceViewDesc.Format = textureDesc.Format;
  shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
  shaderResourceViewDesc.Texture2D.MipLevels = 1;

  // Create the shader resource views.
  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    hr = _pd3dDevice->CreateShaderResourceView(deferredTextures[i], &shaderResourceViewDesc, &deferredResourceViews[i]);
    if (FAILED(hr)) {
      return false;
    }
  }

  // Setup the viewport
  vp.Width = (FLOAT) _renderWidth;
  vp.Height = (FLOAT) _renderHeight;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  _pImmediateContext->RSSetViewports(1, &vp);

  lightVP.Width = 2048.0f;
  lightVP.Height = 2048.0f;
  lightVP.MinDepth = 0.0f;
  lightVP.MaxDepth = 1.0f;
  lightVP.TopLeftX = 0;
  lightVP.TopLeftY = 0;

  InitShadersAndInputLayout();

  InitVertexBuffer();
  InitIndexBuffer();

  // Set primitive topology
  //_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

  // Create the constant buffer
  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(ConstantBuffer);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = _pd3dDevice->CreateBuffer(&bd, nullptr, &_pConstantBuffer);

  if (FAILED(hr))
    return hr;

  D3D11_TEXTURE2D_DESC depthStencilDesc;

  depthStencilDesc.Width = _renderWidth;
  depthStencilDesc.Height = _renderHeight;
  depthStencilDesc.MipLevels = 1;
  depthStencilDesc.ArraySize = 1;
  depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilDesc.SampleDesc.Count = sampleCount;
  depthStencilDesc.SampleDesc.Quality = 0;
  depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
  depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthStencilDesc.CPUAccessFlags = 0;
  depthStencilDesc.MiscFlags = 0;

  _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &_depthStencilBuffer);
  _pd3dDevice->CreateDepthStencilView(_depthStencilBuffer, nullptr, &_depthStencilView);

  _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView);

  depthStencilDesc.Width = 2048;
  depthStencilDesc.Height = 2048;
  depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

  ID3D11Texture2D* lightTex;
  hr = _pd3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &lightTex);

  if (FAILED(hr)) {
    return hr;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
  viewDesc.Flags = 0;
  viewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  viewDesc.Texture2D.MipSlice = 0;

  hr = _pd3dDevice->CreateDepthStencilView(lightTex, &viewDesc, &lightDepthBuffer);

  if (FAILED(hr)) {
    return hr;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC rViewDesc;
  rViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  rViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  rViewDesc.Texture2D.MipLevels = depthStencilDesc.MipLevels;
  rViewDesc.Texture2D.MostDetailedMip = 0;

  hr = _pd3dDevice->CreateShaderResourceView(lightTex, &rViewDesc, &lightDepthView);

  if (FAILED(hr)) {
    return hr;
  }

  lightTex->Release();

  // Rasterizer
  D3D11_RASTERIZER_DESC cmdesc;

  ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.CullMode = D3D11_CULL_NONE;
  hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCullNone);

  ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
  cmdesc.FillMode = D3D11_FILL_WIREFRAME;
  cmdesc.CullMode = D3D11_CULL_NONE;
  hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &RSCullNoneWireFrame);

  D3D11_DEPTH_STENCIL_DESC dssDesc;
  ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
  dssDesc.DepthEnable = true;
  dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

  _pd3dDevice->CreateDepthStencilState(&dssDesc, &DSLessEqual);

  // Blending 

  D3D11_BLEND_DESC blendDesc;
  ZeroMemory(&blendDesc, sizeof(blendDesc));

  D3D11_RENDER_TARGET_BLEND_DESC rtbd;
  ZeroMemory(&rtbd, sizeof(rtbd));

  rtbd.BlendEnable = true;
  rtbd.SrcBlend = D3D11_BLEND_SRC_COLOR;
  rtbd.DestBlend = D3D11_BLEND_BLEND_FACTOR;
  rtbd.BlendOp = D3D11_BLEND_OP_ADD;
  rtbd.SrcBlendAlpha = D3D11_BLEND_ONE;
  rtbd.DestBlendAlpha = D3D11_BLEND_ZERO;
  rtbd.BlendOpAlpha = D3D11_BLEND_OP_ADD;
  rtbd.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

  blendDesc.AlphaToCoverageEnable = false;
  blendDesc.RenderTarget[0] = rtbd;

  _pd3dDevice->CreateBlendState(&blendDesc, &Transparency);

  ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.CullMode = D3D11_CULL_BACK;

  cmdesc.FrontCounterClockwise = true;
  hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CCWcullMode);

  cmdesc.FrontCounterClockwise = false;
  hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &CWcullMode);

  _pImmediateContext->RSSetState(CWcullMode);

  cmdesc.FillMode = D3D11_FILL_WIREFRAME;
  cmdesc.CullMode = D3D11_CULL_NONE;
  hr = _pd3dDevice->CreateRasterizerState(&cmdesc, &wireframe);

  return S_OK;
}

void Application::Cleanup() {
  if (_pImmediateContext) _pImmediateContext->ClearState();
  if (_pSamplerLinear) _pSamplerLinear->Release();

  if (_pTextureRV) _pTextureRV->Release();

  if (_pConstantBuffer) _pConstantBuffer->Release();

  if (_pVertexBuffer) _pVertexBuffer->Release();
  if (_pIndexBuffer) _pIndexBuffer->Release();
  //if (_pPlaneVertexBuffer) _pPlaneVertexBuffer->Release();
  //if (_pPlaneIndexBuffer) _pPlaneIndexBuffer->Release();

  if (_pVertexLayout) _pVertexLayout->Release();
  if (_pVertexShader) _pVertexShader->Release();
  if (controlPointHullShader) controlPointHullShader->Release();
  if (domainShader) domainShader->Release();
  if (shadowPixelShader) shadowPixelShader->Release();
  if (_pRenderTargetView) _pRenderTargetView->Release();
  if (_pSwapChain) _pSwapChain->Release();
  if (_pImmediateContext) _pImmediateContext->Release();
  if (_pd3dDevice) _pd3dDevice->Release();
  if (_depthStencilView) _depthStencilView->Release();
  if (_depthStencilBuffer) _depthStencilBuffer->Release();

  if (DSLessEqual) DSLessEqual->Release();
  if (RSCullNone) RSCullNone->Release();

  if (Transparency) Transparency->Release();
  if (CCWcullMode) CCWcullMode->Release();
  if (CWcullMode) CWcullMode->Release();

  for (auto camera : cameras) {
    delete camera;
  }

  for (auto gameObject : _gameObjects) {
    if (gameObject) {
      delete gameObject;
      gameObject = nullptr;
    }
  }
}

void Application::Update() {
  // Update our time
  static float t = 0.0f;

  t += (float) XM_PI * 0.000125f;

  static float timeSinceStart = 0.0f;
  static DWORD dwTimeStart = 0;

  DWORD dwTimeCur = GetTickCount();

  if (dwTimeStart == 0)
    dwTimeStart = dwTimeCur;

  timeSinceStart = (dwTimeCur - dwTimeStart) / 1000.0f;

  // Update camera

  //float angleAroundZ = XMConvertToRadians(_cameraOrbitAngleXZ);

  //float x = _cameraOrbitRadius * cos(angleAroundZ);
  //float z = _cameraOrbitRadius * sin(angleAroundZ);

  //XMFLOAT3 cameraPos = cameras[selectedCamera]->GetPosition();
  //cameraPos.x = x;
  //cameraPos.z = z;

  //cameras[selectedCamera]->SetPosition(cameraPos);
  cameras[selectedCamera]->Update();

  XMVECTOR toLight = XMLoadFloat3(&XMFLOAT3(-0.57735f, 0.57735f, -0.57735f));
  toLight = XMVector3Transform(toLight, XMMatrixRotationY(t));
  XMStoreFloat3(&basicLight.LightVecW, toLight);

  // Update objects

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->GetView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->GetProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

  XMFLOAT4X4 vp;
  XMStoreFloat4x4(&vp, XMMatrixMultiply(XMMatrixTranspose(projection), XMMatrixTranspose(view)));
  XMFLOAT4 leftPlane = { -vp._11 - vp._41, -vp._12 - vp._42, -vp._13 - vp._43, -vp._14 - vp._44 };
  XMFLOAT4 rightPlane = { vp._11 - vp._41, vp._12 - vp._42, vp._13 - vp._43, vp._14 - vp._44 };
  XMFLOAT4 bottomPlane = { -vp._21 - vp._41, -vp._22 - vp._42, -vp._23 - vp._43, -vp._24 - vp._44 };
  XMFLOAT4 topPlane = { vp._21 - vp._41, vp._22 - vp._42, vp._23 - vp._43, vp._24 - vp._44 };
  XMFLOAT4 nearPlane = { -vp._31 - vp._41, -vp._32 - vp._42, -vp._33 - vp._43, -vp._34 - vp._44 };
  XMFLOAT4 farPlane = { vp._31 - vp._41, vp._32 - vp._42, vp._33 - vp._43, vp._34 - vp._44 };

  XMStoreFloat4(&leftPlane, XMVector4Normalize(XMLoadFloat4(&leftPlane)));
  XMStoreFloat4(&rightPlane, XMVector4Normalize(XMLoadFloat4(&rightPlane)));
  XMStoreFloat4(&topPlane, XMVector4Normalize(XMLoadFloat4(&topPlane)));
  XMStoreFloat4(&bottomPlane, XMVector4Normalize(XMLoadFloat4(&bottomPlane)));
  XMStoreFloat4(&nearPlane, XMVector4Normalize(XMLoadFloat4(&nearPlane)));
  XMStoreFloat4(&farPlane, XMVector4Normalize(XMLoadFloat4(&farPlane)));

  terrain->setCameraPosition(cameras[selectedCamera]->GetPosition());
  terrain->frustumCull(leftPlane, rightPlane, topPlane, bottomPlane, nearPlane, farPlane);
  terrain->Update(timeSinceStart);
  //for (auto chunk : chunks) {
  //  chunk->Update(timeSinceStart);
  //}
  for (auto gameObject : _gameObjects) {
    gameObject->Update(timeSinceStart);
  }
  for (auto gameObject : bones) {
    gameObject->Update(timeSinceStart);
  }
  skeleton->runAnimation(animation);
  skeleton->update(t);
}

void Application::shadowPass(ConstantBuffer& cb) {
  ID3D11ShaderResourceView* nullView = nullptr;
  _pImmediateContext->PSSetShaderResources(4, 1, &nullView);
  _pImmediateContext->OMSetRenderTargets(1, &null, lightDepthBuffer);
  _pImmediateContext->RSSetViewports(1, &lightVP);
  _pImmediateContext->ClearDepthStencilView(lightDepthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

  sphere bounds;
  bounds.centre = XMFLOAT3(0.0f, 0.0f, 0.0f);
  float halfWidth = TERRAIN_WIDTH / 2.0f;
  bounds.radius = sqrtf(halfWidth * halfWidth + halfWidth * halfWidth);

  XMVECTOR lightDir = XMLoadFloat3(&basicLight.LightVecW);
  XMVECTOR lightPos = 2.0f * bounds.radius * lightDir;
  XMVECTOR targetPos = XMLoadFloat3(&bounds.centre);
  XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

  XMMATRIX V = XMMatrixLookAtLH(lightPos, targetPos, up);

  XMFLOAT3 sphereCenterLS;
  XMStoreFloat3(&sphereCenterLS, XMVector3TransformCoord(targetPos, V));

  // Ortho frustum in light space encloses scene.
  float l = sphereCenterLS.x - bounds.radius;
  float b = sphereCenterLS.y - bounds.radius;
  float n = sphereCenterLS.z - bounds.radius;
  float r = sphereCenterLS.x + bounds.radius;
  float t = sphereCenterLS.y + bounds.radius;
  float f = sphereCenterLS.z + bounds.radius;
  XMMATRIX P = XMMatrixOrthographicOffCenterLH(l, r, b, t, n, f);

  XMMATRIX t1(0.5f, 0.0f, 0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f);
  XMMATRIX s = V * P * t1;

  cb.View = XMMatrixTranspose(V);
  cb.Projection = XMMatrixTranspose(P);
  cb.shadowTransform = XMMatrixTranspose(s);

  _pImmediateContext->PSSetShader(nullptr, nullptr, 0);

  _pImmediateContext->IASetInputLayout(_pVertexLayout);
  cb.World = XMMatrixTranspose(terrain->GetWorldMatrix());
  _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
  terrain->Draw(cb, _pConstantBuffer, _pImmediateContext);

  _pImmediateContext->IASetInputLayout(instanceLayout);
  _pImmediateContext->VSSetShader(instanceVertexShader, nullptr, 0);

  for (auto chunk : chunks) {
    chunk->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  _pImmediateContext->IASetInputLayout(_pVertexLayout);
  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);

  // Render all scene objects
  for (int i = 0; i < _gameObjects.size(); i++) {

    // Set world matrix
    cb.World = XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

    // Update constant buffer
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    _gameObjects[i]->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  for (int i = 0; i < bones.size(); i++) {
    // Get render material
    //material = _gameObjects[i]->GetMaterial();

    //// Copy material to shader
    //cb.surface.AmbientMtrl = material.ambient;
    //cb.surface.DiffuseMtrl = material.diffuse;
    //cb.surface.SpecularMtrl = material.specular;

    // Set world matrix
    cb.World = XMMatrixTranspose(bones[i]->GetWorldMatrix());

    // Set texture
    //if (_gameObjects[i]->HasTexture()) {
    //  ID3D11ShaderResourceView * textureRV = _gameObjects[i]->GetTextureRV();
    //  _pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
    //  cb.HasTexture = 1.0f;
    //}
    //else {
    cb.HasTexture = 0.0f;
    //}

    // Update constant buffer
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    bones[i]->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  _pImmediateContext->IASetInputLayout(instanceLayout);
  _pImmediateContext->VSSetShader(instanceVertexShader, nullptr, 0);
  skeleton->draw(cb, _pConstantBuffer);
}

void Application::renderToTextures(ConstantBuffer& cb) {
  ID3D11ShaderResourceView* nullView = nullptr;
  _pImmediateContext->PSSetShaderResources(0, 1, &nullView);
  _pImmediateContext->PSSetShaderResources(1, 1, &nullView);
  _pImmediateContext->OMSetRenderTargets(DEFERRED_BUFFERS, deferredRenderTargets, _depthStencilView);
  float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha

  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    _pImmediateContext->ClearRenderTargetView(deferredRenderTargets[i], ClearColor);
  }
  _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  if (_wireFrame) {
    _pImmediateContext->RSSetState(wireframe);
  }
  else {
    _pImmediateContext->RSSetState(CWcullMode);
  }

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->GetView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->GetProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

  cb.View = XMMatrixTranspose(view);
  cb.Projection = XMMatrixTranspose(projection);

  // Get render material
  Material material = terrain->GetMaterial();

  // Copy material to shader
  cb.surface.AmbientMtrl = material.ambient;
  cb.surface.DiffuseMtrl = material.diffuse;
  cb.surface.SpecularMtrl = material.specular;

  // Set world matrix
  cb.World = XMMatrixTranspose(terrain->GetWorldMatrix());
  cb.HasTexture = 1.0f;
  _pImmediateContext->PSSetShader(deferredTerrainPixelShader, nullptr, 0);
  _pImmediateContext->PSSetShaderResources(0, 1, &terrainTextureRV1);
  _pImmediateContext->PSSetShaderResources(1, 1, &terrainTextureRV2);
  _pImmediateContext->PSSetShaderResources(2, 1, &terrainTextureRV3);
  _pImmediateContext->PSSetShaderResources(3, 1, &terrainTextureRV4);

  // Update constant buffer
  _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

  _pImmediateContext->IASetInputLayout(_pVertexLayout);

  // Draw object
  terrain->Draw(cb, _pConstantBuffer, _pImmediateContext);

  _pImmediateContext->PSSetShader(deferredPixelShader, nullptr, 0);
  _pImmediateContext->IASetInputLayout(instanceLayout);
  _pImmediateContext->VSSetShader(instanceVertexShader, nullptr, 0);

  for (auto chunk : chunks) {
    chunk->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  _pImmediateContext->IASetInputLayout(_pVertexLayout);
  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);

  // Render all scene objects
  for (int i = 0; i < _gameObjects.size(); i++) {
    // Get render material
    //material = _gameObjects[i]->GetMaterial();

    //// Copy material to shader
    //cb.surface.AmbientMtrl = material.ambient;
    //cb.surface.DiffuseMtrl = material.diffuse;
    //cb.surface.SpecularMtrl = material.specular;

    // Set world matrix
    cb.World = XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

    // Set texture
    if (_gameObjects[i]->HasTexture()) {
      ID3D11ShaderResourceView * textureRV = _gameObjects[i]->GetTextureRV();
      _pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
      cb.HasTexture = 1.0f;
    }
    else {
      cb.HasTexture = 0.0f;
    }

    // Update constant buffer
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    _gameObjects[i]->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  for (int i = 0; i < bones.size(); i++) {
    // Get render material
    //material = _gameObjects[i]->GetMaterial();

    //// Copy material to shader
    //cb.surface.AmbientMtrl = material.ambient;
    //cb.surface.DiffuseMtrl = material.diffuse;
    //cb.surface.SpecularMtrl = material.specular;

    // Set world matrix
    cb.World = XMMatrixTranspose(bones[i]->GetWorldMatrix());

    // Set texture
    //if (_gameObjects[i]->HasTexture()) {
    //  ID3D11ShaderResourceView * textureRV = _gameObjects[i]->GetTextureRV();
    //  _pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
    //  cb.HasTexture = 1.0f;
    //}
    //else {
    cb.HasTexture = 0.0f;
    //}

    // Update constant buffer
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    bones[i]->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  _pImmediateContext->IASetInputLayout(instanceLayout);
  _pImmediateContext->VSSetShader(instanceVertexShader, nullptr, 0);
  skeleton->draw(cb, _pConstantBuffer);
}

void Application::shadowMapping() {
  ConstantBuffer cb;

  _pImmediateContext->RSSetState(CWcullMode);
  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
  //_pImmediateContext->HSSetShader(controlPointHullShader, nullptr, 0);
  //_pImmediateContext->DSSetShader(domainShader, nullptr, 0);
  _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
  _pImmediateContext->PSSetSamplers(1, 1, &shadowSampler);
  _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  cb.light = basicLight;
  cb.EyePosW = cameras[selectedCamera]->GetPosition();
  cb.gMaxTessFactor = 1.0f;
  cb.gMinTessFactor = 1.0f;
  cb.gMaxTessDistance = 1.0f;
  cb.gMinTessDistance = 10.0f;

  shadowPass(cb);

  if (_wireFrame) {
    _pImmediateContext->RSSetState(wireframe);
  }
  else {
    _pImmediateContext->RSSetState(CWcullMode);
  }

  _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, _depthStencilView); 
  _pImmediateContext->RSSetViewports(1, &vp);
  float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
  _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
  _pImmediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  _pImmediateContext->PSSetShaderResources(4, 1, &lightDepthView);

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->GetView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->GetProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

  cb.View = XMMatrixTranspose(view);
  cb.Projection = XMMatrixTranspose(projection);

  Material material = terrain->GetMaterial();
  cb.surface.AmbientMtrl = material.ambient;
  cb.surface.DiffuseMtrl = material.diffuse;
  cb.surface.SpecularMtrl = material.specular;
  cb.World = XMMatrixTranspose(terrain->GetWorldMatrix());
  cb.HasTexture = 1.0f;
  _pImmediateContext->PSSetShader(shadowTerrainPixelShader, nullptr, 0);
  _pImmediateContext->PSSetShaderResources(3, 1, &terrainTextureRV1);
  _pImmediateContext->PSSetShaderResources(2, 1, &terrainTextureRV2);
  _pImmediateContext->PSSetShaderResources(1, 1, &terrainTextureRV3);
  _pImmediateContext->PSSetShaderResources(0, 1, &terrainTextureRV4);
  _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);
  _pImmediateContext->IASetInputLayout(_pVertexLayout);
  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);

  // Draw object
  terrain->Draw(cb, _pConstantBuffer, _pImmediateContext);

  _pImmediateContext->PSSetShader(shadowPixelShader, nullptr, 0);

  _pImmediateContext->IASetInputLayout(instanceLayout);
  _pImmediateContext->VSSetShader(instanceVertexShader, nullptr, 0);

  for (auto chunk : chunks) {
    chunk->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  _pImmediateContext->IASetInputLayout(_pVertexLayout);
  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);

  // Render all scene objects
  for (int i = 0; i < _gameObjects.size(); i++) {
    // Get render material
    material = _gameObjects[i]->GetMaterial();

    // Copy material to shader
    cb.surface.AmbientMtrl = material.ambient;
    cb.surface.DiffuseMtrl = material.diffuse;
    cb.surface.SpecularMtrl = material.specular;

    // Set world matrix
    cb.World = XMMatrixTranspose(_gameObjects[i]->GetWorldMatrix());

    // Set texture
    if (_gameObjects[i]->HasTexture()) {
      ID3D11ShaderResourceView * textureRV = _gameObjects[i]->GetTextureRV();
      _pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
      cb.HasTexture = 1.0f;
    }
    else {
      cb.HasTexture = 0.0f;
    }

    // Update constant buffer
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    _gameObjects[i]->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  for (int i = 0; i < bones.size(); i++) {
    // Get render material
    //material = _gameObjects[i]->GetMaterial();

    //// Copy material to shader
    //cb.surface.AmbientMtrl = material.ambient;
    //cb.surface.DiffuseMtrl = material.diffuse;
    //cb.surface.SpecularMtrl = material.specular;

    // Set world matrix
    cb.World = XMMatrixTranspose(bones[i]->GetWorldMatrix());

    // Set texture
    //if (_gameObjects[i]->HasTexture()) {
    //  ID3D11ShaderResourceView * textureRV = _gameObjects[i]->GetTextureRV();
    //  _pImmediateContext->PSSetShaderResources(0, 1, &textureRV);
    //  cb.HasTexture = 1.0f;
    //}
    //else {
    cb.HasTexture = 0.0f;
    //}

    // Update constant buffer
    _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    bones[i]->Draw(cb, _pConstantBuffer, _pImmediateContext);
  }

  _pImmediateContext->IASetInputLayout(instanceLayout);
  _pImmediateContext->VSSetShader(instanceVertexShader, nullptr, 0);
  skeleton->draw(cb, _pConstantBuffer);
}

void Application::deferredRendering() {
  ConstantBuffer cb;

  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
  _pImmediateContext->HSSetShader(controlPointHullShader, nullptr, 0);
  _pImmediateContext->DSSetShader(domainShader, nullptr, 0);
  _pImmediateContext->VSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->PSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->HSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->DSSetConstantBuffers(0, 1, &_pConstantBuffer);
  _pImmediateContext->PSSetSamplers(0, 1, &_pSamplerLinear);
  _pImmediateContext->PSSetSamplers(1, 1, &shadowSampler);
  _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

  cb.light = basicLight;
  cb.EyePosW = cameras[selectedCamera]->GetPosition();
  cb.gMaxTessFactor = 1.0f;
  cb.gMinTessFactor = 1.0f;
  cb.gMaxTessDistance = 1.0f;
  cb.gMinTessDistance = 10.0f;

  renderToTextures(cb);

  _pImmediateContext->VSSetShader(_pVertexShader, nullptr, 0);
  _pImmediateContext->HSSetShader(nullptr, nullptr, 0);
  _pImmediateContext->DSSetShader(nullptr, nullptr, 0);
  _pImmediateContext->PSSetShader(lightingPixelShader, nullptr, 0);

  _pImmediateContext->OMSetRenderTargets(1, &_pRenderTargetView, nullptr);
  float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha
  _pImmediateContext->ClearRenderTargetView(_pRenderTargetView, ClearColor);
  _pImmediateContext->RSSetState(CWcullMode);
  _pImmediateContext->PSSetShaderResources(0, 1, &deferredResourceViews[0]);
  _pImmediateContext->PSSetShaderResources(1, 1, &deferredResourceViews[1]);
  _pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->GetBasicView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->getOrthoProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);
  XMMATRIX world = XMMatrixIdentity();

  cb.World = XMMatrixTranspose(world);
  cb.View = XMMatrixTranspose(view);
  cb.Projection = XMMatrixTranspose(projection);
  _pImmediateContext->UpdateSubresource(_pConstantBuffer, 0, nullptr, &cb, 0, 0);

  UINT stride = sizeof(SimpleVertex);
  UINT offset = 0;

  _pImmediateContext->IASetVertexBuffers(0, 1, &fullScreenVertexBuffer, &stride, &offset);
  _pImmediateContext->IASetIndexBuffer(fullScreenIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

  _pImmediateContext->DrawIndexed(6, 0, 0);
}

void Application::Draw() {
  //
  // Clear buffers
  //

  shadowMapping();
  //deferredRendering();

  //
  // Present our back buffer to our front buffer
  //
  _pSwapChain->Present(0, 0);
}