#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <GL\glew.h>
#include <GL\freeglut.h>

#include "resources/Texture.h"
#include "resources/Shader.h"
#include "world/Sprite.h"
#include "world/Mesh.h"
#include "game/Scene.h"
#include "world/Camera.h"
#include "world/Primitives.h"
#include "FBO.h"
#include "FBOManager.h"
#include "PostProcess.h"
#include "core/Stack.h"

#define MASTER_RENDERER MasterRenderer::get_singleton()
#define RENDERER MASTER_RENDERER->get_active_renderer()
#define FORWARD_RENDERER RENDERER->cast_to_type<ForwardRenderer*>()
#define DEFERRED_RENDERER RENDERER->cast_to_type<DeferredRenderer*>()

class Environment;
class Renderer;

class MasterRenderer : public Object
{
	OBJ_DEFINITION(MasterRenderer, Object);

public:

	void init();

	void set_active_renderer(Renderer* p_renderer);
	Renderer* get_active_renderer() const;

	static MasterRenderer* get_singleton();

private:
	static MasterRenderer* singleton;

	Renderer* active_renderer;
};

class Renderer : public Object
{
	OBJ_DEFINITION(Renderer, Object);

public:
	friend class Viewport;

	Renderer();
	~Renderer();

	void init();
	void check_error();
	void prepare();
	void finish();
	void resized();

	void set_camera(Camera* p_camera);
	
	void set_viewport(Viewport* p_viewport);
	Viewport* get_viewport() const;

	const mat4& get_projection_matrix() const;
	const mat4& get_view_matrix() const;
	const mat4& get_final_matrix() const;

	void use_scissor(const rect2 &area);
	void stop_scissor();

	void use_depth_test(float p_near, float p_far);
	void stop_depth_test();

	void use_culling();
	void stop_culling();

	void use_blending();
	void use_additive_blending();
	void stop_blending();

	void set_draw_on_screen(bool p_draw_on_screen);
	bool get_draw_on_screen() const;

	void use_wireframe();
	void fill();

	Texture2D* get_texture(int p_type) const;
	FBO2D* get_fbo(int p_type) const;

	virtual void render();

	void draw_plane();
	void draw_line(const vec3& p_start, const vec3& p_end, const Color& p_color);

	static void bind_methods();

protected:
	void activate();
	void deactivate();

	void set_viewport();

	void activate_world_transform();
	void deactivate_world_transform();
	void activate_canvas_transform();
	void deactivate_canvas_transform();

	void update();

	Camera* camera;
	Viewport* viewport;

	Vector<FBO2D> buffers;
	Map<int, Texture2D> textures;

	mat4 projection_matrix;
	mat4 view_matrix;
	mat4 final_matrix;
	mat4 canvas_matrix;

	bool draw_on_screen;
};

class ForwardRenderer : public Renderer
{
	OBJ_DEFINITION(ForwardRenderer, Renderer);

public:
	ForwardRenderer();
	ForwardRenderer(Viewport* p_viewport);
	virtual ~ForwardRenderer();

	void set_light_matrix(const mat4& p_light_matrix);
	const mat4& get_light_matrix() const;
		
	FBO2D* get_shadow_buffer() const;
	FBO2D* get_render_buffer() const;
	FBO2D* get_reflection_buffer() const;

	void render() override;

	static void bind_methods();

private:
	Camera *activecamera;
	Shader *activeshader;

	mat4 light_matrix;

	FBO2D* render_buffer;
	FBO2D* shadow_buffer;
	FBO2D* reflection_buffer;

	Stack<mat4> projections;
	Stack<mat4> views;
};

class DeferredRenderer : public Renderer
{
	OBJ_DEFINITION(DeferredRenderer, Renderer);

public:
	DeferredRenderer();
	virtual ~DeferredRenderer();

	enum textures_types
	{
		FINAL_COLOR,
		RENDER_COLOR,
		RENDER_DEPTH,
		DEFERRED_ALBEDO,
		DEFERRED_POSITION,
		DEFERRED_NORMAL,
		DEFERRED_MATERIAL,
		DEFERRED_DEPTH,
		DEFERRED_SPECULAR,
		REFLECTION,
		SHADOW,
		SSAO,
		SSAO_BLUR,
		GODRAY,
		BLOOM,
		DOF,
		LIGHTING
	};

	vec3 get_position_at_pixel(const vec2& p_pos) const;
	vec3 get_material_at_pixel(const vec2& p_pos) const;

	void render() override;

	String get_texture_typename(int p_type) const;

	int get_texture_type(const String& p_typename) const;

	static void bind_methods();

private:
	void generate_ssao_kernel();

	void render_bloom();
	void render_ssao();
	void render_shadowmap();
	void render_reflection();
	void render_flare();
	void render_godray();
	void render_first_pass();
	void render_second_pass();

	FBO2D* deferred_buffer;
	FBO2D* render_buffer;
	FBO2D* shadow_buffer;
	FBO2D* reflection_buffer;
	FBO2D* final_buffer;
	FBO2D* ssao_buffer;
	FBO2D* ssao_blur_buffer;
	FBO2D* godray_buffer;
	FBO2D* bloom_buffer;
	FBO2D* dof_buffer;
	FBO2D* lighting_buffer;

	FBO2D* blur_buffer;

	Camera* reflection_camera;
	Camera* light_camera;

	Shader* first_pass;
	Shader* second_pass;
	Shader* godray;
	Shader* shader_2d;
	Shader* ssao;
	Shader* bloom;

	Environment* environment;

	Vector<Texture2D> flare_textures;

	unsigned kernel_id;
	unsigned noise_id;

	Array<vec4> kernel;

	mat4 light_matrix;
};