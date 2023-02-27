#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stdio.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "EW/Shader.h"
#include "EW/EwMath.h"
#include "EW/Camera.h"
#include "EW/Mesh.h"
#include "EW/Transform.h"
#include "EW/ShapeGen.h"

struct Material {
	glm::vec3 color = glm::vec3(0.9f, 0.9f, 0.9f);
	float ambientK = 0.25;
	float diffuseK = 0.25;
	float specularK = 0.25;
	float shininess = 100;
};

struct Directional {
    glm::vec3 color = glm::vec3(1);
    glm::vec3 direction = glm::vec3(0, -1, 0);
    float intensity = 1;
};

struct Point {
    glm::vec3 color = glm::vec3(1);
    ew::Transform transform;
    float intensity = 1;
    //lin_attenuation
    float linear = 0.7;
    float quadratic = 1.8;
    float range = 5.0f;
};

struct Spot {
    glm::vec3 color = glm::vec3(1);
    ew::Transform transform;
    glm::vec3 direction = glm::vec3(0, -1, 3);
    float intensity = 1;
    //lin_attenuation
    float min_angle = 30;
    float max_angle = 60;
    float angularFallOff = 2;
    float range = 1.0f;
};