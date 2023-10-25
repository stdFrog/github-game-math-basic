#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <algorithm>
#include <vector>
#include <math.h>
#define CLASS_NAME TEXT("Example")

using namespace std;

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow){
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		LoadIcon(NULL, IDI_APPLICATION), LoadCursor(NULL ,IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME,
		NULL
	};

	RegisterClassEx(&wcex);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, NULL, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

HBITMAP hBit;

struct Vector{
	float x,y,z;
	Vector(float _x = 0.f, float _y = 0.f, float _z = 0.f) : x(_x), y(_y), z(_z) {;}
	Vector(POINT pt) : x((int)pt.x), y((int)pt.y), z(0.f) {;}

	Vector operator *(const float value) const {
		Vector v;
		v.x = x * value;
		v.y = y * value;
		return v;
	}

	float Squared(){
		return x * x + y * y;
	}

	float Length(){
		return sqrt(Squared());
	}

	void Normalize(){
		float ret = Length();
		if(ret < 0.000000f){return;}
		x /= ret;
		y /= ret;
	}
};

vector<Vector> Hearts;

void DrawLine(HDC hdc, Vector from, Vector to){
	MoveToEx(hdc, from.x, from.y, NULL);
	LineTo(hdc, to.x, to.y);
}

float Lerp(float p1, float p2, float d1){
	return (1-d1) * p1 + d1 * p2;
}

enum Axis {NONE=0, LEFT=-1, RIGHT=1, UP=3, DOWN=4};

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	HDC hdc, hMemDC;
	PAINTSTRUCT ps;
	HGDIOBJ hOld;
	BITMAP bmp;
	static RECT crt;
	static float Radius, PI, TwoPI;
	static float deltaSeconds, currentDegree, speed, sinf, cosf;
	static Axis Status;
	float deltaDegree;

	switch(iMessage){
		case WM_CREATE:
			SetRect(&crt, 0,0, 800, 600);
			AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);
			SetWindowPos(hWnd, NULL, crt.left, crt.top, crt.right - crt.left, crt.bottom - crt.top, SWP_NOMOVE | SWP_NOZORDER);
			PI = atan(1.0) * 4.0;
			TwoPI = PI * 2;
			speed = 10.f;
			Status = NONE;
			SetTimer(hWnd, 1, 100, NULL);
			return 0;

		case WM_KEYDOWN:
			switch(wParam){
				case VK_LEFT:
					Status = LEFT;
					break;
				case VK_RIGHT:
					Status = RIGHT;
					break;
				case VK_UP:
					Status = UP;
					break;
				case VK_DOWN:
					Status = DOWN;
					break;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;

		case WM_TIMER:
			switch(wParam){
				case 1:
					deltaSeconds+=0.1f;
					hdc = GetDC(hWnd);
					hMemDC = CreateCompatibleDC(hdc);
					
					if(hBit == NULL){
						hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
					}
					hOld = SelectObject(hMemDC, hBit);
					FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));
					
					SetViewportOrgEx(hMemDC, crt.right - crt.left >> 1, crt.bottom - crt.top >> 1, NULL);

					deltaDegree = Status * min(speed * 2.f, speed * deltaSeconds);
					currentDegree += deltaDegree;
					
					if(Hearts.empty()){
						for(float rad = 0.f; rad < TwoPI; rad+=0.001f){
							float sin1 = sin(rad);
							float cos1 = cos(rad);
							float cos2 = cos(2 * rad);
							float cos3 = cos(3 * rad);
							float cos4 = cos(4 * rad);

							float x = 16.f * sin1 * sin1 * sin1;
							float y = 13 * cos1 - 5 * cos2 - 2 * cos3 - cos4;
							Hearts.push_back(Vector(x,y));
						}
					}

					sinf = sin(currentDegree * PI / 180.f);
					cosf = cos(currentDegree * PI / 180.f);

					for(auto const &v : Hearts){
						Vector scale = v * 15.f;
						Vector rotated = Vector(scale.x * cosf - scale.y * sinf, scale.x * sinf + scale.y * cosf);
						SetPixel(hMemDC, rotated.x, rotated.y, RGB(0,0,0));
					}

					SelectObject(hMemDC, hOld);
					DeleteDC(hMemDC);
					ReleaseDC(hWnd, hdc);
					InvalidateRect(hWnd, NULL, FALSE);
					break;
			}
			return 0;
				
		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			if(hBit){
				GetObject(hBit, sizeof(BITMAP), &bmp);
				hMemDC = CreateCompatibleDC(hdc);
				hOld = SelectObject(hMemDC, hBit);

				BitBlt(hdc, 0,0, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);


				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
			}
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			if(hBit){DeleteObject(hBit);}
			KillTimer(hWnd, 1);
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
