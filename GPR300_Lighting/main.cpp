#include "../structs.h"

void processInput(GLFWwindow* window);
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void mousePosCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

float lastFrameTime;
float deltaTime;

int SCREEN_WIDTH = 1080;
int SCREEN_HEIGHT = 720;

double prevMouseX;
double prevMouseY;
bool firstMouseInput = false;

/* Button to lock / unlock mouse
* 1 = right, 2 = middle
* Mouse will start locked. Unlock it to use UI
* */
const int MOUSE_TOGGLE_BUTTON = 1;
const float MOUSE_SENSITIVITY = 0.1f;
const float CAMERA_MOVE_SPEED = 5.0f;
const float CAMERA_ZOOM_SPEED = 3.0f;

Camera camera((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT);

glm::vec3 bgColor = glm::vec3(0);
glm::vec3 lightColor = glm::vec3(1.0f);
glm::vec3 lightPosition = glm::vec3(0.0f, 3.0f, 0.0f);

bool wireFrame = false;
const int MAX_LIGHTS = 3;

Directional dirLight;
Point pointLight[MAX_LIGHTS];
Spot spotLight;
Material material;

int main() {
	if (!glfwInit()) {
		printf("glfw failed to init");
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Lighting", 0, 0);
	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		printf("glew failed to init");
		return 1;
	}

	glfwSetFramebufferSizeCallback(window, resizeFrameBufferCallback);
	glfwSetKeyCallback(window, keyboardCallback);
	glfwSetScrollCallback(window, mouseScrollCallback);
	glfwSetCursorPosCallback(window, mousePosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	//Hide cursor
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Setup UI Platform/Renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	//Dark UI theme.
	ImGui::StyleColorsDark();

	//Used to draw shapes. This is the shader you will be completing.
	Shader litShader("shaders/defaultLit.vert", "shaders/defaultLit.frag");

	//Used to draw light sphere
	Shader unlitShader("shaders/defaultLit.vert", "shaders/unlit.frag");

	ew::MeshData cubeMeshData;
	ew::createCube(1.0f, 1.0f, 1.0f, cubeMeshData);
	ew::MeshData sphereMeshData;
	ew::createSphere(0.5f, 64, sphereMeshData);
	ew::MeshData cylinderMeshData;
	ew::createCylinder(1.0f, 0.5f, 64, cylinderMeshData);
	ew::MeshData planeMeshData;
	ew::createPlane(1.0f, 1.0f, planeMeshData);

	ew::Mesh cubeMesh(&cubeMeshData);
	ew::Mesh sphereMesh(&sphereMeshData);
	ew::Mesh planeMesh(&planeMeshData);
	ew::Mesh cylinderMesh(&cylinderMeshData);

	//Enable back face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	//Enable blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//Initialize shape transforms
	ew::Transform cubeTransform;
	ew::Transform sphereTransform;
	ew::Transform planeTransform;
	ew::Transform cylinderTransform;
	ew::Transform lightTransform;

	cubeTransform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
	sphereTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);

	planeTransform.position = glm::vec3(0.0f, -1.0f, 0.0f);
	planeTransform.scale = glm::vec3(10.0f);

	cylinderTransform.position = glm::vec3(2.0f, 0.0f, 0.0f);

	lightTransform.scale = glm::vec3(0.5f);
	lightTransform.position = glm::vec3(0.0f, 5.0f, 0.0f);

	pointLight[0].transform.position = glm::vec3(-1, 2, -1);
	pointLight[0].color = glm::vec3(1, 0, 0);
	pointLight[1].transform.position = glm::vec3(-2, 2, 1);
	pointLight[1].color = glm::vec3(0, 1, 0);
	pointLight[2].transform.position = glm::vec3(1, 2, -1);
	pointLight[2].color = glm::vec3(0, 0, 1);

	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		glClearColor(bgColor.r,bgColor.g,bgColor.b, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		float time = (float)glfwGetTime();
		deltaTime = time - lastFrameTime;
		lastFrameTime = time;

		cubeTransform.rotation.x += deltaTime;

		//Draw
		litShader.use();
		litShader.setMat4("_Projection", camera.getProjectionMatrix());
		litShader.setMat4("_View", camera.getViewMatrix());
		litShader.setVec3("camera.cameraPos", camera.getPosition());

		litShader.setVec3("material.color", material.color);
		litShader.setFloat("material.ambientK", material.ambientK);
		litShader.setFloat("material.diffuseK", material.diffuseK);
		litShader.setFloat("material.specularK", material.specularK);
		litShader.setFloat("material.shininess", material.shininess);

		litShader.setVec3("dirLight.direction", dirLight.direction);
		litShader.setFloat("dirLight.intensity", dirLight.intensity);
		litShader.setVec3("dirLight.color", dirLight.color);

		//Set some lighting uniforms - "+std::to_string(0)+"
		for (size_t i = 0; i < MAX_LIGHTS; i++) {
			litShader.setVec3("pointLights[" + std::to_string(i) + "].position", pointLight[i].transform.position);
			litShader.setFloat("pointLights[" + std::to_string(i) + "].intensity", pointLight[i].intensity);
			litShader.setVec3("pointLights[" + std::to_string(i) + "].color", pointLight[i].color);
			litShader.setFloat("pointLights[" + std::to_string(i) + "].linear", pointLight[i].linear);
			litShader.setFloat("pointLights[" + std::to_string(i) + "].quadratic", pointLight[i].quadratic);
			litShader.setFloat("pointLights[" + std::to_string(i) + "].range", pointLight[i].range);
		}

		litShader.setVec3("spotLight.color", spotLight.color);
		litShader.setVec3("spotLight.position", spotLight.transform.position);
		litShader.setVec3("spotLight.direction", spotLight.direction);
		litShader.setFloat("spotLight.intensity", spotLight.intensity);
		litShader.setFloat("spotLight.min_angle", glm::cos(glm::radians(spotLight.min_angle)));
		litShader.setFloat("spotLight.max_angle", glm::cos(glm::radians(spotLight.max_angle)));
		litShader.setFloat("spotLight.angularFallOff", spotLight.angularFallOff);
		litShader.setFloat("spotLight.range", spotLight.range);
		
		//Draw cube
		litShader.setMat4("_Model", cubeTransform.getModelMatrix());
		cubeMesh.draw();

		//Draw sphere
		litShader.setMat4("_Model", sphereTransform.getModelMatrix());
		sphereMesh.draw();

		//Draw cylinder
		litShader.setMat4("_Model", cylinderTransform.getModelMatrix());
		cylinderMesh.draw();

		//Draw plane
		litShader.setMat4("_Model", planeTransform.getModelMatrix());
		planeMesh.draw();

		//Draw light as a small sphere using unlit shader, ironically.
		unlitShader.use();
		unlitShader.setMat4("_Projection", camera.getProjectionMatrix());
		unlitShader.setMat4("_View", camera.getViewMatrix());

		for (size_t i = 0; i < MAX_LIGHTS; i++) {
			unlitShader.setMat4("_Model", pointLight[i].transform.getModelMatrix());
			unlitShader.setVec3("_Color", pointLight[i].color);
			sphereMesh.draw();
		}

		//Draw UI
		ImGui::Begin("Material");
		ImGui::ColorEdit3("Material Color", &material.color.r);
		ImGui::SliderFloat("Material Ambient K", &material.ambientK, 0.0f, 1.0f);
		ImGui::SliderFloat("Material Diffuse K", &material.diffuseK, 0.0f, 1.0f);
		ImGui::SliderFloat("Material Specular K", &material.specularK, 0.0f, 1.0f);
		ImGui::SliderFloat("Material Shininess", &material.shininess, 1.0f, 512.0f);
		ImGui::End();

		ImGui::Begin("Directional Light");
		ImGui::DragFloat3("Direction", &dirLight.direction.x);
		ImGui::SliderFloat("Intensity", &dirLight.intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("Color", &dirLight.color.r);
		ImGui::End();

		for (size_t i = 0; i < MAX_LIGHTS; i++) {
			ImGui::Begin(("Point Light " + std::to_string(i + 1)).c_str());
			ImGui::SliderFloat("Intensity", &pointLight[i].intensity, 0.0f, 1.0f);
			ImGui::DragFloat3("Position", &pointLight[i].transform.position.x);
			ImGui::ColorEdit3("Color", &pointLight[i].color.r);
			ImGui::SliderFloat("Linear", &pointLight[i].linear, 0.0014f, 0.7f);
			ImGui::SliderFloat("Quadratic", &pointLight[i].quadratic, 0.000007f, 1.8f);
			ImGui::SliderFloat("Range", &pointLight[i].range, 1.0f, 20.0f);
			ImGui::End();
		}

		ImGui::Begin("Spot Light");
		ImGui::ColorEdit3("Color", &spotLight.color.r);
		ImGui::DragFloat3("Position", &spotLight.transform.position.x);
		ImGui::DragFloat3("Direction", &spotLight.direction.x);
		ImGui::SliderFloat("Intensity", &spotLight.intensity, 0.0f, 1.0f);
		ImGui::SliderFloat("Minimum Angle", &spotLight.min_angle, 0.0f, 90.0f);
		ImGui::SliderFloat("Maximum Angle", &spotLight.max_angle, 0.0f, 90.0f);
		ImGui::SliderFloat("Angular Fall Off", &spotLight.angularFallOff, 1.0f, 4.0f);
		ImGui::SliderFloat("Range", &spotLight.range, 1.0f, 20.0f);
		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwPollEvents();

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}
//Author: Eric Winebrenner
void resizeFrameBufferCallback(GLFWwindow* window, int width, int height)
{
	SCREEN_WIDTH = width;
	SCREEN_HEIGHT = height;
	camera.setAspectRatio((float)SCREEN_WIDTH / SCREEN_HEIGHT);
	glViewport(0, 0, width, height);
}
//Author: Eric Winebrenner
void keyboardCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
	if (keycode == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//Reset camera
	if (keycode == GLFW_KEY_R && action == GLFW_PRESS) {
		camera.setPosition(glm::vec3(0, 0, 5));
		camera.setYaw(-90.0f);
		camera.setPitch(0.0f);
		firstMouseInput = false;
	}
	if (keycode == GLFW_KEY_1 && action == GLFW_PRESS) {
		wireFrame = !wireFrame;
		glPolygonMode(GL_FRONT_AND_BACK, wireFrame ? GL_LINE : GL_FILL);
	}
}
//Author: Eric Winebrenner
void mouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (abs(yoffset) > 0) {
		float fov = camera.getFov() - (float)yoffset * CAMERA_ZOOM_SPEED;
		camera.setFov(fov);
	}
}
//Author: Eric Winebrenner
void mousePosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED) {
		return;
	}
	if (!firstMouseInput) {
		prevMouseX = xpos;
		prevMouseY = ypos;
		firstMouseInput = true;
	}
	float yaw = camera.getYaw() + (float)(xpos - prevMouseX) * MOUSE_SENSITIVITY;
	camera.setYaw(yaw);
	float pitch = camera.getPitch() - (float)(ypos - prevMouseY) * MOUSE_SENSITIVITY;
	pitch = glm::clamp(pitch, -89.9f, 89.9f);
	camera.setPitch(pitch);
	prevMouseX = xpos;
	prevMouseY = ypos;
}
//Author: Eric Winebrenner
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	//Toggle cursor lock
	if (button == MOUSE_TOGGLE_BUTTON && action == GLFW_PRESS) {
		int inputMode = glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;
		glfwSetInputMode(window, GLFW_CURSOR, inputMode);
		glfwGetCursorPos(window, &prevMouseX, &prevMouseY);
	}
}

//Author: Eric Winebrenner
//Returns -1, 0, or 1 depending on keys held
float getAxis(GLFWwindow* window, int positiveKey, int negativeKey) {
	float axis = 0.0f;
	if (glfwGetKey(window, positiveKey)) {
		axis++;
	}
	if (glfwGetKey(window, negativeKey)) {
		axis--;
	}
	return axis;
}

//Author: Eric Winebrenner
//Get input every frame
void processInput(GLFWwindow* window) {

	float moveAmnt = CAMERA_MOVE_SPEED * deltaTime;

	//Get camera vectors
	glm::vec3 forward = camera.getForward();
	glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0,1,0)));
	glm::vec3 up = glm::normalize(glm::cross(forward, right));

	glm::vec3 position = camera.getPosition();
	position += forward * getAxis(window, GLFW_KEY_W, GLFW_KEY_S) * moveAmnt;
	position += right * getAxis(window, GLFW_KEY_D, GLFW_KEY_A) * moveAmnt;
	position += up * getAxis(window, GLFW_KEY_Q, GLFW_KEY_E) * moveAmnt;
	camera.setPosition(position);
}
