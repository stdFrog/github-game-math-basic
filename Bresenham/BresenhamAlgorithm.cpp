#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <cmath>
#define CLASS_NAME TEXT("Bresenhams Algorithm")
#define SMALL_NUMBER 0.0f

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

/* Bresenhams Algorithm(Mid Point Algorithm) - Draw Line */
void myDrawLine(HDC hdc, const POINT &Start, const POINT &End);
bool CohenSutherlandLineClip(POINT &clipStart, POINT &clipEnd, const POINT &minScreen, const POINT &maxScreen);
int myRegion(const POINT &InVector, const POINT &Min, const POINT &Max);
bool EqualsInTolerance(float InFloat1, float InFloat2, float InTolerance = SMALL_NUMBER);


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

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	HDC hdc;
	PAINTSTRUCT ps;
	static POINT st1, ed1, st2, ed2;

	switch(iMessage){
		case WM_CREATE:
			st1 = {10,100};
			ed1 = {400,100};
			st2 = {10, 140};
			ed2 = {400, 140};
			return 0;

		case WM_PAINT:
			hdc = BeginPaint(hWnd, &ps);
			myDrawLine(hdc, st1, ed1);
			myDrawLine(hdc, st2, ed2);
			EndPaint(hWnd, &ps);
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

void myDrawLine(HDC hdc, const POINT &Start, const POINT &End){
	LONG ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	LONG ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	POINT clipStart = {Start.x, Start.y};
	POINT clipEnd = {End.x, End.y};
	POINT ScreenExtend = {(LONG)(ScreenWidth * 0.5f), (LONG)(ScreenHeight * 0.5f)};
	POINT minScreen = {-(ScreenExtend.x), -(ScreenExtend.y)};
	POINT maxScreen = {ScreenExtend.x, ScreenExtend.y};

	if(!CohenSutherlandLineClip(clipStart, clipEnd, minScreen, maxScreen)){
		return;
	}

	POINT StartPosition = {(LONG)(clipStart.x + ScreenWidth * 0.5f), (LONG)(clipStart.y + ScreenHeight * 0.5f)};
	POINT EndPosition = {(LONG)(StartPosition.x - ScreenWidth * 0.5f + 0.5f), LONG(-(StartPosition.y + 0.5f) + ScreenHeight * 0.5f)};

	int Width = EndPosition.x - StartPosition.x;
	int Height = EndPosition.y - StartPosition.y;

	BOOL isGradualSlope = (abs(Width) >= abs(Height));

	int dx = (Width >= 0) ? 1 : -1;
	int dy = (Height > 0) ? 1 : -1;
	int fw = dx * Width;
	int fh = dy * Height;

	int f = isGradualSlope ? fh * 2 - fw : 2 * fw - fh;
	int f1 = isGradualSlope ? 2 * fh : 2 * fw;
	int f2 = isGradualSlope ? 2 * (fh - fw) : 2 * (fw - fh);
	int x = StartPosition.x;
	int y = StartPosition.y;

	if(isGradualSlope){
		while(x != EndPosition.x){
			SetPixel(hdc, x, y, RGB(0,0,0));

			if(f < 0){
				f += f1;
			}else{
				f += f2;
				y += dy;
			}

			x += dx;
		}
	}else{
		while(y != EndPosition.y){
			SetPixel(hdc, x, y, RGB(0,0,0));

			if(f < 0){
				f += f1;
			}else{
				f += f2;
				x += dx;
			}

			y += dy;
		}
	}
}

int myRegion(const POINT &InVector, const POINT &Min, const POINT &Max){
	int ret = 0;

	/* 하위, 상위 비트에 분기에 따라 각각 비트값을 지정한다. */

	/*
			_________________________
			| 1001	| 1000	|  1010	|
			|-------|-------|-------|
			| 0001	| 0000	|  0010	|
			|-------|-------|-------|
			| 0101  | 0100  |  0110 |
			-------------------------
	 */

	if(InVector.x <= Min.x){
		ret = ret | 0b0001;
	}else if(InVector.x > Max.x){
		ret = ret | 0b0010;
	}

	if(InVector.y < Min.y){
		ret = ret | 0b0100;
	}else if(InVector.y > Max.y){
		ret = ret | 0b1000;
	}

	return ret;
}

bool CohenSutherlandLineClip(POINT &clipStart, POINT &clipEnd, const POINT &minScreen, const POINT &maxScreen){
	int start = myRegion(clipStart, minScreen, maxScreen);
	int end = myRegion(clipEnd, minScreen, maxScreen);

	float width = clipEnd.x - clipStart.x;
	float height = clipEnd.y - clipStart.y;

	while(true){
		if((start == 0) && (end == 0)){
			// 화면 안
			return true;
		}else if(start & end){
			// 화면 밖
			return false;
		}else{
			// TODO : 양쪽을 조사해서 클리핑 작업 진행

			POINT clipPosition;
			BOOL isStart = (start != 0);
			int current = isStart ? start : end;
			// 어떤 지점이 안쪽(0b0000 - 화면 영역) 바깥인지 판별

			// 화면 영역을 벗어나 좌측 하단일 때
			if(current < 0b0100){
				if(current & 1){
					clipPosition.x = minScreen.x;
				}else{
					clipPosition.x = maxScreen.x;
				}

				if(EqualsInTolerance(height, 0.0f)){
					clipPosition.y = clipStart.x;
				}else{
					clipPosition.y = clipStart.y + height * (clipPosition.x - clipStart.x) / width;
				}
			}else{
				if(current & 0b0100){
					clipPosition.y = minScreen.y;
				}else{
					clipPosition.y = maxScreen.y;
				}

				if(EqualsInTolerance(width, 0.0f)){
					clipPosition.x = clipStart.x;
				}else{
					clipPosition.x = clipStart.x + width * (clipPosition.y - clipStart.y) / height;
				}
			}

			if(isStart){
				clipStart = {clipPosition.x, clipPosition.y};
				start = myRegion(clipStart, minScreen, maxScreen);
			}else{
				clipEnd = {clipPosition.x, clipPosition.y};
				end = myRegion(clipEnd, minScreen, maxScreen);
			}
		}
	}

	return true;
}

bool EqualsInTolerance( float InFloat1, float InFloat2, float InTolerance) {
	return fabs(InFloat1 - InFloat2) <= InTolerance;
}


