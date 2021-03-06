#include "SDL_image.h"
#include "imgui.h"
#include "portable-file-dialogs.h"

#include "tools/ImageLoader.h"
#include "tools/Filters.h"
#include "utils/Utils.h"
#include "Application.h"
#include "Editor.h"
#include "Renderer.h"
#include "Window.h"
#include "Input.h"
#include "GUI.h"
#include "TorchLoader.h"

Editor::Editor()
	: Module(), bg(nullptr)
{

}

Editor::~Editor()
{

}

void Editor::Start()
{
	App->input->GetMousePosition(this->mouse_position_x, this->mouse_position_y);
	App->input->GetMousePosition(this->last_frame_mouse_position_x, this->last_frame_mouse_position_y);

	this->preferences_panel = false;
	this->anomaly_viewer_panel = false;
	this->load_hdr_image_popup = false;
	this->super_resolution_popup = false;
	this->style_transfer_popup = false;

	this->anomaly_viewer_texture = nullptr;
}

void Editor::Update()
{
	App->input->GetMousePosition(this->mouse_position_x, this->mouse_position_y);

	App->renderer->SetRenderTarget(App->renderer->texture_target);

	App->renderer->SetRenderDrawColor(
		this->tools.GetColor().x,
		this->tools.GetColor().y,
		this->tools.GetColor().z,
		this->tools.GetColor().w
	);

	switch (this->tools.current_tool)
	{
		case TOOLS::STANDARD_BRUSH:
		{
			this->UseStandardBrush();

			break;
		}
		case TOOLS::RUBBER:
		{
			this->UseRubber(SDL_BUTTON_LEFT);

			break;
		}
		case TOOLS::CIRCLE_BRUSH:
		{
			this->UseCirleBrush();

			break;
		}
		case TOOLS::CIRCLE_BRUSH_FILL:
		{
			this->UseCirleBrushFill();

			break;
		}
		case TOOLS::RECTANGLE_BRUSH:
		{
			this->UseRectangleBrush();

			break;
		}
		case TOOLS::RECTANGLE_BRUSH_FILL:
		{
			this->UseRectangleBrushFill();

			break;
		}
		case TOOLS::LINE:
		{
			this->UseLine();

			break;
		}
		case TOOLS::ELLIPSE:
		{
			this->UseEllipse();

			break;
		}
		case TOOLS::ELLIPSE_FILL:
		{
			this->UseEllipseFill();

			break;
		}
		case TOOLS::RECTANGLE:
		{
			this->UseRectangle();

			break;
		}
		case TOOLS::RECTANGLE_FILL:
		{
			this->UseRectangleFill();

			break;
		}
		default:
		{
			break;
		}
	}

	this->UseRubber(SDL_BUTTON_RIGHT);
	
	if (this->bg != nullptr)
	{
		static SDL_Point mouse_to_bg_initial_distance = { 0, 0 };

		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_STATE::KEY_DOWN)
		{
			int mouse_position_x, mouse_position_y;
			App->input->GetMousePosition(mouse_position_x, mouse_position_y);

			mouse_to_bg_initial_distance = {
				mouse_position_x - this->bg_rect.x,
				mouse_position_y - this->bg_rect.y
			};
		}

		if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_STATE::KEY_REPEAT)
		{
			int mouse_position_x, mouse_position_y;
			App->input->GetMousePosition(mouse_position_x, mouse_position_y);

			this->bg_rect = {
				mouse_position_x - mouse_to_bg_initial_distance.x,
				mouse_position_y - mouse_to_bg_initial_distance.y,
				this->bg_rect.w, this->bg_rect.h
			};
		}
	}
	
	App->renderer->SetRenderTarget(nullptr);

	this->last_frame_mouse_position_x = this->mouse_position_x;
	this->last_frame_mouse_position_y = this->mouse_position_y;
}

void Editor::CleanUp()
{
	SDL_DestroyTexture(this->anomaly_viewer_texture);
	SDL_DestroyTexture(this->bg);
}

void Editor::DrawGUI()
{
	this->MainMenuBar();
	this->ToolSelection();
	this->Panels();
	this->PopUps();
}

void Editor::MainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Load image"))
			{
				auto selection = pfd::open_file("Select a file", ".",
					{ "Image Files", "*.png *.jpg *.bmp" })
					.result();

				if (!selection.empty())
				{
					std::string path = selection[0];
					printf("User loaded file %s\n", path.c_str());

					this->bg = this->LoadImg(path);
					this->RenderImg(this->bg, App->renderer->texture_target);
				}
			}

			if (ImGui::MenuItem("Save image"))
			{
				auto destination = pfd::save_file("Select a file", ".",
					{ "Image Files", "*.png *.jpg *.bmp" },
					pfd::opt::force_overwrite).result();

				this->SaveImg(App->renderer->texture_target, destination);
				printf("User saved file %s\n", destination.c_str());
			}

			if (ImGui::MenuItem("Load HDR image"))
			{
				this->load_hdr_image_popup = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Image"))
		{
			if (ImGui::BeginMenu("Filters"))
			{
				if (ImGui::MenuItem("Grayscale"))
				{
					Filters::ApplyGrayScale(App->renderer->texture_target, App->renderer->texture_filter);
				}

				if (ImGui::MenuItem("Blur"))
				{
					Filters::ApplyBlur(App->renderer->texture_target, App->renderer->texture_filter, 3);
				}

				if (ImGui::MenuItem("Gaussian Blur"))
				{
					Filters::ApplyGaussianBlur(App->renderer->texture_target, App->renderer->texture_filter, 5);
				}

				if (ImGui::MenuItem("Negative"))
				{
					Filters::ApplyNegative(App->renderer->texture_target, App->renderer->texture_filter);
				}

				if (ImGui::MenuItem("Laplace Operator"))
				{
					Filters::ApplyLaplace(App->renderer->texture_target, App->renderer->texture_filter);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Enhancement"))
			{
				if (ImGui::BeginMenu("Denoise"))
				{
					if (ImGui::MenuItem("Median Blur"))
					{
						Filters::ApplyMedianBlur(App->renderer->texture_target, App->renderer->texture_filter, 5);
					}

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("Brighten"))
				{
					if (ImGui::MenuItem("Laplace"))
					{
						Filters::ApplyLaplaceEnhancement(App->renderer->texture_target, App->renderer->texture_filter);
					}

					ImGui::EndMenu();
				}

				if (ImGui::MenuItem("Super Resolution"))
				{
					this->super_resolution_popup = true;
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Anomaly Detection"))
			{
				if (ImGui::MenuItem("Leather"))
				{
					this->ApplyFastFlowInferenceLeather();
				}

				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Style Transfer"))
			{
				this->style_transfer_popup = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			if (ImGui::MenuItem("Anomaly Viewer"))
			{
				this->anomaly_viewer_panel = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Settings"))
		{
			if (ImGui::MenuItem("Preferences"))
			{
				this->preferences_panel = true;
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help"))
		{
			if (ImGui::MenuItem("About"))
			{
				App->RequestBrowser("https://github.com/nngg11/DIP-FDU-Project1");
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Editor::ToolSelection()
{
	ImGui::Begin("Tools");

	static const char* items[11] = {
		"Standard Brush",
		"Rubber",
		"Circle Brush",
		"Circle Brush Fill",
		"Square Brush",
		"Square Brush Fill",
		"Line",
		"Ellipse",
		"Ellipse Fill",
		"Rectangle",
		"Rectangle Fill"
	};

	ImGui::Combo("Tool", (int*)&this->tools.current_tool, items, IM_ARRAYSIZE(items));
	ImGui::ColorEdit4("Color", (float*)&this->tools.GetColorReference());
	ImGui::SliderInt("Size", &this->tools.tool_size, 1, 100, "%d", ImGuiSliderFlags_AlwaysClamp);

	ImGui::End();
}

void Editor::Panels()
{
	if (this->preferences_panel)
	{
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::Begin("Preferences", &this->preferences_panel))
		{
			ImGui::Text("User Interface");

			ImGui::Separator();

			static const char* items[] = { "Dark", "Green", "Blue", "Darker"};
			static UI_STYLE item = App->gui->style;
			
			ImGui::Text("Style");
			ImGui::SameLine();
			if (ImGui::Combo("##Combo", (int*)&item, items, IM_ARRAYSIZE(items)))
			{
				App->gui->SetUIStyle(item);
			}

			ImGui::End();
		}
	}

	if (this->anomaly_viewer_panel)
	{
		ImGui::SetNextWindowSize(ImVec2(216, 238));
		if (ImGui::Begin("Anomaly Viewer", &this->anomaly_viewer_panel, ImGuiWindowFlags_NoResize))
		{
			if (this->anomaly_viewer_texture)
			{
				App->renderer->SetRenderTarget(App->renderer->texture_anomaly_viewer_target);

				App->renderer->RenderTexture(this->anomaly_viewer_texture, nullptr, nullptr);

				App->renderer->SetRenderTarget(nullptr);

				ImGui::Image(App->renderer->texture_anomaly_viewer_target, ImVec2(200, 200));
			}
			else
			{
				ImGui::Text("No anomaly detection has been executed");
			}

			ImGui::End();
		}
	}
}

void Editor::PopUps()
{
	if (this->super_resolution_popup)
	{
		ImGui::OpenPopup("Apply Super Resolution");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Apply Super Resolution", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Options");

			ImGui::Separator();

			enum class SCALE_CONTENTS_TYPE { X2, X4 };
			static ImGuiTableFlags flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;
			static int scale_contents_type = (int)SCALE_CONTENTS_TYPE::X4;

			ImGui::AlignTextToFramePadding();
			
			ImGui::Text("Scale");
			ImGui::SameLine(); ImGui::RadioButton("x2", &scale_contents_type, (int)SCALE_CONTENTS_TYPE::X2);
			ImGui::SameLine(); ImGui::RadioButton("x4", &scale_contents_type, (int)SCALE_CONTENTS_TYPE::X4);
			ImGui::SameLine(); App->gui->HelpMarker("Upscale ratio. Default: 4");

			enum class TILE_NUM { TILE_0, TILE_128, TILE_256, TILE_512, TILE_1024, TILE_COUNT };
			static int tile_elem = (int)TILE_NUM::TILE_0;
			const char* tile_elems_names[(int)TILE_NUM::TILE_COUNT] = { "0", "128", "256", "512", "1024"};
			const char* tile_elem_name = (tile_elem >= 0 && tile_elem < (int)TILE_NUM::TILE_COUNT)
				? tile_elems_names[tile_elem]
				: "Unknown";

			ImGui::Text("Tile");
			ImGui::SameLine(); ImGui::SliderInt("##Tile", &tile_elem, 0, (int)TILE_NUM::TILE_COUNT - 1, tile_elem_name);
			ImGui::SameLine(); App->gui->HelpMarker("Tile size. Default: 0, meaning no tiles are used");

			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(100, 0)))
			{
				this->super_resolution_popup = false;
				this->ApplySuperResolution();

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 0)))
			{
				this->super_resolution_popup = false;

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	if (this->load_hdr_image_popup)
	{
		ImGui::OpenPopup("Load HDR Image");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Load HDR Image", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Setup");

			ImGui::Separator();

			static int hdr_element_number = 4;

			static std::vector<std::string> image_paths = { "", "", "", ""};
			static std::vector<float> exposure_times = { 1.0f, 1.0f, 1.0f, 1.0f };

			for (int i = 0; i < hdr_element_number; ++i)
			{
				ImGui::Text("Image %d", i + 1);
				
				ImGui::SameLine();
				ImGui::PushID(std::string(std::to_string(i).append("Input_Path")).c_str());
				ImGui::InputTextWithHint("", "Input path", (char*)image_paths[i].c_str(), image_paths[i].size());
				ImGui::PopID();

				ImGui::SameLine();
				ImGui::PushID(std::string(std::to_string(i).append("Button_More")).c_str());
				if (ImGui::Button("..."))
				{
					auto selection = pfd::open_file("Select a file", ".",
						{ "Image Files", "*.png *.jpg *.bmp" })
						.result();

					if (!selection.empty())
					{
						image_paths[i] = selection[0];
					}
				}
				ImGui::PopID();

				ImGui::SameLine(); ImGui::Text("Exposure time %d", i + 1);
				ImGui::SameLine();
				ImGui::PushID(std::string(std::to_string(i).append("Drag_Exposure_Time")).c_str());
				ImGui::DragFloat("", &exposure_times[i], 0.001f, 0.0f, 20.0f);
				ImGui::PopID();
			}

			if(ImGui::Button("+", ImVec2(20, 20)))
			{
				if (hdr_element_number < 10)
				{
					++hdr_element_number;
					image_paths.push_back("");
					exposure_times.push_back(1.0f);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button("-", ImVec2(20, 20)))
			{
				if (hdr_element_number > 1)
				{
					--hdr_element_number;
					image_paths.pop_back();
					exposure_times.pop_back();
				}
			}
			ImGui::SameLine();
			App->gui->HelpMarker("Locked between 1-10 values");

			ImGui::Separator();
			
			static TONEMAP_TYPE tonemap_type = TONEMAP_TYPE::DRAGO;

			static const char* tonemaps[3] = {
				"Drago",
				"Reinhard",
				"Mantiuk"
			};

			ImGui::Text("Tonemap");
			ImGui::SameLine();
			ImGui::Combo("##Tonemap", (int*)&tonemap_type, tonemaps, IM_ARRAYSIZE(tonemaps));

			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(100, 0)))
			{
				this->ApplyLoadHDRImage(image_paths, exposure_times, tonemap_type);
				
				this->load_hdr_image_popup = false;


				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 0)))
			{				
				for (int i = 0; i < hdr_element_number; ++i)
				{
					image_paths[i].clear();
					exposure_times[i] = 1.0f;
				}

				this->load_hdr_image_popup = false;

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}

	if (this->style_transfer_popup)
	{
		ImGui::OpenPopup("Style Transfer");

		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Style Transfer", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Setup");

			ImGui::Separator();

			static char content_image_path[MAX_PATH];

			ImGui::InputTextWithHint("##ContentImage", "Content image path", content_image_path, IM_ARRAYSIZE(content_image_path));
			ImGui::SameLine();
			if (ImGui::Button("...##ContentImageButton"))
			{
				auto selection = pfd::open_file("Select a file", ".",
					{ "Image Files", "*.png *.jpg" })
					.result();

				if (!selection.empty())
				{
					std::string res = selection[0];
					strcpy_s(content_image_path, res.c_str());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Content path");

			static char style_image_path[MAX_PATH];

			ImGui::InputTextWithHint("##StyleImage", "Style image path", style_image_path, IM_ARRAYSIZE(style_image_path));
			ImGui::SameLine();
			if (ImGui::Button("...##StyleImageButton"))
			{
				auto selection = pfd::open_file("Select a file", ".",
					{ "Image Files", "*.png *.jpg" })
					.result();

				if (!selection.empty())
				{
					std::string res = selection[0];
					strcpy_s(style_image_path, res.c_str());
				}
			}
			ImGui::SameLine();
			ImGui::Text("Style path");

			ImGui::Separator();

			ImGui::Checkbox("Use URST", &App->torch_loader->style_transfer_params.USE_URST);

			if (App->torch_loader->style_transfer_params.USE_URST)
			{
				ImGui::Separator();

				ImGui::Checkbox("High Resolution Mode", &App->torch_loader->style_transfer_params.HIGH_RES_MODE);
				ImGui::DragInt("Thumb Size", &App->torch_loader->style_transfer_params.THUMB_SIZE, 1.0f, 0, 4096);
				ImGui::DragInt("Patch Size", &App->torch_loader->style_transfer_params.PATCH_SIZE, 1.0f, 0, 4000);
				ImGui::SliderInt("Padding", &App->torch_loader->style_transfer_params.PADDING, 0, 128);

				ImGui::Separator();
			}

			ImGui::DragInt("Resize Horizontal", &App->torch_loader->style_transfer_params.RESIZE_H, 5.0f, 0, 7000);
			ImGui::DragInt("Resize Vertical", &App->torch_loader->style_transfer_params.RESIZE_V, 5.0f, 0, 7000);
			ImGui::DragInt("Style Size", &App->torch_loader->style_transfer_params.STYLE_SIZE, 1.0f, 0, 4096);
			ImGui::SliderFloat("Alpha", &App->torch_loader->style_transfer_params.ALPHA, 0.0f, 1.0f);

			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(100, 0)))
			{
				this->ApplyStyleTransferInference(content_image_path, style_image_path);

				this->style_transfer_popup = false;

				ImGui::CloseCurrentPopup();
			}

			ImGui::SetItemDefaultFocus();
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(100, 0)))
			{
				this->style_transfer_popup = false;

				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}

void Editor::UseStandardBrush()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN
		|| App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_REPEAT)
	{
		App->renderer->DrawLine(
			this->last_frame_mouse_position_x - this->bg_rect.x, this->last_frame_mouse_position_y - this->bg_rect.y,
			this->mouse_position_x - this->bg_rect.x, this->mouse_position_y - this->bg_rect.y,
			this->tools.tool_size,
			this->tools.GetColor()
		);
	}
}

void Editor::UseRubber(const int& button)
{
	if (App->input->GetMouseButton(button) == KEY_STATE::KEY_DOWN
		|| App->input->GetMouseButton(button) == KEY_STATE::KEY_REPEAT)
	{
		App->renderer->DrawCircleFill(
			mouse_position_x - this->bg_rect.x, mouse_position_y - this->bg_rect.y,
			this->tools.tool_size,
			ImVec4(255, 255, 255, 255)
		);
	}
}

void Editor::UseCirleBrush()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN
		|| App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_REPEAT)
	{
		App->renderer->DrawCircle(
			mouse_position_x - this->bg_rect.x, mouse_position_y - this->bg_rect.y,
			this->tools.tool_size,
			this->tools.GetColor()
		);
	}
}

void Editor::UseCirleBrushFill()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN
		|| App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_REPEAT)
	{
		App->renderer->DrawCircleFill(
			mouse_position_x - this->bg_rect.x, mouse_position_y - this->bg_rect.y,
			this->tools.tool_size,
			this->tools.GetColor()
		);
	}
}

void Editor::UseRectangleBrush()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN
		|| App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_REPEAT)
	{
		App->renderer->DrawRectangle(
			mouse_position_x - this->tools.tool_size / 2 - this->bg_rect.x, mouse_position_y - this->tools.tool_size / 2 - this->bg_rect.y,
			this->tools.tool_size, this->tools.tool_size,
			this->tools.GetColor()
		);
	}
}

void Editor::UseRectangleBrushFill()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN
		|| App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_REPEAT)
	{
		App->renderer->DrawRectangleFill(
			mouse_position_x - this->tools.tool_size / 2 - this->bg_rect.x, mouse_position_y - this->tools.tool_size / 2 - this->bg_rect.y,
			this->tools.tool_size, this->tools.tool_size,
			this->tools.GetColor()
		);
	}
}

void Editor::UseLine()
{
	static SDL_Point initial_mouse_position = { 0, 0 };
	static SDL_Point final_mouse_position = { 0, 0 };

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		initial_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};
	}

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_UP)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		final_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};

		App->renderer->DrawLine(
			initial_mouse_position.x - this->bg_rect.x, initial_mouse_position.y - this->bg_rect.y,
			final_mouse_position.x - this->bg_rect.x, final_mouse_position.y - this->bg_rect.y,
			this->tools.tool_size,
			this->tools.GetColor()
		);
	}
}

void Editor::UseEllipse()
{
	static SDL_Point initial_mouse_position = { 0, 0 };
	static SDL_Point final_mouse_position = { 0, 0 };

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		initial_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};
	}

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_UP)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		final_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};

		SDL_Point mid_point = {
			(final_mouse_position.x - initial_mouse_position.x) / 2 + initial_mouse_position.x,
			(final_mouse_position.y - initial_mouse_position.y) / 2 + initial_mouse_position.y
		};

		SDL_Point radius = {
			abs((final_mouse_position.x - initial_mouse_position.x) / 2),
			abs((final_mouse_position.y - initial_mouse_position.y) / 2)
		};

		App->renderer->DrawEllipse(
			mid_point.x - this->bg_rect.x, mid_point.y - this->bg_rect.y,
			radius.x, radius.y,
			this->tools.GetColor()
		);
	}
}

void Editor::UseEllipseFill()
{
	static SDL_Point initial_mouse_position = { 0, 0 };
	static SDL_Point final_mouse_position = { 0, 0 };

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		initial_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};
	}

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_UP)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		final_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};

		SDL_Point mid_point = {
			(final_mouse_position.x - initial_mouse_position.x) / 2 + initial_mouse_position.x,
			(final_mouse_position.y - initial_mouse_position.y) / 2 + initial_mouse_position.y
		};

		SDL_Point radius = {
			abs((final_mouse_position.x - initial_mouse_position.x) / 2),
			abs((final_mouse_position.y - initial_mouse_position.y) / 2)
		};

		App->renderer->DrawEllipseFill(
			mid_point.x - this->bg_rect.x, mid_point.y - this->bg_rect.y,
			radius.x, radius.y,
			this->tools.GetColor()
		);
	}
}

void Editor::UseRectangle()
{
	static SDL_Point initial_mouse_position = { 0, 0 };
	static SDL_Point final_mouse_position = { 0, 0 };

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		initial_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};
	}

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_UP)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		final_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};

		SDL_Point diff_point = {
			final_mouse_position.x - initial_mouse_position.x,
			final_mouse_position.y - initial_mouse_position.y
		};

		App->renderer->DrawRectangle(
			initial_mouse_position.x - this->bg_rect.x, initial_mouse_position.y - this->bg_rect.y,
			diff_point.x, diff_point.y,
			this->tools.GetColor()
		);
	}
}

void Editor::UseRectangleFill()
{
	static SDL_Point initial_mouse_position = { 0, 0 };
	static SDL_Point final_mouse_position = { 0, 0 };

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_DOWN)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		initial_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};
	}

	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_STATE::KEY_UP)
	{
		int mouse_position_x, mouse_position_y;
		App->input->GetMousePosition(mouse_position_x, mouse_position_y);

		final_mouse_position = {
			mouse_position_x,
			mouse_position_y
		};

		SDL_Point diff_point = {
			final_mouse_position.x - initial_mouse_position.x,
			final_mouse_position.y - initial_mouse_position.y
		};

		App->renderer->DrawRectangleFill(
			initial_mouse_position.x - this->bg_rect.x, initial_mouse_position.y - this->bg_rect.y,
			diff_point.x, diff_point.y,
			this->tools.GetColor()
		);
	}
}

void Editor::ApplySuperResolution()
{
	App->renderer->SetRenderTarget(App->renderer->texture_target);

	std::string extension = ".jpg";

	std::string input_path;

	char temp_filename[MAX_PATH] = { 0 };
	tmpnam_s(temp_filename);

	input_path.append(temp_filename).append(extension);

	ImageLoader::SaveTexture(App->renderer->renderer, App->renderer->texture_target, input_path);

	std::string out_path;

	char out_temp_filename[MAX_PATH] = { 0 };
	tmpnam_s(out_temp_filename);

	out_path.append(out_temp_filename).append(extension);

	std::string command = std::string("-n ").append("realesrgan-x4plus")
		.append(" -i ").append(input_path)
		.append(" -o ").append(out_path);

	wchar_t* cmd = CharArrayToLPCWSTR(command.c_str());

	SHELLEXECUTEINFO ShExecInfo = { 0 };
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = L".\\vendor\\bin\\realesrgan-ncnn-vulkan\\realesrgan-ncnn-vulkan.exe";
	ShExecInfo.lpParameters = cmd;
	ShExecInfo.lpDirectory = NULL;
	ShExecInfo.nShow = SW_HIDE;
	ShExecInfo.hInstApp = NULL;

	printf("Loading...\n");

	ShellExecuteEx(&ShExecInfo);
	if (ShExecInfo.hProcess != NULL)
	{
		WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
		CloseHandle(ShExecInfo.hProcess);
	}

	delete cmd;

	App->renderer->SetRenderTarget(nullptr);

	this->bg = this->LoadImg(out_path);
	this->RenderImg(this->bg, App->renderer->texture_target);
}

void Editor::ApplyLoadHDRImage(const std::vector<std::string>& image_paths, const std::vector<float>& exposure_times, const TONEMAP_TYPE& tonemap_type)
{
	HDRLoader::LoadHDRImage(image_paths, exposure_times, tonemap_type);
}

void Editor::ApplyFastFlowInferenceLeather()
{
	App->renderer->SetRenderTarget(App->renderer->texture_target);

	std::string input_extension = ".png";
	std::string input_path;

	char input_temp_filename[MAX_PATH] = { 0 };
	tmpnam_s(input_temp_filename);

	input_path.append(input_temp_filename).append(input_extension);

	ImageLoader::SaveTexture(App->renderer->renderer, App->renderer->texture_target, input_path);

	cv::Mat result = App->torch_loader->FastFlowInference(input_path);

	std::string output_extension = ".png";
	std::string output_path;

	char output_temp_filename[MAX_PATH] = { 0 };
	tmpnam_s(output_temp_filename);

	output_path.append(output_temp_filename).append(output_extension);

	cv::imwrite(output_path, result);

	//ImageLoader::SendMatToEditor(result, false);

	if (this->anomaly_viewer_texture != nullptr) SDL_DestroyTexture(this->anomaly_viewer_texture);
	this->anomaly_viewer_texture = ImageLoader::LoadTexture(App->renderer->renderer, output_path);
	this->anomaly_viewer_panel = true;

	result.release();
}

void Editor::ApplyStyleTransferInference(const std::string& content_path, const std::string& style_path)
{
	cv::Mat result = App->torch_loader->StyleTransferInference(content_path, style_path);

	ImageLoader::SendMatToEditor(result, false);

	result.release();
}

SDL_Texture* Editor::LoadImg(const std::string& path) const
{
	SDL_Texture* texture = ImageLoader::LoadTexture(App->renderer->renderer, path);
	
	int width, height;
	ImageLoader::GetTextureDimensions(texture, &width, &height);

	if (App->renderer->texture_workbench_target) SDL_DestroyTexture(App->renderer->texture_workbench_target);
	if (App->renderer->texture_filter) SDL_DestroyTexture(App->renderer->texture_filter);
	if (App->renderer->texture_target) SDL_DestroyTexture(App->renderer->texture_target);

	App->renderer->texture_workbench_target = App->renderer->CreateTexture(
		App->renderer->texture_format,
		SDL_TEXTUREACCESS_STREAMING,
		width, height
	);

	App->renderer->texture_filter = App->renderer->CreateTexture(
		App->renderer->texture_format,
		SDL_TEXTUREACCESS_STREAMING,
		width, height
	);

	App->renderer->texture_target = App->renderer->CreateTexture(
		App->renderer->texture_format,
		SDL_TEXTUREACCESS_TARGET,
		width, height
	);

	App->renderer->texture_target_width = width;
	App->renderer->texture_target_height = height;

	return texture;
}

void Editor::SaveImg(SDL_Texture* texture, const std::string& path) const
{
	App->renderer->SetRenderTarget(texture);
	ImageLoader::SaveTexture(App->renderer->renderer, texture, path);
}

void Editor::RenderImg(SDL_Texture* texture, SDL_Texture* target, const bool& assign_new_bg_rect)
{
	App->renderer->SetRenderTarget(target);

	int w, h;
	ImageLoader::GetTextureDimensions(texture, &w, &h);

	if(assign_new_bg_rect)
		this->bg_rect = { 0, 0, w, h };

	if (texture)
	{
		App->renderer->RenderTexture(texture, nullptr, nullptr);
	}

	App->renderer->SetRenderTarget(nullptr);
}