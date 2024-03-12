#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

extern unsigned int WINDOW_WIDTH;
extern unsigned int WINDOW_HEIGHT;

enum Camera_Move_Direction {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT
};

struct Quaternion {
	float x, y, z, w;

	Quaternion() = default;

	Quaternion(float xp, float yp, float zp, float wp) : x(xp), y(yp), z(zp), w(wp) {}

	Quaternion operator* (Quaternion const& a) {
		Quaternion res = Quaternion(
			this->w * a.x + this->x * a.w + this->y * a.z - this->z * a.y,
			this->w * a.y - this->x * a.z + this->y * a.w + this->z * a.x,
			this->w * a.z + this->x * a.y - this->y * a.x + this->z * a.w,
			this->w * a.w - this->x * a.x - this->y * a.y - this->z * a.z);

		return res;
	}

	static Quaternion conjugate(Quaternion a) {
		return Quaternion(-a.x, -a.y, -a.z, a.w);
	}
};

class Camera {
public:
	glm::vec3 pos;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	
	float zoom;
	float sensitivity;
	float mouseSpeed;

	Camera(glm::vec3 newPos = glm::vec3(0.0f), glm::vec3 newFront = glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3 newWorldUp = glm::vec3(0.0f, 1.0f, 0.0f)) {
		zoom = 45.0f;
		sensitivity = 200.0f;
		mouseSpeed = 2.5f;

		pos = newPos;
		front = newFront;
		worldUp = newWorldUp;
		
		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));
	}

	void init() {
		setupUBO();
		updateUBOProj();
		updateUBOView();
	}

	void updateZoom(float val) {
		zoom -= val;

		if (zoom < 1.0f)
			zoom = 1.0f;
		else if (zoom > 45.0f)
			zoom = 45.0f;

		updateUBOProj();
	}

	void handleCameraMovement(Camera_Move_Direction direc, bool speedUp, float deltaTime) {
		float cameraSpeed = mouseSpeed * deltaTime * (speedUp ? 2.0f : 1.0f);

		if (direc == FORWARD)
			this->pos += cameraSpeed * front;
		else if (direc == BACKWARD)
			this->pos -= cameraSpeed * front;
		else if (direc == LEFT)
			this->pos -= cameraSpeed * right;
		else if (direc == RIGHT)
			this->pos += cameraSpeed * right;

		updateUBOView();
	}

	void handleCameraRotation(float xOffset, float yOffset) {
		xOffset = xOffset / sensitivity;
		yOffset = yOffset / sensitivity;

		glm::vec3 yAxis = glm::normalize(glm::cross(front - pos, up));

		rotate(xOffset, glm::vec3(0.0f, 1.0f, 0.0f));
		rotate(yOffset, yAxis);

		right = glm::normalize(glm::cross(front, worldUp));
		up = glm::normalize(glm::cross(right, front));

		updateUBOView();
	}

	inline glm::mat4 getViewMatrix() {
		return glm::lookAt(pos, front, up);
	}

	inline glm::mat4 getProjMatrix() {
		return glm::perspective(glm::radians(zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
	}

private:
	inline void rotate(double angle, glm::vec3 axis) {
		Quaternion R, quatFront, result;

		quatFront.x = front.x;
		quatFront.y = front.y;
		quatFront.z = front.z;
		quatFront.w = 0;
		
		R.x = axis.x * sin((float)angle / 2.0f);
		R.y = axis.y * sin((float)angle / 2.0f);
		R.z = axis.z * sin((float)angle / 2.0f);
		R.w = cos((float)angle / 2.0f);

		result = R * quatFront * Quaternion::conjugate(R);

		front.x = result.x;
		front.y = result.y;
		front.z = result.z;
	}

	inline void setupUBO() {
		glGenBuffers(1, &UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2 + sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	inline void updateUBOProj() {
		glm::mat4 projMatrix = glm::perspective(glm::radians(zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projMatrix));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	inline void updateUBOView() {
		glm::mat4 viewMatrix = glm::lookAt(pos, pos + front, up);

		glBindBuffer(GL_UNIFORM_BUFFER, UBO);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMatrix));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, sizeof(glm::vec3), glm::value_ptr(pos));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	unsigned int UBO;
};