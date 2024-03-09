#include "light.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"

unsigned int Light::dirLightNum = 0;
unsigned int Light::pointLightNum = 0;
unsigned int Light::spotLightNum = 0;
unsigned int Light::UBO = 0;

static const unsigned int zero = 0;

void Light::init() {
	// create UBO
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(unsigned int) * 3 +MAX_NUM_LIGHTS * (dirLightSize + pointLightSize + spotLightSize), NULL, GL_STATIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, UBO);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(unsigned int), &zero);
	glBufferSubData(GL_UNIFORM_BUFFER, 1 * sizeof(unsigned int), sizeof(unsigned int), &zero);
	glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(unsigned int), sizeof(unsigned int), &zero);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}