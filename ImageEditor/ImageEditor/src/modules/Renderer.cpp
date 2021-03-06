#include <iostream>
#include <cmath>

#include "SDL2_gfxPrimitives.h"

#include "Application.h"
#include "Renderer.h"
#include "Window.h"
#include "Editor.h"
#include "Gui.h"

Renderer::Renderer()
	: Module(), renderer(nullptr), texture_target(nullptr), texture_format(SDL_PIXELFORMAT_RGBA8888)
{

}

Renderer::~Renderer()
{

}

void Renderer::Start()
{
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	this->renderer = SDL_CreateRenderer(App->window->window, -1, SDL_RENDERER_TARGETTEXTURE | SDL_RENDERER_PRESENTVSYNC);
	if (this->renderer == nullptr)
	{
		printf("Renderer can't be created. SDL_GetError: %s\n", SDL_GetError());
	}

	this->texture_target_width = App->window->width;
	this->texture_target_height = App->window->height;

	this->texture_anomaly_viewer_target = SDL_CreateTexture(this->renderer, this->texture_format, SDL_TEXTUREACCESS_TARGET, 200, 200);
	this->texture_workbench_target = SDL_CreateTexture(this->renderer, this->texture_format, SDL_TEXTUREACCESS_TARGET, App->window->width, App->window->height);
	this->texture_filter = SDL_CreateTexture(this->renderer, this->texture_format, SDL_TEXTUREACCESS_STREAMING, App->window->width, App->window->height);
	this->texture_target = SDL_CreateTexture(this->renderer, this->texture_format, SDL_TEXTUREACCESS_TARGET, App->window->width, App->window->height);

	SDL_SetRenderTarget(App->renderer->renderer, App->renderer->texture_target);

	SDL_Rect rect_screen = { 0, 0, App->window->width, App->window->height };
	SDL_SetRenderDrawColor(App->renderer->renderer, 255, 255, 255, 255);
	SDL_RenderFillRect(App->renderer->renderer, &rect_screen);

	SDL_SetRenderTarget(App->renderer->renderer, nullptr);
}

void Renderer::PreUpdate()
{
	SDL_SetRenderTarget(App->renderer->renderer, this->texture_workbench_target);

	SDL_Rect rect_screen = { 0, 0, App->window->width, App->window->height };
	SDL_SetRenderDrawColor(App->renderer->renderer, 175, 175, 175, 255);
	SDL_RenderFillRect(App->renderer->renderer, &rect_screen);

	SDL_SetRenderTarget(App->renderer->renderer, nullptr);

	SDL_SetRenderTarget(App->renderer->renderer, this->texture_target);
}

void Renderer::Update()
{

}

void Renderer::PostUpdate()
{
	if (App->editor->bg == nullptr)
	{
		SDL_Rect target_texture_rect = { 0, 0, this->texture_target_width, this->texture_target_height };
		SDL_RenderCopy(App->renderer->renderer, this->texture_target, nullptr, &target_texture_rect);
	}
	else
	{
		SDL_RenderCopy(App->renderer->renderer, this->texture_target, nullptr, &App->editor->bg_rect);
	}

	SDL_SetRenderTarget(App->renderer->renderer, nullptr);

	App->gui->Draw();

	SDL_RenderPresent(this->renderer);
}

void Renderer::CleanUp()
{
	SDL_DestroyTexture(this->texture_anomaly_viewer_target);
	SDL_DestroyTexture(this->texture_workbench_target);
	SDL_DestroyTexture(this->texture_filter);
	SDL_DestroyTexture(this->texture_target);

	SDL_DestroyRenderer(this->renderer);
}

void Renderer::DrawLine(const int& x1, const int& y1, const int& x2, const int& y2, const int& size, const ImVec4& color)
{
	thickLineRGBA(
		App->renderer->renderer,
		x1, y1,
		x2, y2,
		size,
		color.x, color.y, color.z, color.w
	);
}

void Renderer::DrawCircle(const int& x, const int& y, const int& radius, const ImVec4& color)
{
	App->renderer->SetRenderDrawColor(color.x, color.y, color.z, color.w);

	SDL_Point points[360];
	float factor = (float)M_PI / 180.0f;

	for (int i = 0; i < 360; ++i)
	{
		points[i].x = (int)(x + radius * cos(i * factor));
		points[i].y = (int)(y + radius * sin(i * factor));
	}

	SDL_RenderDrawPoints(this->renderer, points, 360);
}

void Renderer::DrawCircleFill(const int& x, const int& y, const int& radius, const ImVec4& color)
{
	App->renderer->SetRenderDrawColor(color.x, color.y, color.z, color.w);

	for (int w = 0; w < radius * 2; ++w)
	{
		for (int h = 0; h < radius * 2; ++h)
		{
			int dx = radius - w;
			int dy = radius - h;

			if ((dx * dx + dy * dy) <= (radius * radius))
			{
				SDL_RenderDrawPoint(this->renderer, x + dx, y + dy);
			}
		}
	}
}

void Renderer::DrawRectangle(const int& x, const int& y, const int& width, const int& height, const ImVec4& color)
{
	rectangleRGBA(
		this->renderer,
		x, y,
		x + width,
		y + height,
		color.x, color.y, color.z, color.w
	);
}

void Renderer::DrawRectangleFill(const int& x, const int& y, const int& width, const int& height, const ImVec4& color)
{
	boxRGBA(
		this->renderer,
		x, y,
		x + width,
		y + height,
		color.x, color.y, color.z, color.w
	);
}

void Renderer::DrawEllipse(const int& x, const int& y, const int& radius_x, const int& radius_y, const ImVec4& color)
{
	ellipseRGBA(
		this->renderer,
		x, y,
		radius_x, radius_y,
		color.x, color.y, color.z, color.w
	);
}

void Renderer::DrawEllipseFill(const int& x, const int& y, const int& radius_x, const int& radius_y, const ImVec4& color)
{
	filledEllipseRGBA(
		this->renderer,
		x, y,
		radius_x, radius_y,
		color.x, color.y, color.z, color.w
	);
}

void Renderer::SetRenderTarget(SDL_Texture* texture) const
{
	SDL_SetRenderTarget(this->renderer, texture);
}

void Renderer::SetRenderDrawColor(const int& r, const int& g, const int& b, const int& a) const
{
	SDL_SetRenderDrawColor(this->renderer, r, g, b, a);
}

SDL_Texture* Renderer::CreateTexture(const Uint32& format, const int& access, const int& width, const int& height)
{
	return SDL_CreateTexture(
		this->renderer,
		format,
		access,
		width, height
	);
}

void Renderer::RenderTexture(SDL_Texture* texture, SDL_Rect* source_rect, SDL_Rect* destination_rect)
{
	if (SDL_RenderCopy(this->renderer, texture, source_rect, destination_rect) != 0)
	{
		printf("Render copy can't be performed. SDL_GetError(): %s\n", SDL_GetError());
	}
}