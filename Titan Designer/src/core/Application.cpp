#include "Application.h"

#include "types/MethodMaster.h"
#include "WindowManager.h"
#include "input/Keyboard.h"
#include "Serializer.h"
#include "resources/XmlDocument.h"

#include "game/Scene.h"
#include "Time.h"
#include "graphics/PostProcess.h"
#include "graphics/Renderer.h"
#include "Memory.h"
#include "input/Input.h"

#include "Window.h"
#include "Windows.h"
#include "core/platform/Android.h"

#include "core/platform/Platform.h"

#include "game/SceneManager.h"
#include "input/EventManager.h"

#include "NodeManager.h"

#include "graphics/View.h"

#include "ui/SyntaxHighlighter.h"

void Application::InitSDL()
{
	SDL_Init(SDL_INIT_EVERYTHING);

	window = new Window(vec2i(2560, 1370));
	window->CreateGLContext();

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
}
void Application::InitGL()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);	

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
}

void Application::InitPhysics()
{
	
}

void Application::InitRenderer()
{
	//Titan::updatetime = 1000.0 / 60.0;
	MASTER_RENDERER->init();
}

void Application::InitEngine()
{
	Serializer::init();
	CoreNames::init();

	Mouse::init();
	Keyboard::init();
	MessageHandler::init();
	ContentManager::Init();
	TypeManager::init();
	GarbageCollector::init();
	Input::init();
	EventManager::init();
	SceneManager::init();
	Math::init();
	SyntaxMaster::init();

	INPUT->parent = this;
	InitGL();
	InitSDL();
	platform->InitGL();
	InitPhysics();
	Audio::init();
	Font::Init();
	CONTENT->setup();	

	CanvasData::init();
}

void Application::Free()
{
	INPUT->Clean();
}

void Application::prepare_draw()
{
	FBOMANAGER->clear_all();
}

void Application::FinishDraw()
{
	//RENDERER->Finish();
}

void Application::Loop()
{
	Time::Init();
	InitEngine();
	InitRenderer();
	init();
	VIEW->init(WINDOWSIZE_F / 2.0f);

	T_LOG("Finished Initialization");

	SDL_Event event;
	bool running = true;

	FPSLimiter updatelim;
	Timer timer(1000);
	FPSLimiter::FPSInfo i;

	default_target = new RenderTarget;

	//Main Loop
	while (running)
	{
		TIME->Update();

		//Handle Events
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				running = false;
			else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
					running = false;
				else if (
					event.key.keysym.sym == SDLK_F4 &&
					(event.key.keysym.mod == KMOD_LALT || event.key.keysym.mod == KMOD_RALT))
					running = false;
				else
					INPUT->HandleEvent(event);
			}
			else if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				vec2i size = vec2i(event.window.data1, event.window.data2);
				WINDOW->size = size;
				resize(size);
			}
			else if (event.type == SDL_DROPFILE)
			{
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "dropped", event.drop.file, WINDOW->window);
			}
			else
				INPUT->HandleEvent(event);
		}

		if (timer.update())						//Print FPS each time after 1.0s
		{
			GC->clean();

			//restart timer
			timer.start();
		}

		//Handle Update Loop
		i = updatelim.update();

		if (i.needsupdate)						//Sync Update Loop to 60 FPS
		{
			TIME->OnUpdate();
			update();
			//PhysicsWorld2D::update();
			INPUT->Clean();
		}

		if (default_target->should_update())
		{
			prepare_draw();
			ACTIVE_VIEWPORT->draw();
			FinishDraw();

			window->SwapBuffer();					//Switch buffers			
		}

		GC->free();

	}

	//Quit
	Quit();
}

void Application::Quit()
{
	Free();
	NodeManager::Free();
	CONTENT->FreeAll();
	Primitives::Destroy();
	//NodeManager::Free();
	GC->free();
	StringUtils::Free();
	//PhysicsWorld2D::DestroyWorld();
	FBOMANAGER->free();
	ERROR_HANDLER->clean();
	Audio::Free();
	SCENEMANAGER->Free();
	EventManager::get_manager().clean();
	MethodMaster::clean();

	WINDOWMAN->clean();

	SDL_Quit();
}