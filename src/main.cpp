#include <GL/glew.h>
#include <SDL2/SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cassert>
#include <cmath>
#include <vector>
#include <utility>
#include <iterator>
#include <random>
#include <algorithm>
#include <iostream>
#include <unordered_map>
#include <sstream>

#include "objparser.hpp"
#include "sdlmanager.hpp"
#include "camera.hpp"
#include "fps_meter.hpp"
#include "shader.hpp"

struct RenderInfo {
	GLuint gBuffer;
	GLuint gPosition, gNormal, gAlbedoSpec, gDepth;
};

void RenderFullscreenQuad();
void PrepareGbuffer(size_t x, size_t y, RenderInfo& data);
void DeleteGbuffer(RenderInfo& data);
glm::mat4 Resize(SDL_Manager& sdlm, RenderInfo& data);

int main(int argc, char ** argv) {
	(void) argc;
	(void) argv;

	//////////////////////////
	// Parse an object file //
	//////////////////////////

	auto file = parse_obj_file("monkey.wavobj");
	auto worldfile = parse_obj_file("world.wavobj");

	//auto test_vertex = file.objects[0].vertices[11];

	// Check arbitrary vertex for correctness
	//assert(std::abs(test_vertex.x - (-0.704000f)) <= 0.01);
	//assert(std::abs(test_vertex.y - (0.648569f)) <= 0.01);
	//assert(std::abs(test_vertex.z - (0.039883f)) <= 0.01);
	//assert(test_vertex.tx == 0);
	//assert(test_vertex.ty == 0);
	//assert(std::abs(test_vertex.nx - (0.056703f)) <= 0.01);
	//assert(std::abs(test_vertex.ny - (-0.947539f)) <= 0.01);
	//assert(std::abs(test_vertex.nz - (0.314524f)) <= 0.01);

	///////////////
	// SDL Setup //
	///////////////

	SDL_Manager sdlm;
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

	///////////////////////////////////
	// Setup Random Number Generator //
	///////////////////////////////////

	std::mt19937 prng(std::random_device{}());

	/////////////////
	// Shader Prep //
	/////////////////

	Shader_Program geometrypass;
	geometrypass.add("shaders/geometry.v.glsl", Shader::VERTEX);
	geometrypass.add("shaders/geometry.f.glsl", Shader::FRAGMENT);
	geometrypass.compile();
	geometrypass.link();
	geometrypass.use();

	auto uGeoWorld = geometrypass.getUniform("world", Shader::MANDITORY);
	auto uGeoView = geometrypass.getUniform("view", Shader::MANDITORY);
	auto uGeoProjection = geometrypass.getUniform("projection", Shader::MANDITORY);

	auto world_world = glm::scale(glm::translate(glm::mat4(), glm::vec3(0, -1, 0)), glm::vec3(10, 2, 10));
	auto monkey_world = glm::translate(glm::mat4(), glm::vec3(0, 0, 0));
	auto projection = glm::perspective(glm::radians(60.0f), sdlm.size.ratio, 0.1f, 1000.0f);

	Shader_Program lightingpass;
	lightingpass.add("shaders/lighting.v.glsl", Shader::VERTEX);
	lightingpass.add("shaders/lighting.f.glsl", Shader::FRAGMENT);
	lightingpass.compile();
	lightingpass.link();

	auto uLightViewPos = lightingpass.getUniform("viewPos");

	// Set gBuffer textures
	lightingpass.use();
	glUniform1i(lightingpass.getUniform("gPosition"), 0);
	glUniform1i(lightingpass.getUniform("gNormal"), 1);
	glUniform1i(lightingpass.getUniform("gAlbedoSpec"), 2);

	Shader_Program lightbound;
	lightbound.add("shaders/lighteffect.v.glsl", Shader::VERTEX);
	lightbound.add("shaders/lighteffect.f.glsl", Shader::FRAGMENT);
	lightbound.compile();
	lightbound.link();

	auto uLightBoundView = lightbound.getUniform("view", Shader::MANDITORY);
	auto uLightBoundPerspective = lightbound.getUniform("perspective", Shader::MANDITORY);
	auto uLightBoundViewPos = lightbound.getUniform("viewPos");
	auto uLightBoundResolution = lightbound.getUniform("resolution");

	lightbound.use();
	glUniform1i(lightbound.getUniform("gPosition"), 0);
	glUniform1i(lightbound.getUniform("gNormal"), 1);
	glUniform1i(lightbound.getUniform("gAlbedoSpec"), 2);

	Shader_Program drawlights;
	drawlights.add("shaders/drawlight.v.glsl", Shader::VERTEX);
	drawlights.add("shaders/drawlight.f.glsl", Shader::FRAGMENT);
	drawlights.compile();
	drawlights.link();

	auto uDrawLightsView = drawlights.getUniform("view", Shader::MANDITORY);
	auto uDrawLightsPerspective = drawlights.getUniform("perspective", Shader::MANDITORY);

	///////////////////////
	// Vertex Array Prep //
	///////////////////////

	GLuint Monkey_VAO, Monkey_VBO;
	glGenVertexArrays(1, &Monkey_VAO);
	glBindVertexArray(Monkey_VAO);

	glGenBuffers(1, &Monkey_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Monkey_VBO);
	glBufferData(GL_ARRAY_BUFFER, file.objects[0].vertices.size() * sizeof(Vertex), file.objects[0].vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (0 * sizeof(GLfloat))); // Position
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat))); // Texcoords
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (5 * sizeof(GLfloat))); // Normals
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	// World
	GLuint World_VAO, World_VBO;
	glGenVertexArrays(1, &World_VAO);
	glBindVertexArray(World_VAO);

	glGenBuffers(1, &World_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, World_VBO);
	glBufferData(GL_ARRAY_BUFFER, worldfile.objects[0].vertices.size() * sizeof(Vertex), worldfile.objects[0].vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (0 * sizeof(GLfloat))); // Position
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat))); // Texcoords
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*) (5 * sizeof(GLfloat))); // Normals
	
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);

	////////////
	// Lights //
	////////////

	size_t lightcount = 0;

	struct LightData {
		float distance;
		float orbit;
		float height;
		float size;
	};

	std::vector<LightData> lightdata;
	std::vector<glm::vec3> lightposition;
	std::vector<glm::mat4> lightworldmatrix;
	std::vector<glm::mat4> lighteffectworldmatrix;
	std::vector<glm::vec3> lightcolor;

	lightdata.reserve(lightcount);
	lightposition.reserve(lightcount);
	lightworldmatrix.reserve(lightcount);
	lighteffectworldmatrix.reserve(lightcount);
	lightcolor.reserve(lightcount);

	// Color
	std::uniform_real_distribution<float> color_distribution(0, 1);
	std::uniform_real_distribution<float> intensity_distribution(0.1, 3);
	// Position
	std::uniform_real_distribution<float> position_dist_distribution(1, 30);
	std::uniform_real_distribution<float> position_orbit_distribution(0.0f, glm::two_pi<float>());
	std::uniform_real_distribution<float> position_height_distribution(-2.5, 2.5);

	auto create_single_light = [&] {
		// Color
		lightcolor.emplace_back(glm::normalize(glm::vec3(color_distribution(prng), color_distribution(prng), color_distribution(prng))) * intensity_distribution(prng));

		// Position
		auto&& colorit = lightcolor.end() - 1;
		LightData ret;
		ret.distance = position_dist_distribution(prng);
		ret.orbit = position_orbit_distribution(prng);
		ret.height = position_height_distribution(prng);
		constexpr float constant = 1.0;
		constexpr float linear = 0.7;
		constexpr float quadratic = 1.8;
		float lightMax = std::max(std::max(colorit->r, colorit->g), colorit->b);
		ret.size = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0 / 4.0) * lightMax))) / (2 * quadratic);
		lightdata.push_back(std::move(ret));

		lightposition.emplace_back();
		lightworldmatrix.emplace_back();
		lighteffectworldmatrix.emplace_back();

		lightcount += 1;
	};

	auto create_light = [&](size_t count = 10) {
		for (size_t i = 0; i < count; ++i) {
			create_single_light();
		}

		std::cerr << count << " lights added. " << lightcount << " total.\n";
	};

	auto remove_single_light = [&] {
		if (lightcount) {
			lightdata.pop_back();
			lightposition.pop_back();
			lightworldmatrix.pop_back();
			lighteffectworldmatrix.pop_back();
			lightcolor.pop_back();
			lightcount -= 1;
		}
		else {
			std::cerr << "Dammit you, there are no more lights left!\n";
		}
	};

	auto remove_light = [&](size_t count = 10) {
		for (size_t i = 0; i < count; ++i) {
			remove_single_light();
		}

		std::cerr << count << " lights removed. " << lightcount << " total.\n";
	};

	create_light(20);

	auto circlefile = parse_obj_file("sphere.wavobj");
	auto squarefile = parse_obj_file("square.wavobj");

	GLuint Light_VAO, Light_VBO;
	GLuint LightCircle_VBO;
	GLuint LightTransform_VBO;
	GLuint LightColor_VBO;
	GLuint LightPosition_VBO;
	glGenVertexArrays(1, &Light_VAO);
	glBindVertexArray(Light_VAO);

	glGenBuffers(1, &Light_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, Light_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), NULL);
	glBufferData(GL_ARRAY_BUFFER, squarefile.objects[0].vertices.size() * sizeof(Vertex), squarefile.objects[0].vertices.data(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &LightTransform_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, LightTransform_VBO);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (0 * sizeof(GLfloat)));
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (4 * sizeof(GLfloat)));
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (8 * sizeof(GLfloat)));
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*) (12 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);

	glGenBuffers(1, &LightColor_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, LightColor_VBO);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), NULL);
	glEnableVertexAttribArray(1);
	glVertexAttribDivisor(1, 1);

	glGenBuffers(1, &LightPosition_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, LightPosition_VBO);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), NULL);
	glEnableVertexAttribArray(6);
	glVertexAttribDivisor(6, 1);

	glGenBuffers(1, &LightCircle_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, LightCircle_VBO);
	glBufferData(GL_ARRAY_BUFFER, circlefile.objects[0].vertices.size() * sizeof(Vertex), circlefile.objects[0].vertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL);
	
	glBindVertexArray(0);

	/////////////////////
	// Prepare gBuffer //
	/////////////////////

	RenderInfo reninfo;
	PrepareGbuffer(WINDOW_WIDTH, WINDOW_HEIGHT, reninfo);

	///////////////
	// Game Loop //
	///////////////

	bool loop = true;
	bool fullscreen = false, gotmouse = true;
	std::unordered_map<SDL_Keycode, bool> keys;

	SDL_SetRelativeMouseMode(SDL_TRUE);

	float mouseX = 0, mouseY = 0;
	float mouseLX = 0, mouseLY = 0;
	float mouseDX = 0, mouseDY = 0;
	(void) mouseLX;
	(void) mouseLY;

	FPS_Meter fps(true, 2);
	Camera cam(glm::vec3(0, 10, 25));
		
	///////////////
	// Game Loop //
	///////////////

	while (loop) {
		int mousePixelX, mousePixelY;
		SDL_GetRelativeMouseState(&mousePixelX, &mousePixelY);

		mouseDX = (mousePixelX / (float) sdlm.size.height);
		mouseDY = (mousePixelY / (float) sdlm.size.height);

		mouseX = mouseX + mouseDX;
		mouseY = mouseY + mouseDY;

		mouseLX = std::exchange(mouseX, mouseX + mouseDX);
		mouseLY = std::exchange(mouseX, mouseY + mouseDY);

		if (mouseDX || mouseDY) {
			cam.rotate(mouseDY, mouseDX, 50);
		}

		fps.frame(lightcount);

		const float cameraSpeed = 5.0f * fps.get_delta_time();

		// Event Handling
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					loop = false;
					break;
				case SDL_WINDOWEVENT:
					if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
						projection = Resize(sdlm, reninfo);
					}
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_ESCAPE:
							loop = false;
							break;
						case SDLK_F10:
							if (fullscreen) {
								SDL_SetWindowFullscreen(sdlm.mainWindow, 0);
								fullscreen = false;
							}
							else {
								SDL_SetWindowFullscreen(sdlm.mainWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
								fullscreen = true;
							}
							break;
						case SDLK_0:
							remove_light(lightcount);
							break;
						case SDLK_RIGHTBRACKET:
							create_light(10);
							break;
						case SDLK_LEFTBRACKET:
							remove_light(10);
							break;
						case SDLK_EQUALS:
							create_light(1);
							break;
						case SDLK_MINUS:
							remove_light(1);
							break;
						case SDLK_LALT:
						case SDLK_RALT:
							if (gotmouse) {
								SDL_SetRelativeMouseMode(SDL_FALSE);
							}
							else {
								SDL_SetRelativeMouseMode(SDL_TRUE);
							}
							gotmouse = !gotmouse;
							break;
						default:
							break;
					}
					keys[event.key.keysym.sym] = true;
					break;
				case SDL_KEYUP:
					keys[event.key.keysym.sym] = false;
					break;
			}
		}

		if (keys[SDLK_w]) {
			cam.move(glm::vec3(0, 0, cameraSpeed));
		}
		if (keys[SDLK_s]) {
			cam.move(glm::vec3(0, 0, -cameraSpeed));
		}
		if (keys[SDLK_a]) {
			cam.move(glm::vec3(-cameraSpeed, 0, 0));
		}
		if (keys[SDLK_d]) {
			cam.move(glm::vec3(cameraSpeed, 0, 0));
		}
		if (keys[SDLK_LSHIFT]) {
			cam.move(glm::vec3(0, cameraSpeed, 0));
		}
		if (keys[SDLK_LCTRL]) {
			cam.move(glm::vec3(0, -cameraSpeed, 0));
		}

		// Update Light Transforms
		for (size_t i = 0; i < lightcount; ++i) {
			auto&& lp = lightdata[i];
			lp.orbit += glm::radians(15.0f * fps.get_delta_time());

			glm::mat4 height = glm::translate(glm::mat4(), glm::vec3(0, lp.height, 0));
			glm::mat4 orbit = glm::rotate(glm::mat4(), lp.orbit, glm::vec3(0, 1, 0));
			glm::mat4 trans = glm::translate(glm::mat4(), glm::vec3(0, 0, -lp.distance));
			glm::mat4 scale = glm::scale(glm::mat4(), glm::vec3(lp.size * 0.05));
			glm::mat4 effectscale = glm::scale(glm::mat4(), glm::vec3(lp.size));

			glm::mat4 unscaled = orbit * height * trans;
			lightposition[i] = glm::vec3(unscaled * glm::vec4(0, 0, 0, 1));
			lightworldmatrix[i] = unscaled * scale;
			lighteffectworldmatrix[i] = unscaled * effectscale;
		}

		///////////////////
		// Geometry Pass //
		///////////////////

		// Use geometry pass shaders
		geometrypass.use();

		// Update matrix uniforms
		glUniformMatrix4fv(uGeoWorld, 1, GL_FALSE, glm::value_ptr(monkey_world));
		glUniformMatrix4fv(uGeoView, 1, GL_FALSE, glm::value_ptr(cam.get_matrix()));
		glUniformMatrix4fv(uGeoProjection, 1, GL_FALSE, glm::value_ptr(projection));

		// Bind gBuffer in order to write to it
		glBindFramebuffer(GL_FRAMEBUFFER, reninfo.gBuffer);

		// Clear the gBuffer
		glClearColor(0, 0, 0, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use normal depth function
		glDepthFunc(GL_LESS);

		// Bind monkey vertex data
		glBindVertexArray(Monkey_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, Monkey_VBO);

		// Draw elements on the gBuffer
		glDrawArrays(GL_TRIANGLES, 0, file.objects[0].vertices.size());

		// Bind world vertex data
		glBindVertexArray(World_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, World_VBO);

		glUniformMatrix4fv(uGeoWorld, 1, GL_FALSE, glm::value_ptr(world_world));

		glDrawArrays(GL_TRIANGLES, 0, worldfile.objects[0].vertices.size());
		
		// Unbind arrays
		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		// Unbind framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		///////////////////
		// Lighting Pass //
		///////////////////

		// Blit depth pass to current depth
		glBindFramebuffer(GL_READ_FRAMEBUFFER, reninfo.gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, 0, sdlm.size.width, sdlm.size.height, 0, 0, sdlm.size.width, sdlm.size.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Clear color
		glClearColor(0.2f, 0.3f, 0.4f, 1);
		glClear(GL_COLOR_BUFFER_BIT);

		lightingpass.use();

		// Fire the fragment shader if there is an object in front of the square
		// The square is drawn at the very back
		glDepthFunc(GL_GREATER);

		// Bind the gBuffer
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reninfo.gPosition);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, reninfo.gNormal);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, reninfo.gAlbedoSpec);

		// Upload current view position
		glUniform3fv(uLightViewPos, 1, glm::value_ptr(cam.get_location()));

		// Render a quad
		RenderFullscreenQuad();

		//////////////////////////////////
		// Calculate Per Light Lighting //
		//////////////////////////////////

		lightbound.use();

		glBindVertexArray(Light_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, LightTransform_VBO);
		glBufferData(GL_ARRAY_BUFFER, lightcount * sizeof(glm::mat4), lighteffectworldmatrix.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, LightPosition_VBO);
		glBufferData(GL_ARRAY_BUFFER, lightcount * sizeof(glm::vec3), lightposition.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, LightColor_VBO);
		glBufferData(GL_ARRAY_BUFFER, lightcount * sizeof(glm::vec3), lightcolor.data(), GL_STATIC_DRAW);

		glUniformMatrix4fv(uLightBoundPerspective, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(uLightBoundView, 1, GL_FALSE, glm::value_ptr(cam.get_matrix()));
		glUniform3fv(uLightBoundViewPos, 1, glm::value_ptr(cam.get_location()));
		glUniform2f(uLightBoundResolution, sdlm.size.width, sdlm.size.height);

		glDepthFunc(GL_LESS);
		glDepthMask(GL_FALSE);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);

		glDisableVertexAttribArray(0);
		glEnableVertexAttribArray(7);

		glDrawArraysInstanced(GL_TRIANGLES, 0, circlefile.objects[0].vertices.size(), lightcount);

		glDisableVertexAttribArray(7);
		glEnableVertexAttribArray(0);

		glDisable(GL_BLEND);
		glDepthMask(GL_TRUE);

		////////////////
		// Light Pass //
		////////////////

		drawlights.use();

		// Blit depth pass to current depth
		glBindFramebuffer(GL_READ_FRAMEBUFFER, reninfo.gBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

		glBlitFramebuffer(0, 0, sdlm.size.width, sdlm.size.height, 0, 0, sdlm.size.width, sdlm.size.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDepthFunc(GL_LESS);

		glBindVertexArray(Light_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, LightTransform_VBO);
		glBufferData(GL_ARRAY_BUFFER, lightcount * sizeof(glm::mat4), lightworldmatrix.data(), GL_DYNAMIC_DRAW);

		glUniformMatrix4fv(uDrawLightsView, 1, GL_FALSE, glm::value_ptr(cam.get_matrix()));
		glUniformMatrix4fv(uDrawLightsPerspective, 1, GL_FALSE, glm::value_ptr(projection));

		glDrawArraysInstanced(GL_TRIANGLES, 0, squarefile.objects[0].vertices.size(), lightcount);

		glBindVertexArray(0);

		// Swap buffers
		SDL_GL_SwapWindow(sdlm.mainWindow);
	}

	return 0;
}

// RenderQuad() Renders a 1x1 quad in NDC, best used for framebuffer color targets
// and post-processing effects.
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderFullscreenQuad()
{
    if (quadVAO == 0)
    {
        constexpr GLfloat quadVertices[] = {
            // Positions            // Texture Coords
            -1.0f,   1.0f,  1.0f,   0.0f,  1.0f,
            -1.0f,  -1.0f,  1.0f,   0.0f,  0.0f,
             1.0f,   1.0f,  1.0f,   1.0f,  1.0f,
             1.0f,  -1.0f,  1.0f,   1.0f,  0.0f,
        };
        // Setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void PrepareGbuffer(size_t x, size_t y, RenderInfo& data) {
	glGenFramebuffers(1, &data.gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, data.gBuffer);

	// - Position color buffer
	glGenTextures(1, &data.gPosition);
	glBindTexture(GL_TEXTURE_2D, data.gPosition);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, data.gPosition, 0);
  
	// - Normal color buffer
	glGenTextures(1, &data.gNormal);
	glBindTexture(GL_TEXTURE_2D, data.gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, x, y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, data.gNormal, 0);
  
	// - Color + Specular color buffer
	glGenTextures(1, &data.gAlbedoSpec);
	glBindTexture(GL_TEXTURE_2D, data.gAlbedoSpec);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, data.gAlbedoSpec, 0);

	// - Depth buffer
	glGenTextures(1, &data.gDepth);
	glBindTexture(GL_TEXTURE_2D, data.gDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, x, y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, data.gDepth, 0);
  
	// - Tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	constexpr GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, attachments);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DeleteGbuffer(RenderInfo & data) {
	glBindFramebuffer(GL_FRAMEBUFFER, data.gBuffer);
	glDeleteTextures(1, &data.gPosition);
	glDeleteTextures(1, &data.gNormal);
	glDeleteTextures(1, &data.gAlbedoSpec);
	glDeleteTextures(1, &data.gDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &data.gBuffer);
}

glm::mat4 Resize(SDL_Manager & sdlm, RenderInfo & data) {
	sdlm.refresh_size();
	DeleteGbuffer(data);
	PrepareGbuffer(sdlm.size.width, sdlm.size.height, data);
	return glm::perspective(glm::radians(60.0f), sdlm.size.ratio, 0.1f, 1000.0f);
}
