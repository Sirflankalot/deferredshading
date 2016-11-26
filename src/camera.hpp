#pragma once

#include <GL/gl.h>
#include <glm/glm.hpp>

class Camera {
  public:
	Camera() : position(glm::vec3(0.0f, 0.0f, 0.0f)) {
		recalculate_view_matrix();
	};
	Camera(const glm::vec3& pos) : position(pos) {
		recalculate_view_matrix();
	};

	void move(const glm::vec3& offset, float speed_mult = 1.0f);
	void rotate(const float pitch_offset, const float yaw_offset, float speed_mult = 1.0f);
	void set_location(const glm::vec3& loc);
	void set_rotation(const float new_pitch, const float new_yaw);

	void use(GLuint uniform);

	const glm::vec3& get_location();
	const glm::vec3& get_front_vec();
	const glm::vec3& get_up_vec();
	const glm::vec3& get_right_vec();
	const glm::mat4& get_matrix();

  private:
	void recalculate_view_matrix();

	float yaw   = 0.0f;
	float pitch = 0.0f;

	float speed = 1.0f;
	glm::mat4 view;

	glm::vec3 position;
	glm::vec3 direction;
	glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up    = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right = glm::normalize(glm::cross(front, up));
};

#include "camera_impl.hpp"

