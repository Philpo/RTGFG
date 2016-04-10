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

bool Application::handleKeyboard(MSG msg) {
  switch (msg.wParam) {
    case VK_UP:
      cameras[selectedCamera]->move(cameraSpeed * 0.2f, *terrain);
      return true;
    case VK_DOWN:
      cameras[selectedCamera]->move(cameraSpeed * -0.2f, *terrain);
      return true;
    case VK_RIGHT:
      cameras[selectedCamera]->strafe(cameraSpeed * 0.2f, *terrain);
      return true;
    case VK_LEFT:
      cameras[selectedCamera]->strafe(cameraSpeed * -0.2f, *terrain);
      return true;
    case VK_SHIFT:
      if (msg.message == WM_KEYDOWN) {
        wireFrame = !wireFrame;
      }
      return true;
    case VK_CONTROL:
      if (msg.message == WM_KEYDOWN) {
        deferredPipeline = !deferredPipeline;
      }
      return true;
    case VK_END:
      if (msg.message == WM_KEYDOWN) {
        perlinNoise = !perlinNoise;
        delete terrain;
        terrain = new Terrain(noSpecMaterial);

        if (perlinNoise) {
          terrain->perlinNoise(TERRAIN_DEPTH, TERRAIN_WIDTH, 6.0, 10.0, 1.0, 5.0);
        }
        else {
          terrain->diamondSquare(TERRAIN_DEPTH, time(0), 1.0f, 255.0f);
        }

        terrain->generateGeometry(TERRAIN_DEPTH, TERRAIN_WIDTH, 1.0f, 1.0f * tan(XM_PIDIV4), 16, renderHeight, d3dDevice, immediateContext, CELL_WIDTH, CELL_DEPTH);

        planeVertexBuffer->Release();

        if (FAILED(initTerrainVertexBuffer())) {
          cleanup();
          return E_FAIL;
        }

        planeGeometry.vertexBuffer = planeVertexBuffer;
        terrain->setGeometry(planeGeometry);
        terrain->setPosition(0.0f, 0.0f, 0.0f);

        skeleton->setTerrain(terrain);
        for (auto camera : cameras) {
          camera->move(0.0f, *terrain);
        }
      }
      return true;
    case VK_SPACE:
      cameras[selectedCamera]->moveY(cameraSpeed * 0.2f);
      return true;
    case VK_TAB:
      if (msg.message == WM_KEYDOWN) {
        selectedCamera = selectedCamera == cameras.size() - 1 ? 0 : selectedCamera + 1;
      }
      return true;
    case 0x57: // W
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->getOriginRotation();
        rotation.x += XMConvertToRadians(-10.0f);
        bones[1]->setOriginRotation(rotation);
      }
      return true;
    case 0x53: // S
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->getOriginRotation();
        rotation.x += XMConvertToRadians(10.0f);
        bones[1]->setOriginRotation(rotation);
      }
      return true;
    case 0x41: // A
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->getOriginRotation();
        rotation.y += XMConvertToRadians(-10.0f);
        bones[1]->setOriginRotation(rotation);
      }
      return true;
    case 0x44: // D
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[1]->getOriginRotation();
        rotation.y += XMConvertToRadians(10.0f);
        bones[1]->setOriginRotation(rotation);
      }
      return true;
    case 0x45: // E
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[0]->getOriginRotation();
        rotation.x += XMConvertToRadians(-10.0f);
        bones[0]->setOriginRotation(rotation);;
      }
      return true;
    case 0x51: // Q
      if (msg.message == WM_KEYDOWN) {
        XMFLOAT3 rotation = bones[0]->getOriginRotation();
        rotation.x += XMConvertToRadians(10.0f);
        bones[0]->setOriginRotation(rotation);
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

    terrain->cameraRotated();

    lastMousePosX = x;
    lastMousePosY = y;
  }
}

Application::Application() : lastMousePosX(0.0f), lastMousePosY(0.0f) {
  hInst = nullptr;
  hWnd = nullptr;
  driverType = D3D_DRIVER_TYPE_NULL;
  featureLevel = D3D_FEATURE_LEVEL_11_0;
  d3dDevice = nullptr;
  immediateContext = nullptr;
  swapChain = nullptr;
  renderTargetView = nullptr;
  vertexShader = nullptr;
  shadowPixelShader = nullptr;
  vertexLayout = nullptr;
  vertexBuffer = nullptr;
  indexBuffer = nullptr;
  constantBuffer = nullptr;

  dsLessEqual = nullptr;
  rsCullNone = nullptr;
}

Application::~Application() {
  cleanup();
}

HRESULT Application::initialise(HINSTANCE hInstance, int nCmdShow) {
  if (FAILED(initWindow(hInstance, nCmdShow))) {
    return E_FAIL;
  }

  AllocConsole();
  freopen("CONOUT$", "wb", stdout);

  RECT rc;
  GetClientRect(hWnd, &rc);
  windowWidth = rc.right - rc.left;
  windowHeight = rc.bottom - rc.top;

  noSpecMaterial.ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
  noSpecMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  noSpecMaterial.specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
  noSpecMaterial.specularPower = 0.0f;

  if (FAILED(initDevice())) {
    cleanup();

    return E_FAIL;
  }

  terrain = new Terrain(noSpecMaterial);
  terrain->perlinNoise(TERRAIN_DEPTH, TERRAIN_WIDTH, 6.0, 10.0, 1.0, 5.0);
  terrain->generateGeometry(TERRAIN_DEPTH, TERRAIN_WIDTH, 1.0f, 1.0f * tan(XM_PIDIV4), 16, renderHeight, d3dDevice, immediateContext, CELL_WIDTH, CELL_DEPTH);

  if (FAILED(initTerrainVertexBuffer())) {
    cleanup();
    return E_FAIL;
  }

  CreateDDSTextureFromFile(d3dDevice, L"Resources\\Crate_COLOR.dds", nullptr, &textureRV);
  CreateDDSTextureFromFile(d3dDevice, L"Resources\\darkdirt.dds", nullptr, &terrainTextureRV2);
  CreateDDSTextureFromFile(d3dDevice, L"Resources\\stone.dds", nullptr, &terrainTextureRV3);
  CreateDDSTextureFromFile(d3dDevice, L"Resources\\lightdirt.dds", nullptr, &terrainTextureRV1);
  CreateDDSTextureFromFile(d3dDevice, L"Resources\\snow.dds", nullptr, &terrainTextureRV4);

  // Setup Camera
  XMFLOAT3 eye = XMFLOAT3(0.0f, terrain->getCameraHeight(0.0f, -10.f) + 2.0f, -10.0f);
  XMFLOAT3 at = XMFLOAT3(0.0f, 0.0f, 1.0f);
  XMFLOAT3 up = XMFLOAT3(0.0f, 1.0f, 0.0f);

  cameras.push_back(new Camera(true, true, eye, at, up, (float) renderWidth, (float) renderHeight, 1.0f, 1000.0f));
  cameras.push_back(new Camera(true, false, eye, at, up, (float) renderWidth, (float) renderHeight, 1.0f, 1000.0f));

  // Setup the scene's light
  basicLight.ambientLight = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
  basicLight.diffuseLight = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  basicLight.specularLight = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
  basicLight.SpecularPower = 20.0f;
  basicLight.lightVecW = XMFLOAT3(-0.57735f, 0.57735f, -0.57735f);

  Geometry cubeGeometry;
  cubeGeometry.indexBuffer = indexBuffer;
  cubeGeometry.vertexBuffer = vertexBuffer;
  cubeGeometry.numberOfIndices = 36;
  cubeGeometry.vertexBufferOffset = 0;
  cubeGeometry.vertexBufferStride = sizeof(SimpleVertex);

  Geometry skeletonGeometry;
  skeletonGeometry.indexBuffer = indexBuffer;
  skeletonGeometry.vertexBuffer = skeletonVertexBuffer;
  skeletonGeometry.numberOfIndices = 36;
  skeletonGeometry.vertexBufferOffset = 0;
  skeletonGeometry.vertexBufferStride = sizeof(SimpleVertex);

  planeGeometry.vertexBuffer = planeVertexBuffer;
  planeGeometry.numberOfIndices = terrain->getNumIndices();
  planeGeometry.vertexBufferOffset = 0;
  planeGeometry.vertexBufferStride = sizeof(SimpleVertex);

  Material shinyMaterial;
  shinyMaterial.ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
  shinyMaterial.diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
  shinyMaterial.specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
  shinyMaterial.specularPower = 10.0f;

  terrain->setGeometry(planeGeometry);
  terrain->setPosition(0.0f, 0.0f, 0.0f);

  GameObject* root = new GameObject("root", skeletonGeometry, shinyMaterial);
  root->setPosition(0.0f, 2.0f, -2.0f);
  root->setScale(0.25f, 0.25f, 2.0f);
  root->setRotation(0.0f, 0.0f, 0.0f);
  bones.push_back(root);

  GameObject* node1 = new GameObject("node1", skeletonGeometry, shinyMaterial);
  node1->setParent(root);
  node1->setPosition(0.0f, 0.0f, 4.0f);
  node1->setRotation(0.0f, 0.0f, 0.0f);
  node1->setScale(0.25f, 0.25f, 2.0f);
  bones.push_back(node1);

  skeleton = new Skeleton(17, d3dDevice, immediateContext);

  bone1 = new GameObject("head", skeletonGeometry, shinyMaterial);
  bone1->setPosition(-4.0f, terrain->getCameraHeight(-4.0f, -2.0f) + 13.5f, -2.0f);
  bone1->setRotation(0.0f, 0.0f, 0.0f);
  skeleton->addBone(bone1);

  GameObject* bone2 = new GameObject("neck", skeletonGeometry, shinyMaterial);
  bone2->setParent(bone1);
  bone2->setPosition(0.0f, -1.0f, 1.0f);
  bone2->setRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
  bone2->setScale(0.25f, 0.25f, 0.5f);
  skeleton->addBone(bone2);

  GameObject* bone3 = new GameObject("rightshoulder", skeletonGeometry, shinyMaterial);
  bone3->setParent(bone2);
  bone3->setPosition(0.0f, -1.25f, 0.0f);
  bone3->setRotation(0.0f, XMConvertToRadians(90.0f), 0.0f);
  bone3->setScale(0.25f, 0.25f, 1.0f);
  skeleton->addBone(bone3);

  GameObject* bone4 = new GameObject("leftshoulder", skeletonGeometry, shinyMaterial);
  bone4->setParent(bone2);
  bone4->setPosition(0.0f, -1.25f, 0.0f);
  bone4->setRotation(0.0f, XMConvertToRadians(270.0f), 0.0f);
  bone4->setScale(0.25f, 0.25f, 1.0f);
  skeleton->addBone(bone4);

  GameObject* bone5 = new GameObject("rightupperarm", skeletonGeometry, shinyMaterial);
  bone5->setParent(bone3);
  bone5->setPosition(2.0f, 0.0f, 0.0f);
  bone5->setRotation(0.0f, XMConvertToRadians(90.0f), 0.0f);
  bone5->setScale(0.25f, 0.25f, 1.0f);
  skeleton->addBone(bone5);

  GameObject* bone6 = new GameObject("rightlowerarm", skeletonGeometry, shinyMaterial);
  bone6->setParent(bone5);
  bone6->setPosition(2.0f, 0.0f, 0.0f);
  bone6->setRotation(0.0f, XMConvertToRadians(90.0f), 0.0f);
  bone6->setScale(0.25f, 0.25f, 1.0f);
  skeleton->addBone(bone6);

  GameObject* bone7 = new GameObject("leftupperarm", skeletonGeometry, shinyMaterial);
  bone7->setParent(bone4);
  bone7->setPosition(-2.0f, 0.0f, 0.0f);
  bone7->setRotation(0.0f, XMConvertToRadians(270.0f), 0.0f);
  bone7->setScale(0.25f, 0.25f, 1.0f);
  skeleton->addBone(bone7);

  GameObject* bone8 = new GameObject("leftlowerarm", skeletonGeometry, shinyMaterial);
  bone8->setParent(bone7);
  bone8->setPosition(-2.0f, 0.0f, 0.0f);
  bone8->setRotation(0.0f, XMConvertToRadians(270.0f), 0.0f);
  bone8->setScale(0.25f, 0.25f, 1.0f);
  skeleton->addBone(bone8);

  GameObject* bone9 = new GameObject("back", skeletonGeometry, shinyMaterial);
  bone9->setParent(bone2);
  bone9->setPosition(0.0f, -1.5f, 0.0f);
  bone9->setRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
  bone9->setScale(0.25f, 0.25f, 2.0f);
  skeleton->addBone(bone9);

  GameObject* bone10 = new GameObject("righthip", skeletonGeometry, shinyMaterial);
  bone10->setParent(bone9);
  bone10->setPosition(0.0f, -4.25f, 0.0f);
  bone10->setRotation(0.0f, XMConvertToRadians(90.0f), 0.0f);
  bone10->setScale(0.25f, 0.25f, 0.5f);
  skeleton->addBone(bone10);

  GameObject* bone11 = new GameObject("lefthip", skeletonGeometry, shinyMaterial);
  bone11->setParent(bone9);
  bone11->setPosition(0.0f, -4.25f, 0.0f);
  bone11->setRotation(0.0f, XMConvertToRadians(270.0f), 0.0f);
  bone11->setScale(0.25f, 0.25f, 0.5f);
  skeleton->addBone(bone11);

  GameObject* bone12 = new GameObject("rightupperleg", skeletonGeometry, shinyMaterial);
  bone12->setParent(bone10);
  bone12->setPosition(1.25f, 0.25f, 0.0f);
  bone12->setRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
  bone12->setScale(0.25f, 0.25f, 1.5f);
  skeleton->addBone(bone12);

  GameObject* bone13 = new GameObject("rightlowerleg", skeletonGeometry, shinyMaterial);
  bone13->setParent(bone12);
  bone13->setPosition(0.0f, -3.0f, 0.0f);
  bone13->setRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
  bone13->setScale(0.25f, 0.25f, 1.5f);
  skeleton->addBone(bone13);

  GameObject* bone14 = new GameObject("leftupperleg", skeletonGeometry, shinyMaterial);
  bone14->setParent(bone11);
  bone14->setPosition(-1.25f, 0.25f, 0.0f);
  bone14->setRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
  bone14->setScale(0.25f, 0.25f, 1.5f);
  skeleton->addBone(bone14);

  GameObject* bone15 = new GameObject("leftlowerleg", skeletonGeometry, shinyMaterial);
  bone15->setParent(bone14);
  bone15->setPosition(0.0f, -3.0f, 0.0f);
  bone15->setRotation(XMConvertToRadians(90.0f), 0.0f, 0.0f);
  bone15->setScale(0.25f, 0.25f, 1.5f);
  skeleton->addBone(bone15);

  GameObject* bone16 = new GameObject("rightfoot", skeletonGeometry, shinyMaterial);
  bone16->setParent(bone13);
  bone16->setPosition(0.0f, -3.25f, -0.25f);
  bone16->setRotation(0.0f, 0.0f, 0.0f);
  bone16->setScale(0.25f, 0.25f, 0.75f);
  skeleton->addBone(bone16);

  GameObject* bone17 = new GameObject("leftfoot", skeletonGeometry, shinyMaterial);
  bone17->setParent(bone15);
  bone17->setPosition(0.0f, -3.25f, -0.25f);
  bone17->setRotation(0.0f, 0.0f, 0.0f);
  bone17->setScale(0.25f, 0.25f, 0.75f);
  skeleton->addBone(bone17);

  file<> file("test_animation.xml");
  xml_document<> doc;
  doc.parse<0>(file.data());
  xml_node<>* rootNode = doc.first_node();

  for (xml_node<>* frameNode = rootNode->first_node()->first_node(); frameNode; frameNode = frameNode->next_sibling()) {
    KeyFrame frame;
    frame.loadFrame(frameNode);
    animation.push_back(frame);
  }

  skeleton->setRoot(bone1);
  skeleton->setAnimation(animation);
  skeleton->addWaypoint(XMFLOAT3(-50.0f, 0.0f, 50.0f));
  skeleton->addWaypoint(XMFLOAT3(-50.0f, 0.0f, -50.0f));
  skeleton->addWaypoint(XMFLOAT3(50.0f, 0.0f, -50.0f));
  skeleton->addWaypoint(XMFLOAT3(50.0f, 0.0f, 50.0f));
  skeleton->setTerrain(terrain);

  return S_OK;
}

HRESULT Application::initShadersAndInputLayout() {
  HRESULT hr;

  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  hr = compileShaderFromFile(L"GeneralShaders.fx", "VS", "vs_4_0", &pVSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = d3dDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &vertexShader);

  if (FAILED(hr)) {
    pVSBlob->Release();
    return hr;
  }

  ID3DBlob* instanceVSBlob = nullptr;
  hr = compileShaderFromFile(L"GeneralShaders.fx", "INSTANCE_VS", "vs_4_0", &instanceVSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = d3dDevice->CreateVertexShader(instanceVSBlob->GetBufferPointer(), instanceVSBlob->GetBufferSize(), nullptr, &instanceVertexShader);

  if (FAILED(hr)) {
    instanceVSBlob->Release();
    return hr;
  }

  // Compile the hull shader
  ID3DBlob* pHSBlob = nullptr;

  hr = compileShaderFromFile(L"GeneralShaders.fx", "HS", "hs_5_0", &pHSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the control point hull shader
  hr = d3dDevice->CreateHullShader(pHSBlob->GetBufferPointer(), pHSBlob->GetBufferSize(), nullptr, &controlPointHullShader);

  if (FAILED(hr)) {
    pHSBlob->Release();
    return hr;
  }
  pHSBlob->Release();

  // Compile the domain shader
  ID3DBlob* pDSBlob = nullptr;
  hr = compileShaderFromFile(L"GeneralShaders.fx", "DS", "ds_5_0", &pDSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the domain shader
  hr = d3dDevice->CreateDomainShader(pDSBlob->GetBufferPointer(), pDSBlob->GetBufferSize(), nullptr, &domainShader);

  if (FAILED(hr)) {
    pDSBlob->Release();
    return hr;
  }
  pDSBlob->Release();

  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;
  hr = compileShaderFromFile(L"ShadowMapping.fx", "SHADOW_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shadowPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = compileShaderFromFile(L"ShadowMapping.fx", "SHADOW_TERRAIN_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &shadowTerrainPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = compileShaderFromFile(L"DeferredRendering.fx", "DEFERRED_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &deferredPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = compileShaderFromFile(L"DeferredRendering.fx", "DEFERRED_TERRAIN_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &deferredTerrainPixelShader);
  pPSBlob->Release();

  if (FAILED(hr))
    return hr;

  hr = compileShaderFromFile(L"DeferredRendering.fx", "LIGHTING_PS", "ps_4_0", &pPSBlob);

  if (FAILED(hr)) {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = d3dDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &lightingPixelShader);
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
  hr = d3dDevice->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), &vertexLayout);
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
  hr = d3dDevice->CreateInputLayout(instanceDesc, numElements, instanceVSBlob->GetBufferPointer(),
    instanceVSBlob->GetBufferSize(), &instanceLayout);
  instanceVSBlob->Release();

  if (FAILED(hr))
    return hr;

  // Set the input layout
  immediateContext->IASetInputLayout(vertexLayout);

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
  hr = d3dDevice->CreateSamplerState(&sampDesc, &samplerAnistropic);

  sampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
  sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  sampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;

  hr = d3dDevice->CreateSamplerState(&sampDesc, &shadowSampler);

  return hr;
}

HRESULT Application::initVertexBuffer() {
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

  hr = d3dDevice->CreateBuffer(&bd, &InitData, &vertexBuffer);

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

  hr = d3dDevice->CreateBuffer(&bd, &InitData, &skeletonVertexBuffer);

  if (FAILED(hr))
    return hr;

  float left = (float) ((renderWidth / 2.0f) * -1.0f);

  // Calculate the screen coordinates of the right side of the window.
  float right = left + (float) renderWidth;

  // Calculate the screen coordinates of the top of the window.
  float top = (float) (renderHeight / 2);

  // Calculate the screen coordinates of the bottom of the window.
  float bottom = top - (float) renderHeight;

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

  hr = d3dDevice->CreateBuffer(&bd, &InitData, &fullScreenVertexBuffer);

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

  HRESULT hr = d3dDevice->CreateBuffer(&bd, &InitData, &planeVertexBuffer);

  if (FAILED(hr))
    return hr;
}

HRESULT Application::initIndexBuffer() {
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
  hr = d3dDevice->CreateBuffer(&bd, &InitData, &indexBuffer);

  if (FAILED(hr))
    return hr;

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
  hr = d3dDevice->CreateBuffer(&bd, &InitData, &fullScreenIndexBuffer);

  if (FAILED(hr))
    return hr;

  return S_OK;
}

HRESULT Application::initWindow(HINSTANCE hInstance, int nCmdShow) {
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
  hInst = hInstance;
  RECT rc = { 0, 0, 960, 540 };
  AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
  hWnd = CreateWindow(L"TutorialWindowClass", L"FGGC Semester 2 Framework", WS_OVERLAPPEDWINDOW,
    CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance,
    nullptr);
  if (!hWnd)
    return E_FAIL;

  ShowWindow(hWnd, nCmdShow);

  return S_OK;
}

HRESULT Application::compileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut) {
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

HRESULT Application::initDevice() {
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
  sd.BufferDesc.Width = renderWidth;
  sd.BufferDesc.Height = renderHeight;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = sampleCount;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;

  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
    driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDeviceAndSwapChain(nullptr, driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION, &sd, &swapChain, &d3dDevice, &featureLevel, &immediateContext);
    if (SUCCEEDED(hr))
      break;
  }

  if (FAILED(hr))
    return hr;

  // Create a render target view
  ID3D11Texture2D* pBackBuffer = nullptr;
  hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*) &pBackBuffer);

  if (FAILED(hr))
    return hr;

  hr = d3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &renderTargetView);
  pBackBuffer->Release();

  if (FAILED(hr))
    return hr;

  // create deferred rendering resources

  D3D11_TEXTURE2D_DESC textureDesc;
  D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
  D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;

  ZeroMemory(&textureDesc, sizeof(textureDesc));
  textureDesc.Width = renderWidth;
  textureDesc.Height = renderHeight;
  textureDesc.MipLevels = 1;
  textureDesc.ArraySize = 1;
  textureDesc.Format = DXGI_FORMAT_R8G8B8A8_SNORM;
  textureDesc.SampleDesc.Count = 1;
  textureDesc.Usage = D3D11_USAGE_DEFAULT;
  textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  textureDesc.CPUAccessFlags = 0;
  textureDesc.MiscFlags = 0;

  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    hr = d3dDevice->CreateTexture2D(&textureDesc, NULL, &deferredTextures[i]);
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
    hr = d3dDevice->CreateRenderTargetView(deferredTextures[i], &renderTargetViewDesc, &deferredRenderTargets[i]);
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
    hr = d3dDevice->CreateShaderResourceView(deferredTextures[i], &shaderResourceViewDesc, &deferredResourceViews[i]);
    if (FAILED(hr)) {
      return false;
    }
  }

  // Setup the viewport
  vp.Width = (FLOAT) renderWidth;
  vp.Height = (FLOAT) renderHeight;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  immediateContext->RSSetViewports(1, &vp);

  lightVP.Width = 8096.0f;
  lightVP.Height = 8096.0f;
  lightVP.MinDepth = 0.0f;
  lightVP.MaxDepth = 1.0f;
  lightVP.TopLeftX = 0;
  lightVP.TopLeftY = 0;

  initShadersAndInputLayout();

  initVertexBuffer();
  initIndexBuffer();

  // Set primitive topology
  //_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

  // Create the constant buffer
  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(ConstantBuffer);
  bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  bd.CPUAccessFlags = 0;
  hr = d3dDevice->CreateBuffer(&bd, nullptr, &constantBuffer);

  if (FAILED(hr))
    return hr;

  D3D11_TEXTURE2D_DESC depthStencilDesc;

  depthStencilDesc.Width = renderWidth;
  depthStencilDesc.Height = renderHeight;
  depthStencilDesc.MipLevels = 1;
  depthStencilDesc.ArraySize = 1;
  depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilDesc.SampleDesc.Count = sampleCount;
  depthStencilDesc.SampleDesc.Quality = 0;
  depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
  depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthStencilDesc.CPUAccessFlags = 0;
  depthStencilDesc.MiscFlags = 0;

  d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer);
  d3dDevice->CreateDepthStencilView(depthStencilBuffer, nullptr, &depthStencilView);

  immediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView);

  depthStencilDesc.Width = 8096;
  depthStencilDesc.Height = 8096;
  depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
  depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;

  ID3D11Texture2D* lightTex;
  hr = d3dDevice->CreateTexture2D(&depthStencilDesc, nullptr, &lightTex);

  if (FAILED(hr)) {
    return hr;
  }

  D3D11_DEPTH_STENCIL_VIEW_DESC viewDesc;
  viewDesc.Flags = 0;
  viewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  viewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  viewDesc.Texture2D.MipSlice = 0;

  hr = d3dDevice->CreateDepthStencilView(lightTex, &viewDesc, &lightDepthBuffer);

  if (FAILED(hr)) {
    return hr;
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC rViewDesc;
  rViewDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
  rViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  rViewDesc.Texture2D.MipLevels = depthStencilDesc.MipLevels;
  rViewDesc.Texture2D.MostDetailedMip = 0;

  hr = d3dDevice->CreateShaderResourceView(lightTex, &rViewDesc, &lightDepthView);

  if (FAILED(hr)) {
    return hr;
  }

  lightTex->Release();

  // Rasterizer
  D3D11_RASTERIZER_DESC cmdesc;

  ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.CullMode = D3D11_CULL_NONE;
  hr = d3dDevice->CreateRasterizerState(&cmdesc, &rsCullNone);

  ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));
  cmdesc.FillMode = D3D11_FILL_WIREFRAME;
  cmdesc.CullMode = D3D11_CULL_NONE;
  hr = d3dDevice->CreateRasterizerState(&cmdesc, &rsCullNoneWireFrame);

  D3D11_DEPTH_STENCIL_DESC dssDesc;
  ZeroMemory(&dssDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
  dssDesc.DepthEnable = true;
  dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

  d3dDevice->CreateDepthStencilState(&dssDesc, &dsLessEqual);

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

  d3dDevice->CreateBlendState(&blendDesc, &transparency);

  ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

  cmdesc.FillMode = D3D11_FILL_SOLID;
  cmdesc.CullMode = D3D11_CULL_BACK;

  cmdesc.FrontCounterClockwise = true;
  hr = d3dDevice->CreateRasterizerState(&cmdesc, &ccwCullMode);

  cmdesc.FrontCounterClockwise = false;
  hr = d3dDevice->CreateRasterizerState(&cmdesc, &cwCullMode);

  immediateContext->RSSetState(cwCullMode);

  cmdesc.FillMode = D3D11_FILL_WIREFRAME;
  cmdesc.CullMode = D3D11_CULL_NONE;
  hr = d3dDevice->CreateRasterizerState(&cmdesc, &wireframe);

  return S_OK;
}

void Application::cleanup() {
  if (immediateContext) immediateContext->ClearState();
  if (samplerAnistropic) samplerAnistropic->Release();
  if (shadowSampler) shadowSampler->Release();

  if (textureRV) textureRV->Release();
  if (terrainTextureRV1) terrainTextureRV1->Release();
  if (terrainTextureRV2) terrainTextureRV2->Release();
  if (terrainTextureRV3) terrainTextureRV3->Release();
  if (terrainTextureRV4) terrainTextureRV4->Release();
  if (lightDepthView) lightDepthView->Release();
  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    deferredResourceViews[i]->Release();
  }

  if (constantBuffer) constantBuffer->Release();

  if (vertexBuffer) vertexBuffer->Release();
  if (indexBuffer) indexBuffer->Release();
  if (planeVertexBuffer) planeVertexBuffer->Release();
  if (skeletonVertexBuffer) skeletonVertexBuffer->Release();
  if (fullScreenVertexBuffer) fullScreenVertexBuffer->Release();
  if (fullScreenIndexBuffer) fullScreenIndexBuffer->Release();

  if (vertexLayout) vertexLayout->Release();
  if (instanceLayout) instanceLayout->Release();
  if (vertexShader) vertexShader->Release();
  if (instanceVertexShader) instanceVertexShader->Release();
  if (controlPointHullShader) controlPointHullShader->Release();
  if (domainShader) domainShader->Release();
  if (shadowPixelShader) shadowPixelShader->Release();
  if (shadowTerrainPixelShader) shadowTerrainPixelShader->Release();
  if (deferredPixelShader) deferredPixelShader->Release();
  if (deferredTerrainPixelShader) deferredTerrainPixelShader->Release();
  if (lightingPixelShader) lightingPixelShader->Release();
  if (renderTargetView) renderTargetView->Release();
  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    deferredRenderTargets[i]->Release();
  }
  if (swapChain) swapChain->Release();
  if (immediateContext) immediateContext->Release();
  if (d3dDevice) d3dDevice->Release();
  if (depthStencilView) depthStencilView->Release();
  if (lightDepthView) lightDepthView->Release();
  if (depthStencilBuffer) depthStencilBuffer->Release();
  if (lightDepthBuffer) lightDepthBuffer->Release();

  if (dsLessEqual) dsLessEqual->Release();
  if (rsCullNone) rsCullNone->Release();
  if (rsCullNoneWireFrame) rsCullNoneWireFrame->Release();

  if (transparency) transparency->Release();
  if (ccwCullMode) ccwCullMode->Release();
  if (cwCullMode) cwCullMode->Release();
  if (wireframe) wireframe->Release();

  for (auto camera : cameras) {
    delete camera;
  }
  for (auto bone : bones) {
    delete bone;
  }
  delete skeleton;
  delete terrain;
}

void Application::update() {
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
  cameras[selectedCamera]->update();

  if (!deferredPipeline) {
    XMVECTOR toLight = XMLoadFloat3(&XMFLOAT3(-0.57735f, 0.57735f, -0.57735f));
    toLight = XMVector3Transform(toLight, XMMatrixRotationY(t));
    XMStoreFloat3(&basicLight.lightVecW, toLight);
  }
  else {
    basicLight.lightVecW = XMFLOAT3(-0.57735f, 0.57735f, -0.57735f);
  }

  // Update objects

  // extract frustum planes
  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->getView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->getProjection();

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

  XMFLOAT4 planes[6];
  planes[0] = leftPlane;
  planes[1] = rightPlane;
  planes[2] = topPlane;
  planes[3] = bottomPlane;
  planes[4] = nearPlane;
  planes[5] = farPlane;

  terrain->setCameraPosition(cameras[selectedCamera]->getPosition());
  terrain->frustumCull(planes);
  terrain->update(timeSinceStart);

  for (auto gameObject : bones) {
    gameObject->update(timeSinceStart);
  }

  skeleton->walk();
  skeleton->update(t);
}

// renders the scene from the light's perspective to create the shadow map
void Application::shadowPass(ConstantBuffer& cb) {
  // unbind depth map from pixel shader, since we're now writing to it
  ID3D11ShaderResourceView* nullView = nullptr;
  immediateContext->PSSetShaderResources(4, 1, &nullView);
  // depth only writing
  immediateContext->OMSetRenderTargets(1, &null, lightDepthBuffer);
  immediateContext->RSSetViewports(1, &lightVP);
  immediateContext->ClearDepthStencilView(lightDepthBuffer, D3D11_CLEAR_DEPTH, 1.0f, 0);

  // taken from Frank Luna 3D Game Programming with DirectX 11
  sphere bounds;
  bounds.centre = XMFLOAT3(0.0f, 0.0f, 0.0f);
  float halfWidth = TERRAIN_WIDTH / 2.0f;
  bounds.radius = sqrtf(halfWidth * halfWidth + halfWidth * halfWidth);

  XMVECTOR lightDir = XMLoadFloat3(&basicLight.lightVecW);
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

  cb.view = XMMatrixTranspose(V);
  cb.projection = XMMatrixTranspose(P);
  // projective texturing matrix
  cb.shadowTransform = XMMatrixTranspose(s);

  // depth only rendering, so no need for a pixel shader
  immediateContext->PSSetShader(nullptr, nullptr, 0);

  immediateContext->IASetInputLayout(vertexLayout);
  cb.world = XMMatrixTranspose(terrain->getWorldMatrix());
  immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
  terrain->draw(cb, constantBuffer, immediateContext);

  for (int i = 0; i < bones.size(); i++) {
    cb.world = XMMatrixTranspose(bones[i]->getWorldMatrix());
    cb.hasTexture = 0.0f;

    // Update constant buffer
    immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    bones[i]->draw(cb, constantBuffer, immediateContext);
  }

  immediateContext->IASetInputLayout(instanceLayout);
  immediateContext->VSSetShader(instanceVertexShader, nullptr, 0);
  skeleton->draw(cb, constantBuffer);
}

// deferred rendering pass
void Application::renderToTextures(ConstantBuffer& cb) {
  // unbind G-buffers from pixel shader as we're now writing to them
  ID3D11ShaderResourceView* nullView = nullptr;
  immediateContext->PSSetShaderResources(0, 1, &nullView);
  immediateContext->PSSetShaderResources(1, 1, &nullView);
  immediateContext->OMSetRenderTargets(DEFERRED_BUFFERS, deferredRenderTargets, depthStencilView);
  float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f }; // red,green,blue,alpha

  for (int i = 0; i < DEFERRED_BUFFERS; i++) {
    immediateContext->ClearRenderTargetView(deferredRenderTargets[i], ClearColor);
  }
  immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

  if (wireFrame) {
    immediateContext->RSSetState(wireframe);
  }
  else {
    immediateContext->RSSetState(cwCullMode);
  }

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->getView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->getProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

  cb.view = XMMatrixTranspose(view);
  cb.projection = XMMatrixTranspose(projection);

  // Get render material
  Material material = terrain->getMaterial();

  // Copy material to shader
  cb.surface.ambientMtrl = material.ambient;
  cb.surface.diffuseMtrl = material.diffuse;
  cb.surface.specularMtrl = material.specular;

  // Set world matrix
  cb.world = XMMatrixTranspose(terrain->getWorldMatrix());
  cb.hasTexture = 1.0f;
  immediateContext->PSSetShader(deferredTerrainPixelShader, nullptr, 0);
  immediateContext->PSSetShaderResources(3, 1, &terrainTextureRV1);
  immediateContext->PSSetShaderResources(2, 1, &terrainTextureRV2);
  immediateContext->PSSetShaderResources(1, 1, &terrainTextureRV3);
  immediateContext->PSSetShaderResources(0, 1, &terrainTextureRV4);

  // Update constant buffer
  immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

  immediateContext->IASetInputLayout(vertexLayout);

  // Draw object
  terrain->draw(cb, constantBuffer, immediateContext);

  immediateContext->PSSetShader(deferredPixelShader, nullptr, 0);

  for (int i = 0; i < bones.size(); i++) {
    cb.world = XMMatrixTranspose(bones[i]->getWorldMatrix());
    cb.hasTexture = 0.0f;

    // Update constant buffer
    immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    bones[i]->draw(cb, constantBuffer, immediateContext);
  }

  immediateContext->IASetInputLayout(instanceLayout);
  immediateContext->VSSetShader(instanceVertexShader, nullptr, 0);
  skeleton->draw(cb, constantBuffer);
}

void Application::shadowMapping() {
  ConstantBuffer cb;

  immediateContext->RSSetState(cwCullMode);
  immediateContext->VSSetShader(vertexShader, nullptr, 0);
  immediateContext->HSSetShader(controlPointHullShader, nullptr, 0);
  immediateContext->DSSetShader(domainShader, nullptr, 0);
  immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->HSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->DSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->PSSetSamplers(0, 1, &samplerAnistropic);
  immediateContext->PSSetSamplers(1, 1, &shadowSampler);
  immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

  cb.light = basicLight;
  cb.eyePosW = cameras[selectedCamera]->getPosition();
  cb.maxTessFactor = 1.0f;
  cb.minTessFactor = 1.0f;
  cb.maxTessDistance = 1.0f;
  cb.minTessDistance = 10.0f;

  // first create the shadow map
  shadowPass(cb);

  if (wireFrame) {
    immediateContext->RSSetState(wireframe);
  }
  else {
    immediateContext->RSSetState(cwCullMode);
  }

  immediateContext->OMSetRenderTargets(1, &renderTargetView, depthStencilView); 
  immediateContext->RSSetViewports(1, &vp);
  float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
  immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
  immediateContext->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
  // bind the shadow map as input to the pixel shader
  immediateContext->PSSetShaderResources(4, 1, &lightDepthView);

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->getView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->getProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);

  cb.view = XMMatrixTranspose(view);
  cb.projection = XMMatrixTranspose(projection);

  Material material = terrain->getMaterial();
  cb.surface.ambientMtrl = material.ambient;
  cb.surface.diffuseMtrl = material.diffuse;
  cb.surface.specularMtrl = material.specular;
  cb.world = XMMatrixTranspose(terrain->getWorldMatrix());
  cb.hasTexture = 1.0f;
  immediateContext->PSSetShader(shadowTerrainPixelShader, nullptr, 0);
  immediateContext->PSSetShaderResources(3, 1, &terrainTextureRV1);
  immediateContext->PSSetShaderResources(2, 1, &terrainTextureRV2);
  immediateContext->PSSetShaderResources(1, 1, &terrainTextureRV3);
  immediateContext->PSSetShaderResources(0, 1, &terrainTextureRV4);
  immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);
  immediateContext->IASetInputLayout(vertexLayout);
  immediateContext->VSSetShader(vertexShader, nullptr, 0);

  // Draw object
  terrain->draw(cb, constantBuffer, immediateContext);

  immediateContext->PSSetShader(shadowPixelShader, nullptr, 0);

  for (int i = 0; i < bones.size(); i++) {
    cb.world = XMMatrixTranspose(bones[i]->getWorldMatrix());
    cb.hasTexture = 0.0f;

    // Update constant buffer
    immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

    // Draw object
    bones[i]->draw(cb, constantBuffer, immediateContext);
  }

  immediateContext->IASetInputLayout(instanceLayout);
  immediateContext->VSSetShader(instanceVertexShader, nullptr, 0);
  skeleton->draw(cb, constantBuffer);
}

void Application::deferredRendering() {
  ConstantBuffer cb;

  immediateContext->VSSetShader(vertexShader, nullptr, 0);
  immediateContext->HSSetShader(controlPointHullShader, nullptr, 0);
  immediateContext->DSSetShader(domainShader, nullptr, 0);
  immediateContext->VSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->PSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->HSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->DSSetConstantBuffers(0, 1, &constantBuffer);
  immediateContext->PSSetSamplers(0, 1, &samplerAnistropic);
  immediateContext->PSSetSamplers(1, 1, &shadowSampler);
  immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);

  cb.light = basicLight;
  cb.eyePosW = cameras[selectedCamera]->getPosition();
  cb.maxTessFactor = 1.0f;
  cb.minTessFactor = 1.0f;
  cb.maxTessDistance = 1.0f;
  cb.minTessDistance = 10.0f;

  // render to G-buffers
  renderToTextures(cb);

  immediateContext->VSSetShader(vertexShader, nullptr, 0);
  immediateContext->HSSetShader(nullptr, nullptr, 0);
  immediateContext->DSSetShader(nullptr, nullptr, 0);
  immediateContext->PSSetShader(lightingPixelShader, nullptr, 0);

  immediateContext->OMSetRenderTargets(1, &renderTargetView, nullptr);
  float ClearColor[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
  immediateContext->ClearRenderTargetView(renderTargetView, ClearColor);
  immediateContext->RSSetState(cwCullMode);
  // bind G-buffers as input to the pixel shader
  immediateContext->PSSetShaderResources(0, 1, &deferredResourceViews[0]);
  immediateContext->PSSetShaderResources(1, 1, &deferredResourceViews[1]);
  immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  XMFLOAT4X4 viewAsFloats = cameras[selectedCamera]->getBasicView();
  XMFLOAT4X4 projectionAsFloats = cameras[selectedCamera]->getOrthoProjection();

  XMMATRIX view = XMLoadFloat4x4(&viewAsFloats);
  XMMATRIX projection = XMLoadFloat4x4(&projectionAsFloats);
  XMMATRIX world = XMMatrixIdentity();

  cb.world = XMMatrixTranspose(world);
  cb.view = XMMatrixTranspose(view);
  cb.projection = XMMatrixTranspose(projection);
  immediateContext->UpdateSubresource(constantBuffer, 0, nullptr, &cb, 0, 0);

  UINT stride = sizeof(SimpleVertex);
  UINT offset = 0;

  immediateContext->IASetVertexBuffers(0, 1, &fullScreenVertexBuffer, &stride, &offset);
  immediateContext->IASetIndexBuffer(fullScreenIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

  immediateContext->DrawIndexed(6, 0, 0);
}

void Application::draw() {
  if (!deferredPipeline) {
    shadowMapping();
  }
  else {
    deferredRendering();
  }

  swapChain->Present(0, 0);
}