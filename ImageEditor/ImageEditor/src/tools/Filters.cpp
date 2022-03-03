#include <iostream>
#include <vector>

#include "SDL.h"

#include "modules/Renderer.h"
#include "modules/Window.h"
#include "modules/Editor.h"
#include "Application.h"
#include "Filters.h"

void Filters::ApplyGrayScale(SDL_Texture* target, SDL_Texture* filter)
{
	Uint32 format = App->renderer->texture_format;
	SDL_PixelFormat* pixel_format = SDL_AllocFormat(format);

	int target_w, target_h;
	SDL_QueryTexture(target, nullptr, nullptr, &target_w, &target_h);

	App->renderer->SetRenderTarget(target);

	SDL_Surface* target_surface = SDL_CreateRGBSurfaceWithFormat(
		0,
		target_w, target_h,
		32,
		App->renderer->texture_format
	);

	if (SDL_RenderReadPixels(
		App->renderer->renderer,
		nullptr,
		App->renderer->texture_format,
		target_surface->pixels,
		target_surface->pitch)
		!= 0
		)
	{
		printf("Error reading pixels. SDL_GetError: %s\n", SDL_GetError());
	}

	App->renderer->SetRenderTarget(nullptr);

	Uint32* u_target_pixels = (Uint32*)target_surface->pixels;
	
	int pitch, w, h;
	SDL_QueryTexture(filter, nullptr, nullptr, &w, &h);

	void* filter_pixels;

	if (SDL_LockTexture(filter, nullptr, (void**)&filter_pixels, &pitch))
	{
		printf("Texture can't be locked. SDL_GetError: %s\n", SDL_GetError());
	}

	Uint32* u_filter_pixels = (Uint32*)filter_pixels;
	
	for (int i = 0; i < w * h; ++i)
	{
		Uint8 target_r, target_g, target_b;
		SDL_GetRGB(u_target_pixels[i], pixel_format, &target_r, &target_g, &target_b);
		
		Uint8 grayscale = (target_r + target_g + target_b) / 3;
		u_filter_pixels[i] = SDL_MapRGB(pixel_format, grayscale, grayscale, grayscale);
	}

	memcpy(filter_pixels, u_filter_pixels, (pitch / 4) * h);

	SDL_UnlockTexture(filter);

	SDL_FreeSurface(target_surface);

	SDL_FreeFormat(pixel_format);

	App->editor->RenderImg(App->renderer->renderer, filter, target);
}

void Filters::ApplyBlur(SDL_Texture* target, SDL_Texture* filter, const int& kernel_size)
{
	Uint32 format = App->renderer->texture_format;
	SDL_PixelFormat* pixel_format = SDL_AllocFormat(format);

	int target_w, target_h;
	SDL_QueryTexture(target, nullptr, nullptr, &target_w, &target_h);

	App->renderer->SetRenderTarget(target);

	SDL_Surface* target_surface = SDL_CreateRGBSurfaceWithFormat(
		0,
		target_w, target_h,
		32,
		App->renderer->texture_format
	);

	if (SDL_RenderReadPixels(
		App->renderer->renderer,
		nullptr,
		App->renderer->texture_format,
		target_surface->pixels,
		target_surface->pitch)
		!= 0
		)
	{
		printf("Error reading pixels. SDL_GetError: %s\n", SDL_GetError());
	}

	App->renderer->SetRenderTarget(nullptr);

	Uint32* u_target_pixels = (Uint32*)target_surface->pixels;

	int pitch, w, h;
	SDL_QueryTexture(filter, nullptr, nullptr, &w, &h);

	void* filter_pixels;

	if (SDL_LockTexture(filter, nullptr, (void**)&filter_pixels, &pitch))
	{
		printf("Texture can't be locked. SDL_GetError: %s\n", SDL_GetError());
	}

	Uint32* u_filter_pixels = (Uint32*)filter_pixels;

	std::vector<float> kernel(kernel_size * kernel_size, 0.0f);
	
	for (int i = 0; i < kernel_size; ++i)
	{
		for (int j = 0; j < kernel_size; ++j)
		{
			kernel[i * kernel_size + j] = 1.0f / 9.0f;
		}
	}

	Uint32** u_target_pixels_2d;
	u_target_pixels_2d = new Uint32*[h];
	for (int i = 0; i < h; ++i)
		u_target_pixels_2d[i] = new Uint32[w];

	Uint32** u_filter_pixels_2d;
	u_filter_pixels_2d = new Uint32 * [h];
	for (int i = 0; i < h; ++i)
		u_filter_pixels_2d[i] = new Uint32[w];

	for (int row = 0; row < h; ++row)
	{
		for (int col = 0; col < w; ++col)
		{
			u_target_pixels_2d[row][col] = Filters::Convert1dTo2dArray(u_target_pixels, row, col, w);
		}
	}

	for (int row = 0; row < h; ++row)
	{
		for (int col = 0; col < w; ++col)
		{
			Uint8 target_r, target_g, target_b;
			SDL_GetRGB(u_target_pixels_2d[row][col], pixel_format, &target_r, &target_g, &target_b);

			Uint8 grayscale = (target_r + target_g + target_b) / 3;

			u_filter_pixels_2d[row][col] = SDL_MapRGB(pixel_format, grayscale, grayscale, grayscale);;
		}
	}

	for (int row = 0; row < h; ++row)
	{
		for (int col = 0; col < w; ++col)
		{
			u_filter_pixels[row * w + col] = u_filter_pixels_2d[row][col];
		}
	}

	memcpy(filter_pixels, u_filter_pixels, (pitch / 4) * h);

	SDL_UnlockTexture(filter);

	for (int i = 0; i < h; ++i)
	{
		delete[] u_target_pixels_2d[i];
	}

	delete[] u_target_pixels_2d;

	for (int i = 0; i < h; ++i)
	{
		delete[] u_filter_pixels_2d[i];
	}

	delete[] u_filter_pixels_2d;

	SDL_FreeSurface(target_surface);

	SDL_FreeFormat(pixel_format);

	App->editor->RenderImg(App->renderer->renderer, filter, target);
}

Uint32 Filters::Convert1dTo2dArray(Uint32* array, const int& i, const int& j, const int& columns)
{
	return array[(i * columns + j)];
}