#include "stdafx.h"
#include "ImGuiManager.h"
#include "External/include/imgui/imgui.h"

namespace Kaka
{
	ImGuiManager::ImGuiManager()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui::StyleColorsDark();
	}

	ImGuiManager::~ImGuiManager()
	{
		ImGui::DestroyContext();
	}
}
