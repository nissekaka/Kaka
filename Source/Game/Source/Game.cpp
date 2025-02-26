#include "Game.h"
#include <Core/Utility/KakaMath.h>

#include "ECS/Components/Components.h"
#include "Graphics/Drawable/ModelFactory.h"
#include "imgui/imgui.h"

constexpr int WINDOW_WIDTH = 1920;
constexpr int WINDOW_HEIGHT = 1080;


namespace Kaka
{
	Game::Game() : wnd(WINDOW_WIDTH, WINDOW_HEIGHT, L"Kaka") {}

	int Game::Go()
	{
		entities.reserve(MAX_ENTITIES);

		//entities.push_back(ecs.CreateEntity());
		//entities.back()->AddComponent(TransformComponent{});
		//entities.back()->AddComponent(ModelComponent{ &wnd.Gfx().LoadModelData("Assets/Models/sponza_pbr/Sponza.obj") });
		//entities.back()->GetComponent<ModelComponent>()->vertexShader = ShaderFactory::GetVertexShader(wnd.Gfx(), eVertexShaderType::ModelTAAInstanced);
		//entities.back()->GetComponent<ModelComponent>()->pixelShader = ShaderFactory::GetPixelShader(wnd.Gfx(), ePixelShaderType::Model);
		//entities.back()->GetComponent<TransformComponent>()->SetScale(0.1f);

		//entities.push_back(ecs.CreateEntity());
		//entities.back().AddComponent(TransformComponent{});
		//entities.back().AddComponent(ModelComponent{ "Assets/Models/crawler/CH_NPC_Crawler_01_22G3S_SK.fbx" });
		//entities.back().GetComponent<ModelComponent>()->vertexShader = ShaderFactory::GetVertexShader(wnd.Gfx(), eVertexShaderType::ModelTAAInstanced);
		//entities.back().GetComponent<ModelComponent>()->pixelShader = ShaderFactory::GetPixelShader(wnd.Gfx(), ePixelShaderType::Model);
		//entities.back().GetComponent<TransformComponent>()->x = 20.0f;
		//entities.back().GetComponent<TransformComponent>()->z = 20.0f;
		//entities.back().GetComponent<TransformComponent>()->scale = 0.1f;

		for (int i = 0; i < 100; i++)
		{
			for (int j = 0; j < 100; j++)
			{
				float randomX = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
				float randomZ = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

				entities.push_back(ecs.CreateEntity());
				entities.back()->AddComponent(Ecs::TransformComponent{});
				Ecs::TransformComponent* transform = entities.back()->GetComponent<Ecs::TransformComponent>();
				transform->SetPositionX(i * 10.0f + randomX);
				transform->SetPositionZ(j * 10.0f + randomZ);
				transform->SetScale(0.1f);

				entities.back()->AddComponent(Ecs::VelocityComponent{});

				entities.back()->AddComponent(Ecs::ModelComponent{ &wnd.Gfx().LoadModelData("Assets/Models/crawler/CH_NPC_Crawler_01_22G3S_SK.fbx") });
				entities.back()->GetComponent<Ecs::ModelComponent>()->vertexShader = ShaderFactory::GetVertexShader(wnd.Gfx(), eVertexShaderType::ModelTAAInstanced);
				entities.back()->GetComponent<Ecs::ModelComponent>()->pixelShader = ShaderFactory::GetPixelShader(wnd.Gfx(), ePixelShaderType::Model);
			}
		}

		ecs.RegisterModelComponents(wnd.Gfx());
		wnd.Gfx().BuildRenderQueue();

		while (true)
		{
			// Process all messages pending
			if (const auto code = Window::ProcessMessages())
			{
				// If return optional has value, we're quitting
				return *code;
			}

			// If no value
			Update(timer.UpdateDeltaTime());
		}
	}

	void Game::Update(const float aDeltaTime)
	{
		HandleInput(aDeltaTime);

		//ecs.GetEntity(0)->GetComponent<TransformComponent>()->yaw += 0.1f * aDeltaTime;

		ecs.UpdateComponents(aDeltaTime);

		wnd.Gfx().UpdateLights(aDeltaTime);
		FrameContext renderContext = { aDeltaTime, timer.GetTotalTime(), timer.GetFPS() };

		wnd.Gfx().BeginFrame();
		wnd.Gfx().Render(renderContext);

		// Display all entities with their components with ImGui
		if (wnd.Gfx().showImGui)
		{
			wnd.Gfx().ShowEntities(entities, selectedEntity);
		}

		wnd.Gfx().EndFrame();
	}

	void Game::HandleInput(const float aDeltaTime)
	{
		UNREFERENCED_PARAMETER(aDeltaTime);

		// TODO All of these set through wnd.Gfx() -> etc need to be handled better

		while (const auto e = wnd.keyboard.ReadKey())
		{
			if (!e->IsPressed())
			{
				continue;
			}

			switch (e->GetKeyCode())
			{
				case VK_ESCAPE:
					if (wnd.CursorEnabled())
					{
						wnd.DisableCursor();
						wnd.mouse.EnableRaw();
					}
					else
					{
						wnd.EnableCursor();
						wnd.mouse.DisableRaw();
					}
					break;
				case VK_F1:
					wnd.Gfx().showImGui = !wnd.Gfx().showImGui;
					break;
				case VK_F2:
					wnd.Gfx().showStatsWindow = !wnd.Gfx().showStatsWindow;
					break;
				case VK_F3:
					wnd.Gfx().drawLightDebug = !wnd.Gfx().drawLightDebug;
					break;
				case VK_F4:
					wnd.Gfx().drawDebug = !wnd.Gfx().drawDebug;
					break;
				case 'T':
					wnd.Gfx().temporalAntiAliasing.taaData.useTAA = !wnd.Gfx().temporalAntiAliasing.taaData.useTAA;
					break;
				case 'C':
				{
					// Create entity
					entities.push_back(ecs.CreateEntity());
					entities.back()->AddComponent(Ecs::TransformComponent{});
					Ecs::TransformComponent* transformComponent = entities.back()->GetComponent<Ecs::TransformComponent>();
					transformComponent->SetPosition(entities.size() * 20.0f, transformComponent->GetPosition().y, transformComponent->GetPosition().z);
					transformComponent->SetScale(0.1f);

					entities.back()->AddComponent(Ecs::ModelComponent{ &wnd.Gfx().LoadModelData("Assets/Models/crawler/CH_NPC_Crawler_01_22G3S_SK.fbx") });
					entities.back()->GetComponent<Ecs::ModelComponent>()->vertexShader = ShaderFactory::GetVertexShader(wnd.Gfx(), eVertexShaderType::ModelTAAInstanced);
					entities.back()->GetComponent<Ecs::ModelComponent>()->pixelShader = ShaderFactory::GetPixelShader(wnd.Gfx(), ePixelShaderType::Model);

					entities.back()->AddComponent(Ecs::VelocityComponent{});

					wnd.Gfx().ClearRenderData();
					ecs.RegisterModelComponents(wnd.Gfx());
					wnd.Gfx().BuildRenderQueue();
					selectedEntity = entities.back()->GetID();
				}
				break;
				case 'B':
				{
					// Create entity
					entities.push_back(ecs.CreateEntity());
					entities.back()->AddComponent(Ecs::TransformComponent{});
					Ecs::TransformComponent* transformComponent = entities.back()->GetComponent<Ecs::TransformComponent>();
					transformComponent->SetPosition(entities.size() * 20.0f, transformComponent->GetPosition().y, transformComponent->GetPosition().z);
					transformComponent->SetScale(0.1f);

					entities.back()->AddComponent(Ecs::ModelComponent{ &wnd.Gfx().LoadModelData("Assets/Models/wizard/SM_wizard.fbx") });
					entities.back()->GetComponent<Ecs::ModelComponent>()->vertexShader = ShaderFactory::GetVertexShader(wnd.Gfx(), eVertexShaderType::ModelTAAInstanced);
					entities.back()->GetComponent<Ecs::ModelComponent>()->pixelShader = ShaderFactory::GetPixelShader(wnd.Gfx(), ePixelShaderType::Model);

					entities.back()->AddComponent(Ecs::VelocityComponent{});

					wnd.Gfx().ClearRenderData();
					ecs.RegisterModelComponents(wnd.Gfx());
					wnd.Gfx().BuildRenderQueue();
					selectedEntity = entities.back()->GetID();
				}
				break;
				case 'F':
				{
					// Destroy entity
					if (entities.size() > 0)
					{
						ecs.DestroyEntity(entities.back()->GetID());
						wnd.Gfx().ClearRenderData();
						ecs.RegisterModelComponents(wnd.Gfx());
						wnd.Gfx().BuildRenderQueue();

						entities.pop_back();

						if (entities.size() > 0)
						{
							selectedEntity = entities.back()->GetID();
						}
					}
				}
				break;
				case 'R':
					wnd.Gfx().useReflectiveShadowMap = !wnd.Gfx().useReflectiveShadowMap;
					break;
			}
		}

		// TODO Should be handled by camera

		if (!wnd.CursorEnabled())
		{
			if (wnd.keyboard.KeyIsPressed(VK_SHIFT))
			{
				wnd.Gfx().camera.cameraSpeed = wnd.Gfx().camera.cameraSpeedBoost;
			}
			else
			{
				wnd.Gfx().camera.cameraSpeed = wnd.Gfx().camera.cameraSpeedDefault;
			}

			wnd.Gfx().camera.cameraInput = { 0.0f, 0.0f, 0.0f };

			if (wnd.keyboard.KeyIsPressed('W'))
			{
				wnd.Gfx().camera.cameraInput.z += 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('A'))
			{
				wnd.Gfx().camera.cameraInput.x -= 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('S'))
			{
				wnd.Gfx().camera.cameraInput.z -= 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed('D'))
			{
				wnd.Gfx().camera.cameraInput.x += 1.0f;
			}

			if (wnd.keyboard.KeyIsPressed(VK_SPACE))
			{
				wnd.Gfx().camera.cameraInput.y += 1.0f;
			}
			if (wnd.keyboard.KeyIsPressed(VK_CONTROL))
			{
				wnd.Gfx().camera.cameraInput.y -= 1.0f;
			}

			wnd.Gfx().camera.cameraVelocity = {
				Interp(wnd.Gfx().camera.cameraVelocity.x, aDeltaTime * wnd.Gfx().camera.cameraSpeed * wnd.Gfx().camera.cameraInput.x, aDeltaTime * wnd.Gfx().camera.cameraMoveInterpSpeed),
				Interp(wnd.Gfx().camera.cameraVelocity.y, aDeltaTime * wnd.Gfx().camera.cameraSpeed * wnd.Gfx().camera.cameraInput.y, aDeltaTime * wnd.Gfx().camera.cameraMoveInterpSpeed),
				Interp(wnd.Gfx().camera.cameraVelocity.z, aDeltaTime * wnd.Gfx().camera.cameraSpeed * wnd.Gfx().camera.cameraInput.z, aDeltaTime * wnd.Gfx().camera.cameraMoveInterpSpeed),
			};

			wnd.Gfx().camera.Translate(wnd.Gfx().camera.cameraVelocity);
		}

		while (const auto delta = wnd.mouse.ReadRawDelta())
		{
			if (!wnd.CursorEnabled())
			{
				wnd.Gfx().camera.Rotate(static_cast<float>(delta->x), static_cast<float>(delta->y));
			}
		}
	}
}
