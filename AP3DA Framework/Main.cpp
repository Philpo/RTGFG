#include "Application.h"
#include <windowsx.h>

double timeLastFrame, frameRate, freq;
__int64 counterStart;

/*
* Taken from https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter
*/
HRESULT startCounter() {
  LARGE_INTEGER li;
  if (!QueryPerformanceFrequency(&li)) {
    return -1;
  }

  freq = double(li.QuadPart) / 1000.0;

  QueryPerformanceCounter(&li);
  counterStart = li.QuadPart;

  return S_OK;
}

/*
* Taken from https://stackoverflow.com/questions/1739259/how-to-use-queryperformancecounter
*/
double getCounter() {
  LARGE_INTEGER li;
  QueryPerformanceCounter(&li);
  return double(li.QuadPart - counterStart) / freq;
}

/*
* Taken from Frank Luna: 3D Game Programming with DirectX 11
*/
void calculateFrameRateStats(Application* const app) {
  static int frameCnt = 0;
  static double timeElapsed = 0.0;

  frameCnt++;

  if (getCounter() - timeElapsed >= 1.0f) {
    float fps = (float) frameCnt;
    float mspf = 1000.0f / fps;

    wostringstream outs;
    outs.precision(6);
    outs << L"Cork    " << L"FPS: " << fps << L"    " << L"Frame Time: " << mspf << L" (ms)";
    app->setWindowCaption(outs);

    frameCnt = 0;
    timeElapsed += 1000.0;
  }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  Application* theApp = new Application();

  if (FAILED(theApp->Initialise(hInstance, nCmdShow))) {
    return -1;
  }

  // Main message loop
  MSG msg = { 0 };
  
  startCounter();

  while (WM_QUIT != msg.message) {
    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      bool handled = false;

      if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
        handled = theApp->HandleKeyboard(msg);
      }
      else if (msg.message == WM_LBUTTONDOWN) {
        theApp->handleMouseClick(msg.wParam, GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
      }
      else if (msg.message == WM_MOUSEMOVE) {
        theApp->handleMouseMovement(msg.wParam, GET_X_LPARAM(msg.lParam), GET_Y_LPARAM(msg.lParam));
      }
      else if (WM_QUIT == msg.message)
        break;

      if (!handled) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    else {
      calculateFrameRateStats(theApp);
      theApp->Update();
      theApp->Draw();
    }
  }

  delete theApp;
  theApp = nullptr;

  return (int) msg.wParam;
}