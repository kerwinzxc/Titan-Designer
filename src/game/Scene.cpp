#include "Scene.h"

#include "graphics/View.h"

#include "graphics/Renderer.h"
#include "Game.h"

#include "input/EventHandler.h"

#include "SceneManager.h"
#include "core/WindowManager.h"

#include "ui/Label.h"

#include "core/Serializer.h"

#include "world/Model.h"
#include "world/Terrain.h"
#include "world/Sky.h"

#include "input/Keyboard.h"

#include "world\Raycaster.h"

Scene::Scene(const String &p_name)
{
	set_name(p_name);

	SCENEMANAGER->add_scene(this);
}

Scene::~Scene()
{
	SCENEMANAGER->remove_scene(name);
}

void Scene::Init()
{	
	LoadContent();
	
	Viewport* v = VIEW->get_active_viewport();

	//mouse_position = MOUSE->get_position();

	if (v)
	{
		World2D* w = new World2D;

		v->set_world(w);

		Model* m = new Model("assets/Models/672a8302d72f34f79980861feb2bdcd5/untitled.dae");
		m->set_pos(vec3(50, 50, 25));
		m->set_size(vec3(10.0f));
		m->set_name("plane");

		Terrain* t = new Terrain();
		t->set_name("terrain");

		Water* water = new Water;
		water->set_name("water");

		Sky* sky = new Sky;

		Clouds* c = new Clouds;
		c->set_name("clouds");
		
		Vegetation* veg = new Vegetation(t);

		w->add_worldobject(sky);
		w->add_worldobject(m);
		w->add_worldobject(t);
		w->add_worldobject(water);
		w->add_worldobject(veg);
		w->add_worldobject(c);

		w->get_active_camera()->set_rotation(vec3(0, 0, PI));

		w->init();
	}
}

void Scene::LoadContent()
{
	//run("Init");
}

void Scene::Start()
{
	Game::GetGame()->StartScene(this);
}

void Scene::update()
{
	Camera* c = VIEW->get_active_viewport()->get_world()->get_active_camera();
	Terrain* t = VIEW->get_active_viewport()->get_world()->get_worldobject("terrain")->cast_to_type<Terrain*>();

	vec3 rotate_speed = 1.0f * 0.000001f * TIME->get_deltatime();
	vec3 movement_speed = 10.0f * 0.000001f * TIME->get_deltatime();

	//rotation

	if (KEYBOARD->is_button_pressed(Key::KEY_UP))
		c->rotate(rotate_speed * vec3(1, 0, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_DOWN))
		c->rotate(rotate_speed * vec3(-1, 0, 0));
		
	if (KEYBOARD->is_button_pressed(Key::KEY_LEFT))
		c->rotate(rotate_speed * vec3(0, 1, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_RIGHT))
		c->rotate(rotate_speed * vec3(0, -1, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_M))
		c->rotate(rotate_speed * vec3(0, 0, 1));

	if (KEYBOARD->is_button_pressed(Key::KEY_N))
		c->rotate(rotate_speed * vec3(0, 0, -1));

	//movement

	if (KEYBOARD->is_button_pressed(Key::KEY_A))
		c->move(movement_speed * vec3(-1, 0, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_D))
		c->move(movement_speed * vec3(1, 0, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_W))
		c->move(movement_speed * vec3(0, 1, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_S))
		c->move(movement_speed * vec3(0, -1, 0));

	if (KEYBOARD->is_button_pressed(Key::KEY_E))
		c->move(movement_speed * vec3(0, 0, 1));

	if (KEYBOARD->is_button_pressed(Key::KEY_F))
		c->move(movement_speed * vec3(0, 0, -1));

	if (KEYBOARD->is_button_pressed(Key::KEY_T))
		CONTENT->LoadShader("EngineCore/PostProcess")->reload();

	vec3 pos = c->get_pos();
	float height = t->get_height(pos.get_xy());

	if (c->get_pos().z < height + 1.0f)
		c->set_pos(vec3(pos.x, pos.y, height + 1.0f));
		
	c->look_at(c->get_pos() + vec3(0, 1, 0), vec3(0, 0, -1));

	VIEW->update();

	Raycaster r = Raycaster(c);
	vec3 p = r.raycast(MOUSE->get_position());
}

void Scene::Draw()
{	
	VIEW->draw();
}

void Scene::handle_event(Event *e)
{
	InputEvent* input = reinterpret_cast<InputEvent*>(e);

	if (!input)
		return;

	switch (input->type)
	{
	case InputEvent::KEYPRESS:

		

		break;


	}
}

#undef CLASSNAME
#define CLASSNAME Scene

void Scene::bind_methods()
{
	REG_METHOD(Start);
}