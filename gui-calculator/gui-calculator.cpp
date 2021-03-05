#include "includes.h"
#include "resource.h"
#include "Calculator/calculator.h"

#define IS_POPUP 1

const ImVec2 WindowSize{ 300, 400 }; // 400 615

#if IS_POPUP == 1
auto wnd_flags = WS_POPUP;
DWORD dxwf_flags = user_dxwf_flags::ENABLE_WINDOW_ALPHA | user_dxwf_flags::ENABLE_WINDOW_BLUR;
#else
auto wnd_flags = WS_OVERLAPPEDWINDOW;
DWORD dxwf_flags = user_dxwf_flags::NONE;
#endif // IS_POPUP

ImVec2 CurrentWindowSize = WindowSize;
static bool is_title_bar_hovered = false;
static bool is_resizing = false;

POINT g_ptMousePos;
bool g_bIsMouseCaptured = false;

RECT rc;
POINT ptCursor;
POINT ptDelta;

std::unique_ptr<CCalculator>g_calculator = std::make_unique<CCalculator>();

void InitImGui();

void DestroyImGui();

void WndProcHandlerCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WndProcMouseMoveCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WndProcLButtonDownCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void WndProcLButtonUpCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void RenderCallback();

void BeginSceneCallback();

void ResetCallback();

LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
	/*AllocConsole();
	freopen("CONOUT$", "w", stdout);*/

	if (DXWFInitialization(hInst) == FALSE)
	{
		MessageBox(NULL, "Error initialize DXWF", "Calculator", MB_OK | MB_ICONERROR);
		return 1;
	}

	DXWFWndProcCallbacks(DXWF_WNDPROC_WNDPROCHANDLER_, WndProcHandlerCallback);
#if IS_POPUP == 1
	DXWFWndProcCallbacks(DXWF_WNDPROC_WM_MOUSEMOVE_, WndProcMouseMoveCallback);
	DXWFWndProcCallbacks(DXWF_WNDPROC_WM_LBUTTONDOWN_, WndProcLButtonDownCallback);
	DXWFWndProcCallbacks(DXWF_WNDPROC_WM_LBUTTONUP_, WndProcLButtonUpCallback);
#endif

	DXWFRendererCallbacks(DXWF_RENDERER_RESET_, ResetCallback);
	DXWFRendererCallbacks(DXWF_RENDERER_LOOP_, RenderCallback);
	DXWFRendererCallbacks(DXWF_RENDERER_BEGIN_SCENE_LOOP_, BeginSceneCallback);
	
	RECT screen_rect;
	GetWindowRect(GetDesktopWindow(), &screen_rect);
	int x = screen_rect.right / 2 - WindowSize.x / 2;
	int y = screen_rect.bottom / 2 - WindowSize.y / 2;

	if (DXWFCreateWindow("Calculator",
		x, y,
		WindowSize.x, WindowSize.y,
		wnd_flags,
		NULL,
		dxwf_flags,
		IDI_ICON1))
	{
		InitImGui();
		DXWFRenderLoop();
	}
	else
	{
		return 1;
	}

	g_calculator.release();

	DestroyImGui();

	DXWFTerminate();
}

void InitImGui()
{
	ImGui::CreateContext();

	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Tahoma.ttf", 28.9f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	ImGui_ImplDX9_Init(DXWFGetD3DDevice());
	ImGui_ImplWin32_Init(DXWFGetHWND());

	ImGui::StyleColorsDark();

	auto &style = ImGui::GetStyle();

	style.AntiAliasedFill = true;
	style.AntiAliasedLines = true;
	style.CurveTessellationTol = true;

	style.Colors[ImGuiCol_Border] = ImVec4();

#if IS_POPUP == 0
	ImGui::StyleColorsLight();
	ImGui::GetStyle().WindowRounding = 0.f;
#else
	ImGui::GetStyle().WindowRounding = 8.f;
#endif // IS_POPUP
}

void DestroyImGui()
{	
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();	
	ImGui::DestroyContext();
}

void WndProcHandlerCallback(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam);
}

void RenderCallback()
{
	static bool application_is_open = true;

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2());
#if IS_POPUP == 1
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Once);
#else
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));
#endif // IS_POPUP

#if IS_POPUP == 1
	if (ImGui::Begin(u8"Калькулятор", &application_is_open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar))
#else
	constexpr ImGuiWindowFlags WndFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin(u8"Калькулятор", nullptr, WndFlags);
#endif // IS_POPUP
	{
#if IS_POPUP == 1
		CurrentWindowSize = ImGui::GetWindowSize();
		is_title_bar_hovered = ImGui::GetCurrentWindow()->DC.LastItemStatusFlags == ImGuiItemStatusFlags_HoveredRect;
		is_resizing = ImGui::GetCurrentWindow()->DC.WindowIsResizing || (int)ImGui::GetCurrentWindow()->ResizeBorderHeld != -1;
#endif // IS_POPUP

		auto io = ImGui::GetIO();

		static char expression[4048] = { 0 };

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.f, 2.f));
		float input_width = io.DisplaySize.x / 2.f;
		input_width += io.DisplaySize.x / 4.f - 12.5f;
		ImGui::PushItemWidth(input_width);
		ImGui::InputText("##input_block", expression, 256, ImGuiInputTextFlags_CharsDecimal);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		if (ImGui::Button("C", ImVec2(io.DisplaySize.x / 4.f - 5.f, 34.f))) {
			size_t len = strlen(expression);
			if (len > 0)
				expression[len - 1] = '\0';
		}

		const char* buttons[] = {
			"(", ")", "AC", "-", "9", "8", "7", "/", "6", "5", "4", "*", "3", "2", "1", "+", ".", "0", "+/-", "="
		};

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.f);

		float button_height = (ImGui::GetContentRegionAvail().y / 5.f) - 1.5f; //b1g meme

		for (auto button : buttons)
		{
			if (button == ")" || button == "AC" || button == "-"
				|| button == "8" || button == "7" || button == "/"
				|| button == "5" || button == "4" || button == "*"
				|| button == "2" || button == "1" || button == "+"
				|| button == "0" || button == "+/-" || button == "=") ImGui::SameLine();

			auto calculate = []() -> void
			{
				g_calculator->Setup(std::string(expression));
				g_calculator->Compute();
				std::string ex = std::to_string(g_calculator->GetResult());

				for (int i = ex.size() - 1; i >= 0; i--)
				{
					auto is_point = ex[i] == '.';
					auto is_zero = ex[i] == '0';

					if (is_point || is_zero)
					{
						ex.erase(i);
						if (is_point)
							break;
					}
					else
						break;
				}

				strcpy(expression, ex.c_str());
			};

			static bool pressed = false;
			if (GetAsyncKeyState(VK_RETURN))
			{
				if (!pressed)
				{
					calculate();
					pressed = true;
				}
			}
			else { pressed = false; }

			if (ImGui::Button(button, ImVec2((io.DisplaySize.x / 4.f) - 5.6f, button_height)))
			{
				static bool negate = false;
				if (button == "AC")
				{
					RtlZeroMemory(expression, sizeof(expression));
				}
				else if (button == "+/-")
				{
					negate = !negate;
					if (negate)
					{
						std::string negate_val = std::string("-") + std::string(expression);
						strcpy(expression, negate_val.c_str());
					}
					else
					{
						std::string negate_revert;
						for (int i = 1; i < (sizeof(expression) / sizeof(expression[0])) - 1; i++)
						{
							negate_revert += expression[i];
						}
						strcpy(expression, negate_revert.c_str());
					}
				}
				else if (button == "=")
				{
					calculate();
				}
				else
				{
					strcat(expression, button);
				}
			}
		}
		ImGui::PopStyleVar();

		ImGui::End();
	}

	ImGui::EndFrame();

	if (application_is_open == false)
		PostQuitMessage(0);
}

void BeginSceneCallback()
{
	ImGui::Render();
	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
}

void ResetCallback()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
}

void WndProcMouseMoveCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_bIsMouseCaptured && (is_title_bar_hovered || is_resizing))
	{
		GetWindowRect(hWnd, &rc);
		GetCursorPos(&ptCursor);
		ptDelta.x = g_ptMousePos.x - ptCursor.x;
		ptDelta.y = g_ptMousePos.y - ptCursor.y;

		if (is_resizing)
		{
			MoveWindow(hWnd, rc.left , rc.top, CurrentWindowSize.x, CurrentWindowSize.y, TRUE);
		}
		else
		{
			MoveWindow(hWnd, rc.left - ptDelta.x, rc.top - ptDelta.y, CurrentWindowSize.x, CurrentWindowSize.y, TRUE);
		}	
		g_ptMousePos.x = ptCursor.x;
		g_ptMousePos.y = ptCursor.y;
	}
}

void WndProcLButtonDownCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!g_bIsMouseCaptured)
	{
		SetCapture(hWnd);
		g_bIsMouseCaptured = true;
		GetCursorPos(&g_ptMousePos);
	}
}

void WndProcLButtonUpCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (g_bIsMouseCaptured)
	{
		ReleaseCapture();
		g_bIsMouseCaptured = false;
	}
}