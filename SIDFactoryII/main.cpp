
#include <iostream>
#include <string>

#include "foundation/graphics/viewport.h"
#include "foundation/input/keyboard.h"
#include "foundation/input/mouse.h"
#include "foundation/platform/platform_factory.h"
#include "libraries/picopng/picopng.h"
#include "runtime/editor/editor_facility.h"
#include "utils/config/configtypes.h"
#include "utils/configfile.h"
#include "utils/delegate.h"
#include "utils/event.h"
#include "utils/keyhookstore.h"
#include "utils/utilities.h"

using namespace Foundation;
using namespace Editor;

// Forward declaration
void Run(IPlatform& inPlatform, int inArgc, char* inArgv[]);
void BuildResource();

// Functions
int main(int inArgc, char* inArgv[])
{
	//BuildResource();

	// Initialize SDL
	const int sdl_init_result = SDL_Init(SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO);
	if (sdl_init_result < 0)
	{
		std::cout << "SDL initialization failed. SDL Error: " << SDL_GetError();

		SDL_Quit();
		return -1;
	}

	// Create the platform
	IPlatform* platform = Foundation::CreatePlatform();

	// Run the editor
	Run(*platform, inArgc, inArgv);

	// Destroy the platform
	delete platform;

	// Close down SDL
	SDL_Quit();

	return 0;
}


void Run(IPlatform& inPlatform, int inArgc, char* inArgv[])
{
	// Read the config file
	std::vector<std::string> valid_configuration_sections;
	valid_configuration_sections.push_back("default");
	valid_configuration_sections.push_back(inPlatform.GetName());
#ifdef _DEBUG
	valid_configuration_sections.push_back("debug");
#endif //

	std::string config_path = inPlatform.Storage_GetConfigHomePath();
	Utility::ConfigFile configFile(inPlatform, config_path + "config.ini", valid_configuration_sections);

	// Create viewport (client view size)
	const int width = 1280;
	const int height = 720;
	float window_scaling = Utility::GetSingleConfigurationValue<Utility::Config::ConfigValueFloat>(configFile, "Window.Scaling", 1.0);

	if (window_scaling > 2.0)
	{
		window_scaling = 2.0;
		std::cout << "Warning: Window.Scaling is higher than 2.0. Limiting to 2.0";
	}
	else if (window_scaling < 0.5)
	{
		window_scaling = 0.5;
		std::cout << "Warning: Window.Scaling is lower than 0.5. Limiting to 0.5";
	}

	Viewport viewport(width, height, window_scaling, std::string("SID Factory II"));

	Mouse mouse(window_scaling);
	Keyboard keyboard;

	// Editor facility
	EditorFacility editor(&inPlatform, &viewport, configFile);

	// Start editor
	editor.Start(inArgc > 1 ? inArgv[1] : nullptr);

	// Variable for dragging the window
	bool is_dragging_window = false;
	Point mouse_position_at_start_drag;

	// Ticking test
	unsigned int last_tick = SDL_GetTicks();

	const unsigned int updates_per_second = 30;
	const unsigned int target_frame_time = 1000 / updates_per_second;

	// Listen for SDL events
	SDL_Event event;
	bool force_quit = false;

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
	SDL_EventState(SDL_WINDOWEVENT, SDL_ENABLE);

	while (!editor.IsDone() && !force_quit)
	{
		// Get tick count and maintain delta time
		const unsigned int tick = SDL_GetTicks();
		const int delta_tick = tick - last_tick;

		// Collect keyboard and mouse events
		keyboard.BeginCollect();
		mouse.BeginCollect(viewport.GetClientRectInWindow());

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				editor.TryQuit();
				break;
			case SDL_DROPFILE:
				editor.TryLoad(std::string(event.drop.file));
				SDL_free(event.drop.file);
				break;
			case SDL_TEXTINPUT:
				keyboard.KeyText(event.text.text);
				break;
			case SDL_KEYDOWN:
				keyboard.KeyDown(event.key.keysym.sym);
				break;
			case SDL_KEYUP:
				keyboard.KeyUp(event.key.keysym.sym);
				break;
			case SDL_MOUSEWHEEL:
				mouse.PushMouseWheelChange(static_cast<int>(event.wheel.x), static_cast<int>(event.wheel.y));
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					editor.OnWindowResized();
					break;
				case SDL_WINDOWEVENT_FOCUS_LOST:
					keyboard.Flush();
					break;
				}
			}
		}

		keyboard.EndCollect();
		mouse.EndCollect();

		// Update mouse
		mouse.Update(delta_tick);

		// Do dragging
		if (!is_dragging_window)
		{
			if (!mouse.IsInsideClientRect() && mouse.IsButtonPressed(Mouse::Left))
			{
				mouse_position_at_start_drag = mouse.GetPosition();
				is_dragging_window = true;
			}
		}
		else
		{
			if (!mouse.IsButtonDown(Mouse::Left))
				is_dragging_window = false;
			else
			{
				Point mouse_delta = mouse.GetPosition() - mouse_position_at_start_drag;
				Point window_position = viewport.GetWindowPosition() + mouse_delta;
				viewport.SetWindowPosition(window_position);
			}
		}

		// Update editor
		editor.Update(keyboard, mouse, delta_tick);

		// Yield the process, if it is required
		const unsigned int ticks_passed = SDL_GetTicks() - tick;

		if (ticks_passed < target_frame_time)
			SDL_Delay(target_frame_time - ticks_passed);

		// Refresh last tick
		last_tick = tick;
	}

	// Stop editor
	editor.Stop();
}


void BuildResource()
{
	//Utility::MakeBinaryResourceIncludeFile("logo_test.png", "data_logo.h", "data_logo", "Resource");
}
