#include "basewin.h"
#include "Vector.h"
#include <vector>
#include <random>

using namespace std;

class MainWindow : public BaseWindow<MainWindow>
{
	RECT crt;
	HBITMAP hBit;
	COLORREF color;
	Vector<float> Center;
	Vector<float> start, end, Point;
	vector<Vector<float>> Circle;

	HANDLE hTimer;
	BOOL InitializeTimer();

	public:
		MainWindow() : hTimer(NULL), hBit(NULL) {;}

	public:
		void OnCreate(WPARAM wParam, LPARAM lParam);
		void OnDestroy(WPARAM wParam, LPARAM lParam);
		void OnTimer(WPARAM wParam, LPARAM lParam);
		void OnPaint(WPARAM wParam, LPARAM lParam);

		void GetSinCos(float *sinf, float *cosf, float angle);
		void GetSinCosRad(float *sinf, float *cosf, float radian);
		void DrawLine(HDC hdc, int x, int y, int dx, int dy);
		void DrawLine(HDC hdc, POINT start, POINT end);
		void DrawLine(HDC hdc, Vector<float> start, Vector<float> end);
		void DrawBitmap(HDC hdc, int x, int y);
		void WaitTimer();

		LPCWSTR ClassName() const {return L"Projection Example Window Class";}
		LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int nCmdShow)
{
	MainWindow win;
	if(!win.Create(L"Projection Example"))
	{
		return 0;
	}

	ShowWindow(win.Window(), nCmdShow);

	MSG msg = {0};
	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg, nullptr, 0,0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

		win.WaitTimer();
	}

	return 0;
}

LRESULT MainWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
		case WM_CREATE:
			OnCreate(wParam, lParam);
			return 0;

		case WM_DESTROY:
			OnDestroy(wParam, lParam);
			return 0;

		case WM_TIMER:
			OnTimer(wParam, lParam);
			return 0;

		case WM_PAINT:
			OnPaint(wParam, lParam);
			return 0;

		default:
			return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
}

void MainWindow::GetSinCos(float *sinf, float *cosf, float angle)
{
	if(angle == 0.f)
	{
		*sinf = 0.f;
		*cosf = 1.f;
	}else if(angle == 90.f)
	{	
		*sinf = 1.f;
		*cosf = 0.f;
	}else if(angle == 180.f)
	{
		*sinf = 0.f;
		*cosf = -1.f;
	}else if(angle == 270.f)
	{
		*sinf = -1.f;
		*cosf = 0.f;
	}else{
		float radian = angle * atan(1.f) * 4.f / 180.f;
		GetSinCosRad(sinf, cosf, radian);
	}
}

void MainWindow::GetSinCosRad(float *sinf, float *cosf, float radian)
{
	*sinf = sin(radian);
	*cosf = cos(radian);
}

void MainWindow::DrawLine(HDC hdc, int x, int y, int dx, int dy)
{
	MoveToEx(hdc, x, y, NULL);
	LineTo(hdc, dx, dy);
}

void MainWindow::DrawLine(HDC hdc, POINT start, POINT end)
{
	DrawLine(hdc, start.x, start.y, end.x, end.y);
}

void MainWindow::DrawLine(HDC hdc, Vector<float> start, Vector<float> end)
{
	DrawLine(hdc, (int)start.x, (int)start.y, (int)end.x, (int)end.y);
}

BOOL MainWindow::InitializeTimer()
{
	hTimer = CreateWaitableTimer(NULL, FALSE, NULL);

	if(hTimer == NULL)
	{
		return FALSE;
	}

	LARGE_INTEGER li = {0};
	if(!SetWaitableTimer(hTimer, &li, (1000/60), NULL, NULL, FALSE))
	{
		CloseHandle(hTimer);
		hTimer = NULL;
		return FALSE;
	}

	return TRUE;
}

void MainWindow::WaitTimer()
{
	if(MsgWaitForMultipleObjects(1, &hTimer, FALSE, INFINITE, QS_ALLINPUT) == WAIT_OBJECT_0)
	{
		InvalidateRect(m_hWnd, NULL, FALSE);
	}
}

void MainWindow::DrawBitmap(HDC hdc, int x, int y)
{
	BITMAP bmp;
	GetObject(hBit, sizeof(BITMAP), &bmp);

	HDC hMemDC = CreateCompatibleDC(hdc);
	HGDIOBJ hOld = SelectObject(hMemDC, hBit);

	BitBlt(hdc, x,y, bmp.bmWidth, bmp.bmHeight, hMemDC, 0,0, SRCCOPY);

	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);
}

void MainWindow::OnCreate(WPARAM wParam, LPARAM lParam)
{
	if(!InitializeTimer())
	{
		return;
	}
	
	SetTimer(m_hWnd, 1, 10, NULL);
	GetClientRect(m_hWnd, &crt);
	int iWidth = crt.right - crt.left;
	int iHeight = crt.bottom - crt.top;
	
	Center = Vector<float>(iWidth >> 1, iHeight >> 1);

	color = RGB(255,0,0);

	if(Circle.empty())
	{
		float radius = 10.f;
		for(float x = -radius; x <= radius; x++)
		{
			for(float y = -radius; y <= radius; y++)
			{
				Vector v = Vector(x,y);
				float size = v.Squared();
				if(size <= radius * radius)
				{
					Circle.push_back(Vector(x,y));
				}
			}
		}
	}
}

void MainWindow::OnDestroy(WPARAM wParam, LPARAM lParam)
{
	KillTimer(m_hWnd, 1);
	if(hBit){DeleteObject(hBit);}
	if(hTimer){CloseHandle(hBit);}
	PostQuitMessage(0);
}

void MainWindow::OnTimer(WPARAM wParam, LPARAM lParam)
{
	static float deltaTime = 0.01f;
	static float duration = 6.f;
	static float elapsedTime = 0.f;
	static float currentDegree = 0.f;
	static float rotateSpeed = 180.f;
	static float distance = 250.f;
	static random_device rd;
	static mt19937 mt(rd());
	static uniform_real_distribution<float> randomY(-200.f, 200.f);

	static float PI = atan(1.f) * 4.f;
	static float TwoPI = PI * 2.f;
	static float InvPI = 1.f / PI;

	switch(wParam)
	{
		case 1:
			{
				HDC hdc = GetDC(m_hWnd);
				HDC hMemDC = CreateCompatibleDC(hdc);
				GetClientRect(m_hWnd, &crt);
				if(hBit == NULL)
				{
					hBit = CreateCompatibleBitmap(hdc, crt.right, crt.bottom);
				}
				HGDIOBJ hOld = SelectObject(hMemDC, hBit);
				FillRect(hMemDC, &crt, GetSysColorBrush(COLOR_WINDOW));

				//TODO : Drawing
				{
					// Mid Line
					// DrawLine(hMemDC, 0, crt.bottom - crt.top >> 1, crt.right - crt.left, crt.bottom - crt.top >> 1);
				}

				{
					// Point circle
					elapsedTime = min(duration, max(elapsedTime + deltaTime, 0.f));
					if(elapsedTime == duration)
					{
						start = Vector(-400.f, randomY(mt));
						end = Vector(400.f, randomY(mt));
						elapsedTime = 0.f;
					}

					currentDegree = fmod(currentDegree + deltaTime * rotateSpeed, 360.f);
					float sinf, cosf;
					GetSinCos(&sinf, &cosf, currentDegree);
					Point = Vector(cosf, sinf) * distance;

					color = RGB(255, 0,0);
					for(auto const &v : Circle)
					{
						SetPixel(hMemDC, Center.x + v.x + Point.x, Center.y + v.y + Point.y, color);
					}

					DrawLine(hMemDC, Center + start, Center + end);

					HPEN hPen = CreatePen(PS_SOLID, 0, RGB(255, 0, 0));
					HPEN hOldPen = (HPEN)SelectObject(hMemDC, hPen);
					DrawLine(hMemDC, start + Center, Point + Center);
					DeleteObject(SelectObject(hMemDC, hOldPen));

				}

				{
					// Projection
					Vector unitv = (end - start).GetNormalize();		// 점과 점을 빼 벡터를 만들고 벡터의 길이를 1로 만든다.
					Vector u = Point - start;							// 목표와 시작점을 빼 그 사이를 잇는 벡터를 만든다.
					Vector projV = unitv * (u.Dot(unitv));				// 만들어진 벡터를 평면의 단위 벡터와 내적하여 길이를 구한다.
																		// 이후 방향성을 추가하여 벡터로 만든다.
					Vector projectedPoint = start + projV;				// 투영 벡터 + 시작점 = 투영된 점의 위치
					// float distance = (projectedPoint - Point).Norm();

					color = RGB(255, 0, 255);
					for(auto const &v : Circle)
					{
						SetPixel(hMemDC, Center.x + v.x + projectedPoint.x, Center.y + v.y + projectedPoint.y, color);
					}

					HPEN hPen = CreatePen(PS_SOLID, 0, RGB(128, 128, 128));
					HPEN hOldPen = (HPEN)SelectObject(hMemDC, hPen);
					DrawLine(hMemDC, Center + projectedPoint, Center + Point);
					DeleteObject(SelectObject(hMemDC, hOldPen));
				}

				SelectObject(hMemDC, hOld);
				DeleteDC(hMemDC);
				ReleaseDC(m_hWnd, hdc);
			}
			InvalidateRect(m_hWnd, NULL, FALSE);
			break;
	}
}

void MainWindow::OnPaint(WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(m_hWnd, &ps);
	if(hBit != NULL)
	{
		DrawBitmap(hdc, 0,0);
	}
	EndPaint(m_hWnd, &ps);
}
