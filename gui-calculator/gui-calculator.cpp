#include "includes.h"
#include "resource.h"
#include "Calculator/calculator.h"

std::unique_ptr<CCalculator>g_calculator = std::make_unique<CCalculator>();

const ImVec2 WindowSize{ 300, 400 };

void InitImGui();

void DestroyImGui();

void WndProcHandlerCallback(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

void RenderCallback();

void BeginSceneCallback();

void ResetCallback();

LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int)
{
	/*AllocConsole();
	freopen("CONOUT$", "w", stdout);*/

	DXWFInitialization(hInst);

	DXWFWndProcCallbacks(DXWF_WNDPROC_WNDPROCHANDLER_, WndProcHandlerCallback);

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
		WS_OVERLAPPEDWINDOW,
		NULL,
		user_dx_flags::NONE,
		IDI_ICON1))
	{
		InitImGui();
		DXWFRenderLoop();
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

	ImGui::StyleColorsLight();
	ImGui::GetStyle().WindowRounding = 0.f;
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
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowPos(ImVec2());
	ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));

	constexpr ImGuiWindowFlags WndFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
		| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;
	ImGui::Begin("##calculator", nullptr, WndFlags);

	auto io = ImGui::GetIO();

	static char expression[256] = { 0 };

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.f, 2.f));
	float input_width = io.DisplaySize.x / 2.f;
	input_width += io.DisplaySize.x / 4.f - 13.f;
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

	float button_height = io.DisplaySize.y / 6.f; //b1g meme
	button_height += (button_height - ((WindowSize.y - 19.f) / 6.f)) / 5.f;//pizdec =))))
	
	for (auto button : buttons)
	{
		if (button == ")" || button == "AC" || button == "-"
			|| button == "8" || button == "7" || button == "/"
			|| button == "5" || button == "4" || button == "*"
			|| button == "2" || button == "1" || button == "+"
			|| button == "0" || button == "+/-" || button == "=") ImGui::SameLine();

		auto calculate = []() -> void
		{
			g_calculator->setup(std::string(expression));
			g_calculator->compute();
			std::string ex = std::to_string(g_calculator->getResult());

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

	ImGui::EndFrame();
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