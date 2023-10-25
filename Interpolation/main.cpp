#include <windows.h>

#define CLASS_NAME "Image BillinearInterpolation"

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam);
void OnExitSizeMove(HWND hWnd, WPARAM wParam, LPARAM lParam);

void VirtualMemoryBlt(BITMAPINFOHEADER *pih, BYTE *pData);
void BilinearOperation(HWND hWnd);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow){
	WNDCLASS wc = {
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME
	};

	RegisterClass(&wc);

	HWND hWnd = CreateWindow(
			CLASS_NAME,
			CLASS_NAME,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			(HMENU)NULL,
			hInst,
			NULL);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, nullptr, 0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam){
	switch(iMessage){
		case WM_CREATE:
			OnCreate(hWnd, wParam, lParam);
			return 0;

		case WM_PAINT:
			OnPaint(hWnd, wParam, lParam);
			return 0;

		case WM_SIZE:
			OnSize(hWnd, wParam, lParam);
			return 0;

		case WM_EXITSIZEMOVE:
			OnExitSizeMove(hWnd, wParam, lParam);
			return 0;

		case WM_DESTROY:
			OnDestroy(hWnd, wParam, lParam);
			return 0;
	}

	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}


BITMAPINFOHEADER g_ih;
BYTE *g_pData = NULL;

HDC g_hMemDC;
HBITMAP g_hMemBmp;

void OnCreate(HWND hWnd, WPARAM wParam, LPARAM lParam){
	HDC hdc = GetDC(hWnd);

	HANDLE hFile = CreateFile(
				"Image.bmp",
				GENERIC_READ,
				0,NULL,
				OPEN_EXISTING,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);

	if(hFile != INVALID_HANDLE_VALUE){
		DWORD dwRead;

		SetFilePointer(hFile, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);
		ReadFile(hFile, &g_ih, sizeof(g_ih), &dwRead, NULL);

		// 비트맵의 실제 데이터 즉, 레스터 데이터는
		// DWORD 단위(4바이트)로 정렬된다.
		// 보통 아래 식으로 전체 크기를 한 번에 구하는 편이다
		/*
			int ImageSize = 
			((ih.biWidth * ih.biBitCount + 31) & ~31) >> 3 * ih.biHeight

			이 식은 아래와 같이 단순 연산으로도 표현할 수 있다
			근래에 와서야 통용되는 식인데 예전 4,8,16비트 그래픽 시절엔
			위와 같이 정확한 계산이 필요했다
		 */
		int iAlign = (g_ih.biWidth * 3 + 3) / 4 * 4;
		g_pData = new BYTE[iAlign * g_ih.biHeight];

		ReadFile(hFile, g_pData, iAlign * g_ih.biHeight, &dwRead, NULL);
		CloseHandle(hFile);

		g_hMemDC = CreateCompatibleDC(hdc);
		g_hMemBmp = CreateCompatibleBitmap(hdc, g_ih.biWidth, g_ih.biHeight);

		// 함수로 분리하여 Size 메시지에서도 이미지를 메모리에 미리 출력할 수 있게끔 만든다
		VirtualMemoryBlt(&g_ih, g_pData);

	}else{
		MessageBox(HWND_DESKTOP, "Error", "Warning", MB_ICONWARNING);
	}

	ReleaseDC(hWnd, hdc);
}


void VirtualMemoryBlt(BITMAPINFOHEADER *pih, BYTE *pData){
	HGDIOBJ hOld = SelectObject(g_hMemDC, g_hMemBmp);

	SetDIBitsToDevice(
				g_hMemDC,					// 출력 대상
				0,0,						// left, top (시작 지점)
				pih->biWidth,				// right (가로 크기)
				pih->biHeight,				// bottom (세로 크기)
				0,0,						// 출력할 이미지의 시작 좌표
				0,							// 스캔 라인(가로 : 시작 지점)
				pih->biHeight,				// 스캔 라인(세로 : 읽어들일 끝 지점)
				pData,						// 비트맵 레스터 데이터
				(BITMAPINFO*)pih,			// 비트맵 정보(색상면, 픽셀당 비트 수 등등 + 색상값) 
				DIB_RGB_COLORS				// 화면에 색상을 표현하는 방법
			);

	SelectObject(g_hMemDC, hOld);
}


void OnDestroy(HWND hWnd, WPARAM wParam, LPARAM lParam){
	if(g_pData){delete [] g_pData;}
	if(g_hMemBmp){DeleteObject(g_hMemBmp);}
	if(g_hMemDC){DeleteDC(g_hMemDC);}

	PostQuitMessage(0);
}

void OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam){
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);

	if(g_hMemDC){
		RECT rt;
		BITMAP bmp;
		GetClientRect(hWnd, &rt);

		GetObject(g_hMemBmp, sizeof(bmp), &bmp);
		HGDIOBJ hOld = SelectObject(g_hMemDC, g_hMemBmp);

		StretchBlt(hdc, 0,0, rt.right, rt.bottom, g_hMemDC, 0,0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

		SelectObject(g_hMemDC, hOld);
	}

	EndPaint(hWnd, &ps);
}

void OnSize(HWND hWnd, WPARAM wParam, LPARAM lParam){
	/*
	static WPARAM Prev = 0;

	if((wParam == SIZE_MAXIMIZED && Prev == SIZE_RESTORED) ||
			wParam == SIZE_RESTORED && Prev == SIZE_MAXIMIZED){
		BillinearOperation(hWnd);
	}

	Prev = wParam;
	*/

	if(wParam != SIZE_MINIMIZED)
	{
		if(wParam == SIZE_MAXIMIZED || wParam == SIZE_RESTORED)
		{
				BilinearOperation(hWnd);
		}
	}
}

void OnExitSizeMove(HWND hWnd, WPARAM wParam, LPARAM lParam){
	// 이 메시지는 wParam과 lParam을 사용해선 안된다.
	// 또, 이 메시지를 따로 처리하는 경우 반드시 return 0로
	// 모든 처리를 프로그래머가 끝냈음을 알려야 한다.
	BilinearOperation(hWnd);
}

void BilinearOperation(HWND hWnd){
	// 현재 화면 크기 불러옴
	RECT rt;
	GetClientRect(hWnd, &rt);

	// 어떠한 공식이 있는건 아니다
	// 단순히 1차원 배열로 비트맵을 읽어왔고
	// 해당 버퍼의 정렬이 아래 식과 같기 때문에 통일했을 뿐이다
	int ScreenWidth = rt.right - rt.left;
	int ScreenHeight = rt.bottom - rt.top;
	int ScreenAlign = (ScreenWidth * 3 + 3 ) / 4 * 4;

	int BitmapWidth = g_ih.biWidth;
	int BitmapHeight = g_ih.biHeight;
	int BitmapAlign = (BitmapWidth * 3 + 3) / 4 * 4;

	BYTE *pNewImage = new BYTE[ScreenAlign * ScreenHeight];

	double RateWidth = (double)ScreenWidth / (double)BitmapWidth;
	double RateHeight = (double)ScreenHeight / (double)BitmapHeight;

	/*
				dx		1-dx
			p1 ---------------- p2
			|					|
			|			T		| dy
			|					|
			|					|
         	|					| 1-dy
			|					|
			p3 ---------------- p4

			여기서 나뉘는데 식을 보기 좋게 하려면 p2가 p1 하단에 위치하면 된다
	 */

	for(int i=0; i<ScreenHeight; i++){
		for(int j=0; j<ScreenWidth; j++){
			// 늘어난 화면의 크기이면서 인접한 점 p1의 좌표가 된다
			double P1_x = j / RateWidth;
			double P1_y = i / RateHeight;

			// 이게 실제 p1의 화면 좌표
			int Pixel_x = (int)P1_x;
			int Pixel_y = (int)P1_y;
			
			// 거리
			double dx = P1_x - Pixel_x;
			double dy = P1_y - Pixel_y;

			// 인접한 네 점을 계산한다
			// 단, 일차원 배열이므로 정렬 값을 곱하여 다음 줄로 이동한다
			int p1,p2,p3,p4;

			// 정렬 값을 그대로 이용해서 포인트를 지정한다.
			p1 = Pixel_y * BitmapAlign + Pixel_x * 3;
			p2 = Pixel_y * BitmapAlign + (Pixel_x+1) * 3;
			p3 = (Pixel_y+1) * BitmapAlign + Pixel_x * 3;
			p4 = (Pixel_y+1) * BitmapAlign + (Pixel_x+1) * 3;

			for(int k=0; k<3; k++){
				// 비어있거나 겹쳐진 비트를 대체할 값을 안접한 점으로부터 계산해낸다.
				double ColorBit = dx * dy * g_pData[p4+k] +
									dx * (1-dy) * g_pData[p2+k] +
									(1-dx) * dy * g_pData[p3+k] +
									(1-dx) * (1-dy) * g_pData[p1+k];

				pNewImage[i * ScreenAlign + j * 3 + k] = (BYTE)ColorBit;
			}
		}
	}


	DeleteObject(g_hMemBmp);

	HDC hdc = GetDC(hWnd);
	g_hMemBmp = CreateCompatibleBitmap(hdc, ScreenWidth, ScreenHeight);
	ReleaseDC(hWnd, hdc);

	BITMAPINFOHEADER ih = g_ih;
	ih.biWidth = ScreenWidth;
	ih.biHeight = ScreenHeight;

	VirtualMemoryBlt(&ih, pNewImage);

	delete [] pNewImage;

	InvalidateRect(hWnd, &rt, FALSE);
}
