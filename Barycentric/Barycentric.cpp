#define _WIN32_WINNT 0x0A00
#include <windows.h>
#pragma GCC diagnostic ignored "-Wunused-parameter"
#define CLASS_NAME TEXT("Barycentric Coordinate")

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpszCmdLine, int nCmdShow)
{
	WNDCLASSEX wcex = {
		sizeof(wcex),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,0,
		hInst,
		NULL, LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)(COLOR_WINDOW+1),
		NULL,
		CLASS_NAME,
		NULL
	};

	RegisterClassEx(&wcex);

	RECT crt;
	SetRect(&crt, 0,0, 800, 600);
	AdjustWindowRectEx(&crt, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_CLIENTEDGE);

	HWND hWnd = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				CLASS_NAME,
				CLASS_NAME,
				WS_OVERLAPPEDWINDOW,
				CW_USEDEFAULT, CW_USEDEFAULT, crt.right - crt.left, crt.bottom - crt.top,
				NULL,
				(HMENU)NULL,
				hInst,
				NULL
			);

	ShowWindow(hWnd, nCmdShow);

	MSG msg;
	while(GetMessage(&msg, nullptr, 0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}


POINT Triangle[] =
{
	{400, 200},
	{200, 400},
	{600, 400}
};

void DrawLine(HDC hdc, POINT from, POINT to)
{
	MoveToEx(hdc, from.x, from.y, NULL);
	LineTo(hdc, to.x, to.y);
}

void FillMesh(HDC hdc, POINT* Mesh)
{
	#define min(a,b) ((b) ^ ((a)^(b)) & -((a)<(b)))
	#define max(a,b) ((a) ^ ((a)^(b)) & -((a)<(b)))

	// Rect
	POINT Min, Max;
	
	// 삼각형 외접 사각 영역 구하기
	Min.x = min(min(Mesh[0].x, Mesh[1].x), Mesh[2].x);
	Min.y = min(min(Mesh[0].y, Mesh[1].y), Mesh[2].y);
	Max.x = max(max(Mesh[0].x, Mesh[1].x), Mesh[2].x);
	Max.y = max(max(Mesh[0].y, Mesh[1].y), Mesh[2].y);

	// 아핀 결합의 기초를 응용하는 문제다.
	// 아핀 결합은 점과 점을 결합하여 새로운 점을 만드는 공식을 말한다.

	// 이 점을 만드는데 사용되는 식은 다음과 같다.
	// P4 = P1 * S + P2 * T + (1 - S - T) * P3
	//    = SP1 + TP2 + P3 - SP3 - TP3
	//    = SP1 - SP3 + TP2 - TP3 + P3
	//    = S(P1 - P3) + T(P2 - P3) + P3
	
	// (P4 - P3) = S(P1 - P3) + T(P2 - P3)

	// 이 식이 유도되는 과정을 이해하려면 벡터 공간, 아핀 공간을 구분하고
	// 직선의 방정식을 이용하여 선을 만들 줄 알아야 한다.

	// 직선의 방정식은 우리가 아는 y = ax + b가 맞으며
	// 여기서 b는 y절편 즉, 수평한 점을 생성하는데 사용될 뿐이므로
	// 2^32 크기를 갖는 윈도우 스크린 좌표계에선 그냥 무시해도 된다.

	// 중요한건 기울기이며 ax의 a 즉, 스칼라를 제한하면 원하는 위치에 점을 생성할 수 있다.

	// 이 a의 값을 무한이 아닌 [0 1]범위로 제한하여 선형한 위치에 존재하는 점을 만들어 낼 것이다.
	// 아핀 공간에서 임의의 두 점을 잡고 선형 보간을 적용하여 새로운 점을 만들면
	// 이 점은 언제나 두 점의 사이에 위치하게 되며 임의의 값을 갖는 점이라면 그 값 역시 제한적이게 된다.

	// 여기서 확장하여 임의의 두 점을 임의의 세 점으로 늘리고
	// 선형 보간을 적용하여 점을 만들면 이 점은 언제나 세 개의 기준점을 넘어가지 않는다.

	// 알다시피 점이 세 개가 모였을 때 이들을 잇는 선을 그리면 삼각형이 되며
	// 삼각형 내부에 있는 점의 위치를 구하는데에도 위 공식을 응용할 수 있다.
	
	// 아핀 공간 즉, 점은 서로간 감산 연산을 수행하면 그 의미를 잃고
	// R2 즉, 2차원 평면의 벡터로 변환된다.

	// [x] 
	// [y]  이처럼 단순한 벡터로 변환되는데 이를 굳이 식으로 써보면 다음과 같다.

	// u = P2 - P1

	// 아핀 공간이 제외되며 2차원 벡터 공간의 벡터가 되는데
	// 그냥 점이라 생각하고 받아들이면 이해하기는 더 쉬울 것이다.

	// 이 u벡터로부터 좌표를 얻을 수 있으며 이 좌표가
	// 삼각형 영역 내부에 속하는지를 판단하고 픽셀화 시키면 삼각형 내부를 칠할 수 있다.

	// 참고로, 픽셀화란 쉽게 말해서 스크린 좌표계로 점을 이동시키고 색상을 입히는 일련의 과정을 말한다.

	// 일단, 앞에서 아핀 결합의 공식을 보였으므로 이걸 어떻게 응용해야 할 지 생각해보자.
	// 아핀 결합의 공식 끝에 유도된 식을 벡터로 도식화하면 다음과 같다.

	// (P4 - P3) = S(P1 - P3) + T(P2 - P3)
	//  ->  w = S * u + T * v

	// 이 벡터는 u와 v벡터가 선형 독립의 관계를 가질 때 평면의 어느곳으로든 향할 수 있다.
	// 벡터가 아닌 점이라는 개념으로 똑같이 서술해보면  P4라는 새로운 점은 평면의 어느곳에든 위치할 수 있다는 것이다.
	// 그런데 우리는 이미 P4의 위치를 알고 있다.

	// 사실 삼각형을 포함하는 외접 사각영역을 특정하여 이에 대한 루프를 돌리고
	// 이 점이 삼각형 내부에 포함되는지를 점검하는 함수를 만들고 있기 때문에
	// 점을 생성하는 것과는 반대로 이 점을 만들기 위한 아핀 결합의 스칼라 값을 구하는 공식이 필요하다.

	// 이를 구하기 위한 공식을 세워보자.
	// 위의 식에서 w는 이미 벡터이므로 스칼라 값을 유도하기 위해선 벡터간의 내적 연산이 필요하다.

	// 이를 위해 이미 구해둔 w벡터에 u와 v를 내적하고 도출된 실수 값으로부터 u와 v에 결합된 스칼라값을 구해야 한다.

	// w dot u = (S * u + T * v) dot u
	// w dot v = (S * u + T * v) dot v

	// 위의 두 식을 연립하여 풀기 위해 가감법(감산)을 적용하여
	// 식을 유도해보면 쉽게 이해할 수 있다.

	// 손으로 풀면 빠른데 글로 옮기는 것은 귀찮으므로 패스할까 했으나 아래 적어뒀다.

	// 이렇게 구한 식을 코드로 보이면 다음과 같다.
	
	// w dot u = (S * u + T * v) dot u
	// w dot v = (S * u + T * v) dot v

	// w dot u = S(u dot u) + T(v dot u)
	// w dot v = S(u dot v) + T(v dot v)

	// S와 T를 구하기 위해 내적 연산을 추가로 곱하여 전개한다.

	// (w dot u)(u dot v) = S(u dot u)(u dot v) + T(u dot v)(u dot v)
	// (w dot v)(u dot u) = S(u dot v)(u dot u) + T(v dot v)(u dot u)

	// (w dot u)(u dot v) = T(u dot v) (u dot v)
	// (w dot v)(u dot u) = T(v dot v) (u dot u)

	// 0 = T(u dot v)(u dot v) - (w dot u)(u dot v)
	// 0 = T(v dot v)(u dot u) - (w dot v)(u dot u)

	// -T(u dot v)(u dot v) = -(w dot u)(u dot v)
	// -T(v dot v)(u dot u) = -(w dot v)(u dot u)

	// T(u dot v)(u dot v) = (w dot u)(u dot v)				//1
	// T(v dot v)(u dot u) = (w dot v)(u dot u)				//2

	// 귀찮으므로 두 방정식 중 위 1번에 대해서만 전개함
	// (Tu^2 dot Tv^2) = (w dot u)(u dot v)
	// = T(u^2 dot v^2)	= (w dot u)(u dot v)
	// = T(u dot v)^2 = (w dot u)(u dot v)
	// = T = (w dot u)(u dot v) / (u dot v)^2

	// 여기다가 2번도 전개하고 이어 붙이면 된다.
	// 그러면 대충 아래와 같은 모양이 된다.
	// T = (w dot u)(u dot v) / (u dot v)(u dot v)
	// T = (w dot v)(u dot u) / (v dot v)(u dot u)

	// T = (w dot u)(u dot v) - (w dot v)(u dot u) / (u dot v)(u dot v) - (v dot v)(u dot u)

	// 최종 식을 구했으므로 이제 T를 구해주기만 하면 된다
	// 필요한 요소를 구하고 S에 대한 식도 전개하여 T와 같은 방식으로 값을 유도한다.

	// u와 v벡터를 구한다.
	POINT u,v;
	u = {Mesh[1].x - Mesh[0].x, Mesh[1].y - Mesh[0].y};
	v = {Mesh[2].x - Mesh[0].x, Mesh[2].y - Mesh[0].y};

	float udotu = u.x * u.x + u.y * u.y;
	float udotv = u.x * v.x + u.y * v.y;
	float vdotv = v.x * v.x + v.y * v.y;

	float denominator = udotv * udotv - vdotv * udotu;
	float InvDenominator = 1.f / denominator;

	for(int x = Min.x; x <= Max.x; x++)
	{
		for(int y = Min.y; y <= Max.y; y++)
		{
			// 루프를 돌면서 P4라는 새로운 점(NewPoint)까지의 이동 벡터를 구한다.
			POINT NewPoint = {x, y};
			POINT w = {NewPoint.x - Mesh[0].x, NewPoint.y - Mesh[0].y};

			float wdotu = w.x * u.x + w.y * u.y;
			float wdotv = w.x * v.x + w.y * v.y;

			float S = ((wdotv * udotv) - (wdotu * vdotv)) * InvDenominator;
			float T = ((wdotu * udotv) - (wdotv * udotu)) * InvDenominator;

			float one = 1.f - S - T;

			if(((S >= 0.f) && (S <= 1.f)) && ((T >= 0.f) && (T <= 1.f)) && ((one >= 0.f) && (one <= 1.f)))
			{
				SetPixel(hdc, x,y, RGB(0,0,255));
			}
		}
	}

}


LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch(iMessage)
	{
		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hWnd, &ps);
				
				// TODO : ...

				{
					int count = sizeof(Triangle)/sizeof(Triangle[0]);
					for(int i=0; i<count; i++)
					{
						DrawLine(hdc, Triangle[i], Triangle[(i+1)%count]);
					}

					FillMesh(hdc, Triangle);
				}
				
				EndPaint(hWnd, &ps);
			}
			return 0;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
	}
	
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}
