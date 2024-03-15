#include "gui.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "entity.hpp"
#include "ImGuiFileDialog.h"
#include "renderer.hpp"
#include "camera.hpp"

void openFileDialog();

extern Renderer rs;
extern Camera camera;

static bool showDialog = false;
static bool objList = false;
static bool matList = false;
static bool lightList = false;
static bool addShader = false;
static bool addSkybox = false;
static string objFilePath = "";
static string vertexShaderPath = "";
static string fragmentShaderPath = "";
static string geometryShaderPath = "";
static string fileType = "";
static string diffuseTexturePath = "";
static string specularTexturePath = "";
static string normalTexturePath = "";
static string heightTexturePath = "";
static string rightFacePath = "";
static string leftFacePath = "";
static string topFacePath = "";
static string bottomFacePath = "";
static string frontFacePath = "";
static string backFacePath = "";
static string* curFilePath = &objFilePath;

struct ColorOption {
	std::string name;
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float shininess;
	bool isSelected = false;
};

std::vector<ColorOption> colorOptions = {
	{"emerald", {0.0215f, 0.1745f, 0.0215f}, {0.07568f, 0.61424f, 0.07568f}, {0.633f, 0.727811f, 0.633f}, 0.6f},
	{"jade", {0.135f, 0.2225f, 0.1575f}, {0.54f, 0.89f, 0.63f}, {0.316228f, 0.316228f, 0.316228f}, 0.1f},
	{"obsidian", {0.05375f, 0.05f, 0.06625f}, {0.18275f, 0.17f, 0.22525f}, {0.332741f, 0.328634f, 0.346435f}, 0.3f},
	{"pearl", {0.25f, 0.20725f, 0.20725f}, {1.0f, 0.829f, 0.829f}, {0.296648f, 0.296648f, 0.296648f}, 0.088f},
	{"ruby", {0.1745f, 0.01175f, 0.01175f}, {0.61424f, 0.04136f, 0.04136f}, {0.727811f, 0.626959f, 0.626959f}, 0.6f},
	{"turquoise", {0.1f, 0.18725f, 0.1745f}, {0.396f, 0.74151f, 0.69102f}, {0.297254f, 0.30829f, 0.306678f}, 0.1f},
	{"brass", {0.329412f, 0.223529f, 0.027451f}, {0.780392f, 0.568627f, 0.113725f}, {0.992157f, 0.941176f, 0.807843f}, 0.21794872f},
	{"bronze", {0.2125f, 0.1275f, 0.054f}, {0.714f, 0.4284f, 0.18144f}, {0.393548f, 0.271906f, 0.166721f}, 0.2f},
	{"chrome", {0.25f, 0.25f, 0.25f}, {0.4f, 0.4f, 0.4f}, {0.774597f, 0.774597f, 0.774597f}, 0.6f},
	{"copper", {0.19125f, 0.0735f, 0.0225f}, {0.7038f, 0.27048f, 0.0828f}, {0.256777f, 0.137622f, 0.086014f}, 0.1f},
	{"gold", {0.24725f, 0.1995f, 0.0745f}, {0.75164f, 0.60648f, 0.22648f}, {0.628281f, 0.555802f, 0.366065f}, 0.4f},
	{"silver", {0.19225f, 0.19225f, 0.19225f}, {0.50754f, 0.50754f, 0.50754f}, {0.508273f, 0.508273f, 0.508273f}, 0.4f},
	{"black plastic", {0.0f, 0.0f, 0.0f}, {0.01f, 0.01f, 0.01f}, {0.50f, 0.50f, 0.50f}, .25f},
	{"cyan plastic", {0.0f, 0.1f, 0.06f}, {0.0f, 0.50980392f, 0.50980392f}, {0.50196078f, 0.50196078f, 0.50196078f}, .25f},
	{"green plastic", {0.0f, 0.0f, 0.0f}, {0.1f, 0.35f, 0.1f}, {0.45f, 0.55f, 0.45f}, .25f},
	{"red plastic", {0.0f, 0.0f, 0.0f}, {0.5f, 0.0f, 0.0f}, {0.7f, 0.6f, 0.6f}, .25f},
	{"white plastic", {0.0f, 0.0f, 0.0f}, {0.55f, 0.55f, 0.55f}, {0.70f, 0.70f, 0.70f}, .25f},
	{"yellow plastic", {0.0f, 0.0f, 0.0f}, {0.5f, 0.5f, 0.0f}, {0.60f, 0.60f, 0.50f}, .25f},
	{"black rubber", {0.02f, 0.02f, 0.02f}, {0.01f, 0.01f, 0.01f}, {0.4f, 0.4f, 0.4f}, .078125f},
	{"cyan rubber", {0.0f, 0.05f, 0.05f}, {0.4f, 0.5f, 0.5f}, {0.04f, 0.7f, 0.7f}, .078125f},
	{"green rubber", {0.0f, 0.05f, 0.0f}, {0.4f, 0.5f, 0.4f}, {0.04f, 0.7f, 0.04f}, .078125f},
	{"red rubber", {0.05f, 0.0f, 0.0f}, {0.5f, 0.4f, 0.4f}, {0.7f, 0.04f, 0.04f}, .078125f},
	{"white rubber", {0.05f, 0.05f, 0.05f}, {0.5f, 0.5f, 0.5f}, {0.7f, 0.7f, 0.7f}, .078125f},
	{"yellow rubber", {0.05f, 0.05f, 0.0f}, {0.5f, 0.5f, 0.4f}, {0.7f, 0.7f, 0.04f}, .078125f}
};

// Main Menu
void showMainMenuBar() {
	if (ImGui::Begin("Current Position")) {
		ImGui::Text("X: %f", camera.pos.x);
		ImGui::Text("Y: %f", camera.pos.y);
		ImGui::Text("Z: %f", camera.pos.z);
		ImGui::End();
	}
	if (showDialog) {
		openFileDialog();
		return;
	}
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Load Shader..")) {
				addShader = true;
			}
			if (ImGui::MenuItem("Load Cubemap..")) {
				addSkybox = true;
			}
			// quit
			if (ImGui::MenuItem("Quit")) {
				// quit
				ImGui_ImplOpenGL3_Shutdown();
				ImGui_ImplGlfw_Shutdown();
				ImGui::DestroyContext();
				glfwTerminate();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("List")) {
			if (ImGui::MenuItem("Objects", "", &objList)) {}
			if (ImGui::MenuItem("Materials", "", &matList)) {}
			if (ImGui::MenuItem("Lights", "", &lightList)) {}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (addShader) {
		showAddShader();
	}

	if (addSkybox) {
		showAddSkybox();
	}

	if (objList) {
		showObjList();
	}

	if (matList) {
		showMaterialList();
	}

	if (lightList) {
		showLightList();
	}
}

// show objects list
void showObjList() {
	if (ImGui::Begin("Objects")) {
		vector<unsigned int> toDelete;
		for (auto const& [eID, e] : rs.entities) {
			ImGui::Checkbox(e->name.c_str(), &e->render);
			ImGui::SameLine();

			// properties button
			if (ImGui::Button(("Properties##" + std::to_string(e->ID)).c_str(), ImVec2(100, 20))) {
				e->showProperties = true;
			}

			if (e->showProperties) {
				showObjectProperties(*e);
			}

			ImGui::SameLine();
			// delete button
			if (ImGui::Button(("Delete##" + std::to_string(e->ID)).c_str(), ImVec2(100, 20))) {
				toDelete.push_back(eID);
			}
		}

		if (ImGui::Button("Add Object")) {
			ImGui::OpenPopup("Add Object");
		}
		// popup menu for adding objects
		if (ImGui::BeginPopup("Add Object")) {
			ImGui::SeparatorText("Obejct Type");
			if (ImGui::MenuItem("Cube")) {
				rs.addEntity(CUBE);
			}
			if (ImGui::MenuItem("Sphere")) {
				rs.addEntity(SPHERE);
			}
			if (ImGui::MenuItem("Model")) {
				showDialog = true;
				curFilePath = &objFilePath;
				fileType = ".obj,.fbx";
			}
			ImGui::EndPopup();
		}
		
		// load model if the file path is not empty
		if (!objFilePath.empty()) {
			rs.addEntity(OTHER, objFilePath);
			objFilePath.clear();
		}

		for (auto eID : toDelete) {
			rs.removeEntity(eID);
		}
		ImGui::End();
	}
	else {
        ImGui::End();
        return;
	}
}

// show materials list
void showMaterialList() {
	if (ImGui::Begin("Materials")) {
		vector<unsigned int> toDelete;
		for (auto const& [mID, m] : rs.materials) {
			ImGui::Text(m->name.c_str());
			ImGui::SameLine();

			if (ImGui::Button(("Properties##" + m->name).c_str())) {
				m->showProperties = true;
			}
			ImGui::SameLine();

			if (ImGui::Button(("Delete##" + m->name).c_str())) {
				toDelete.push_back(mID);
			}

			if (m->showProperties) {
				showMaterialProperties(*m);
			}
		}

		if (ImGui::Button("Add Material")) {
			ImGui::OpenPopup("MaterialTypePopup");
		}
		static int selectedMaterialType = -1;
		if (ImGui::BeginPopup("MaterialTypePopup", NULL)) {
			ImGui::Text("Select Material Type:");
			ImGui::Separator();

			// Option for "Single Color"
			if (ImGui::Selectable("Single Color", selectedMaterialType == 0)) {
				rs.addMaterial(true);
				ImGui::CloseCurrentPopup(); // Optionally close the popup after selection
			}

			// Option for "Textures"
			if (ImGui::Selectable("Textures", selectedMaterialType == 1)) {
				rs.addMaterial(false);
				ImGui::CloseCurrentPopup(); // Optionally close the popup after selection
			}

			ImGui::EndPopup();
		}

		for (auto mID : toDelete) {
			rs.removeMaterial(mID);
		}

		ImGui::End();
	}
	else {
		ImGui::End();
		return;
	}
}

// show lights lit
void showLightList() {
	if (lightList && ImGui::Begin("Lights")) {
		vector<unsigned int> toDelete;
		for (auto const& [lID, l] : rs.lights) {
			ImGui::Checkbox(l->name.c_str(), &l->visible);
			ImGui::SameLine();

			if (ImGui::Button(("Properties##" + l->name).c_str())) {
				l->showProperties = true;
			}
			ImGui::SameLine();

			if (ImGui::Button(("Delete##" + l->name).c_str())) {
				toDelete.push_back(lID);
			}

			if (l->showProperties) {
				showLightProperties(*l);
			}
		}

		if (ImGui::Button("Add Light")) {
			ImGui::OpenPopup("Add Light");
		}
		if (ImGui::BeginPopup("Add Light")) {
			if (ImGui::MenuItem("Directional Light")) {
				rs.addLight(DIRECTIONAL);
			}
			if (ImGui::MenuItem("Point Light")) {
				rs.addLight(POINT);
			}
			if (ImGui::MenuItem("Spot Light")) {
				rs.addLight(SPOT);
			}
			ImGui::EndPopup();
		}

		for (auto lID : toDelete) {
			rs.removeLight(lID);
		}

		ImGui::End();
	}
	else {
		ImGui::End();
		return;
	}
}

// show light properties
void showLightProperties(Light& l) {
	if (ImGui::Begin(l.name.c_str())) {
		ImGui::SeparatorText("Color");
		static float diffuseColor[3];
		diffuseColor[0] = l.lightComponent.ambient.x;
		diffuseColor[1] = l.lightComponent.ambient.y;
		diffuseColor[2] = l.lightComponent.ambient.z;
		if (ImGui::ColorEdit3("Ambient and Difuse", diffuseColor)) {
			l.lightComponent.ambient = glm::vec3(diffuseColor[0], diffuseColor[1], diffuseColor[2]);
			l.lightComponent.diffuse = glm::vec3(diffuseColor[0], diffuseColor[1], diffuseColor[2]);
		}
		static float specularColor[3];
		specularColor[0] = l.lightComponent.specular.x;
		specularColor[1] = l.lightComponent.specular.y;
		specularColor[2] = l.lightComponent.specular.z;
		if (ImGui::ColorEdit3("Specular", specularColor)) {
			l.lightComponent.specular = glm::vec3(specularColor[0], specularColor[1], specularColor[2]);
		}

		if (l.type == POINT || l.type == SPOT) {
			ImGui::SeparatorText("Position");
			ImGui::DragFloat("X##Position", &l.position.x, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Y##Position", &l.position.y, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Z##Position", &l.position.z, 0.1f, -100.0f, 100.0f);
		}

		if (l.type == DIRECTIONAL) {
			DirectionalLight &dl = (DirectionalLight&)l;
			ImGui::SeparatorText("Direction");
			ImGui::DragFloat("X##Direction", &dl.direction.x, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Y##Direction", &dl.direction.y, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Z##Direction", &dl.direction.z, 0.1f, -100.0f, 100.0f);
		}
		else if (l.type == POINT) {
			PointLight &pl = (PointLight&)l;
			ImGui::SeparatorText("Attenuation");
			ImGui::DragFloat("Constant", &pl.attenuation.constant, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Linear", &pl.attenuation.linear, 0.1f, 0.0f, 100.0f);
			ImGui::DragFloat("Quadratic", &pl.attenuation.quadratic, 0.1f, 0.0f, 100.0f);
		} 
		else if (l.type == SPOT) {
			SpotLight &sl = (SpotLight&)l;
			ImGui::SeparatorText("Direction");
			ImGui::DragFloat("X##Direction", &sl.direction.x, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Y##Direction", &sl.direction.y, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Z##Direction", &sl.direction.z, 0.1f, -100.0f, 100.0f);
			ImGui::SeparatorText("Attenuation");
			ImGui::DragFloat("Constant", &sl.attenuation.constant, 0.1f, 0.0f, 5.0f);
			ImGui::DragFloat("Linear", &sl.attenuation.linear, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Quadratic", &sl.attenuation.quadratic, 0.005f, 0.0f, 0.1f);
			ImGui::SeparatorText("Cutoff");
			static float innerCutOff = 12.5f;
			if (ImGui::DragFloat("Inner", &innerCutOff, 1.0f, 10.0f, 40.0f)) {
				sl.cutOff = glm::cos(glm::radians(innerCutOff));
			}
			static float outerCutOff = 17.5f;
			if (ImGui::DragFloat("Outer", &outerCutOff, 1.0f, 0.0f, 50.0f)) {
				sl.outerCutOff = glm::cos(glm::radians(outerCutOff));
			}
		}
		
		ImGui::Spacing();ImGui::Spacing();ImGui::Spacing();ImGui::Spacing();
		if (ImGui::Button("Close")) {
			l.showProperties = false;
		}

		ImGui::End();
	}
	else {
		ImGui::End();
		return;
	}
}

// show material properties	
void showMaterialProperties(Material& m) {
	if (ImGui::Begin(m.name.c_str())) {
		ImGui::SeparatorText("Material Properties");
		ImGui::DragFloat("Shininess", &m.shininess, m.shininess, 0.0f, 100.0f);
		ImGui::Spacing();
		if(m.isColor) {
			ImGui::Text("Color");
			int i = 0;
			for (auto& color : colorOptions) {
				// Display the material name with a checkbox
				if (ImGui::Checkbox(color.name.c_str(), &color.isSelected)) {
					// When selected, update all other materials to not selected
					for (auto& c : colorOptions) {
						c.isSelected = false;
					}
					color.isSelected = true;

					m.ambient = glm::vec3(color.ambient[0], color.ambient[1], color.ambient[2]);
					m.diffuse = glm::vec3(color.diffuse[0], color.diffuse[1], color.diffuse[2]);
					m.specular = glm::vec3(color.specular[0], color.specular[1], color.specular[2]);
					m.shininess = color.shininess * 128.0f;
				}

				// Display a colored rectangle next to the name
				ImGui::SameLine();
				ImVec4 diffuseColor = ImVec4(color.diffuse[0], color.diffuse[1], color.diffuse[2], 1.0f);
				ImGui::ColorButton("##color", diffuseColor, ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoPicker);
			}
		}
		else {
			ImGui::Text("Textures");
			ImGui::DragFloat("Parallax Mapping Height Scale", &m.heightScale, 0.01f, 0.0f, 1.0f);
			ImGui::DragFloat("Parallax Mapping Minimum Layers", &m.minLayers, 1.0f, 1.0f, 16.0f);
			ImGui::DragFloat("Parallax Mapping Maximum Layers", &m.maxLayers, 1.0f, 16.0f, 64.0f);

			// Texture list box
			if (ImGui::BeginListBox("##Textures")) {
				for (int i = 0; i < m.textures.size(); i++) {
					ImGui::PushID(i); // Ensure unique ID for each texture

					// Convert the texture type to a string for display
					std::string typeStr = "";
					switch (m.textures[i].type) {
					case TEXTURE_DIFFUSE: typeStr = "Diffuse"; break;
					case TEXTURE_SPECULAR: typeStr = "Specular"; break;
					case TEXTURE_NORMAL: typeStr = "Normal"; break;
					case TEXTURE_HEIGHT: typeStr = "Height"; break;
					}

					// Display texture path and type
					ImGui::Text("%s - %s", m.textures[i].name.c_str(), typeStr.c_str());
					ImGui::SameLine();

					// Button to delete this texture
					if (ImGui::Button(("Delete##" + m.textures[i].path).c_str())) {
						m.textures.erase(m.textures.begin() + i);
						ImGui::PopID();
						break; // Break to avoid continuing with an invalidated iterator
					}

					ImGui::PopID();
				}
				ImGui::EndListBox();
			}

			// Buttons to add textures of different types
			if (ImGui::Button("Add Diffuse Map")) {
				fileType = ".png,.jpg,.jpeg";
				showDialog = true;
				curFilePath = &diffuseTexturePath;
			}
			ImGui::SameLine();
			if (ImGui::Button("Add Specular Map")) {
				fileType = ".png,.jpg,.jpeg";
				showDialog = true;
				curFilePath = &specularTexturePath;
			}
			ImGui::Spacing();
			if (ImGui::Button("Add Normal Map")) {
				fileType = ".png,.jpg,.jpeg";
				showDialog = true;
				curFilePath = &normalTexturePath;
			}
			ImGui::SameLine();
			if (ImGui::Button("Add Height Map")) {
				fileType = ".png,.jpg,.jpeg";
				showDialog = true;
				curFilePath = &heightTexturePath;
			}

			if (showDialog) {
				showFileDialog();
			}

			if (diffuseTexturePath != "") {
				m.addTexture(TEXTURE_DIFFUSE, diffuseTexturePath);
				diffuseTexturePath = "";
			}
			if (specularTexturePath != "") {
				m.addTexture(TEXTURE_SPECULAR, specularTexturePath);
				specularTexturePath = "";
			}
			if (normalTexturePath != "") {
				m.addTexture(TEXTURE_NORMAL, normalTexturePath);
				normalTexturePath = "";
			}
			if (heightTexturePath != "") {
				m.addTexture(TEXTURE_HEIGHT, heightTexturePath);
				heightTexturePath = "";
			}
		}
		// show shader list
		ImGui::Spacing();
		ImGui::Text("Shader");
		if (ImGui::BeginListBox("##listbox", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
			for (auto const& [sID, s] : rs.shaders) {
				if (ImGui::Selectable(s->name.c_str(), sID == m.shaderID)) {
					m.shaderID = sID;
				}
			}
			ImGui::EndListBox();
		}
		if (ImGui::Button("Close")) {
			m.showProperties = false;
		}
		ImGui::End();
	}
	else {
		ImGui::End();
		return;
	}
}

// show Object properties
void showObjectProperties(Entity& e) {
	vector<Component>& comps = e.components;
	
	if (ImGui::Begin(("Object name: " + e.name).c_str())) {
		ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

		// position
		if (ImGui::CollapsingHeader("Object Position")) {
			ImGui::DragFloat("X##Position", &e.pos.x, 0.1f, -100, 100);
			ImGui::DragFloat("Y##Position", &e.pos.y, 0.1f, -100, 100);
			ImGui::DragFloat("Z##Position", &e.pos.z, 0.1f, -100, 100);
		}
		// scale
		if (ImGui::CollapsingHeader("Object Scale")) {
			ImGui::DragFloat("X##Scale", &e.scale.x, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Y##Scale", &e.scale.y, 0.1f, 0.1f, 10.0f);
			ImGui::DragFloat("Z##Scale", &e.scale.z, 0.1f, 0.1f, 10.0f);
		}
		// rotation
		if (ImGui::CollapsingHeader("Object Global Rotation")) {
			ImGui::DragFloat("X##Rotation", &e.rotation.x, 1.0f, -180.0f, 180.0f);
			ImGui::DragFloat("Y##Rotation", &e.rotation.y, 1.0f, -180.0f, 180.0f);
			ImGui::DragFloat("Z##Rotation", &e.rotation.z, 1.0f, -180.0f, 180.0f);
		}

		ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

		// combo menu for each mesh item
		if (ImGui::BeginCombo("##combo", rs.meshes[e.selectedComponent->meshID]->name.c_str())) {
			for (auto const& comp : comps) {
				bool isSelected = (e.selectedComponent == &comp);
				if (ImGui::Selectable(rs.meshes[comp.meshID]->name.c_str(), isSelected))
					e.selectedComponent == &comp;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::Spacing(); ImGui::Spacing(); ImGui::Spacing();

		Component& c = *e.selectedComponent;

		// Material
		if (ImGui::CollapsingHeader("Material")) {
			ImGui::SeparatorText("Apply Existing Material");
			if (ImGui::BeginListBox("##listbox", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing()))) {
				for (auto const& [matID, mat] : rs.materials) {
					if (ImGui::Selectable(mat->name.c_str(), matID == c.matID)) {
						c.matID = matID;
					}
				}
				ImGui::EndListBox();
			}
		}

		Mesh& mesh = *rs.meshes[e.selectedComponent->meshID];

		// size
		if (ImGui::CollapsingHeader("Size")) {
			if (mesh.type == CUBE) {
				ImGui::DragFloat("Length", &c.scale.x, 1.0f, 0, 100);
				ImGui::DragFloat("Width", &c.scale.y, 1.0f, 0, 100);
				ImGui::DragFloat("Height", &c.scale.z, 1.0f, 0, 100);
			}
			else if (mesh.type == SPHERE) {
				static bool checked = false;
				ImGui::DragFloat("Radius X", &c.scale.x, 1.0f, 0, 100);
				ImGui::DragFloat("Radius Y", &c.scale.y, 1.0f, 0, 100);
				ImGui::DragFloat("Radius Z", &c.scale.z, 1.0f, 0, 100);
				if (ImGui::Checkbox("Uniform Scale", &checked)) {
					if (checked) {
						//TODO: make uniform scale

					}
				}
			}
			else {
				//printf("Mesh type not supported\n");
			}
		}

		// shape, only for sphere
		if (mesh.type == SPHERE) {
			if (ImGui::CollapsingHeader("Shape")) {
				Sphere& sphere = dynamic_cast<Sphere&>(mesh);
				if (ImGui::DragInt("Stack Count", &sphere.stackCount, 1.0, 2, 100)) {
					sphere.updateStacks();
				}
				if (ImGui::DragInt("Sector Count", &sphere.sectorCount, 1.0, 1, 100)) {
					sphere.updateSectors();
				}
			}
		}
	
		if (ImGui::Button("Close")) {
			e.showProperties = false;
		}

		ImGui::End();
	}
	else {
		ImGui::End();
		return;
	}
}

void showAddShader() {
	if (ImGui::Begin("Add Shader", &addShader)) {
		// Button and text box for adding vertex shader

		ImGui::SetNextItemWidth(150);
		ImGui::InputText("##vertexShaderPath", const_cast<char*>(vertexShaderPath.c_str()), vertexShaderPath.size(), ImGuiInputTextFlags_ReadOnly);
		if (ImGui::Button("Add Vertex Shader")) {
			showDialog = true;
			fileType = ".vert";
			curFilePath = &vertexShaderPath;
		}
		ImGui::Spacing();

		// Button and text box for adding fragment shader
		ImGui::SetNextItemWidth(150);
		ImGui::InputText("##fragmentShaderPath", const_cast<char*>(fragmentShaderPath.c_str()), fragmentShaderPath.size(), ImGuiInputTextFlags_ReadOnly);
		if (ImGui::Button("Add Fragment Shader")) {
			showDialog = true;
			fileType = ".frag";
			curFilePath = &fragmentShaderPath;
		}
		ImGui::Spacing();

		// Button and text box for adding geometry shader
		ImGui::SetNextItemWidth(150);
		ImGui::InputText("##geometryShaderPath", const_cast<char*>(geometryShaderPath.c_str()), geometryShaderPath.size(), ImGuiInputTextFlags_ReadOnly);
		if (ImGui::Button("Add Geometry Shader")) {
			showDialog = true;
			fileType = ".geom";
			curFilePath = &geometryShaderPath;
		}
		ImGui::Spacing();
		ImGui::Spacing();

		if (ImGui::Button("Add Shader")) {
			// check if the paths are filled
			if (!vertexShaderPath.empty() && !fragmentShaderPath.empty()) {
				rs.addShader(vertexShaderPath, fragmentShaderPath, geometryShaderPath);
				addShader = false;

				// clear the paths
				vertexShaderPath.clear();
				fragmentShaderPath.clear();
				geometryShaderPath.clear();
			}
		}

		// Close button for the popup
		if (ImGui::Button("Close")) {
			addShader = false;
			// clear the paths
			vertexShaderPath.clear();
			fragmentShaderPath.clear();
			geometryShaderPath.clear();
		}

		ImGui::End();
	}
}

void showAddSkybox() {
	if (ImGui::Begin("Add Skybox")) {
		// Input boxes for each face of the cubemap
		const string faceLabels[] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
		std::string* facePaths[] = { &rightFacePath, &leftFacePath, &topFacePath, &bottomFacePath, &frontFacePath, &backFacePath };

		for (int i = 0; i < 6; i++) {
			ImGui::Text((faceLabels[i] + " Face:").c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(150);
			ImGui::InputText(("##" + std::string(faceLabels[i]) + "FacePath").c_str(), const_cast<char*>(facePaths[i]->c_str()), facePaths[i]->size(), ImGuiInputTextFlags_ReadOnly);

			if (ImGui::Button(("Add " + std::string(faceLabels[i]) + " Face").c_str())) {
				showDialog = true;
				fileType = ".jpg,.png,.jpeg"; // Assuming the cubemap faces are in JPG format
				curFilePath = facePaths[i];
			}

			ImGui::Spacing();
		}

		ImGui::Spacing();

		if (ImGui::Button("Add Cubemap")) {
			bool allPathsFilled = true;
			// check if allhe paths are filled
			for (const string* path : facePaths) 
				if (path->empty()) {
					allPathsFilled = false;
					break;
				}
			if (allPathsFilled) {
				rs.setupSkybox(vector<string>{rightFacePath, leftFacePath, topFacePath, bottomFacePath, frontFacePath, backFacePath});
				addSkybox = false;
				// Reset the paths
				for (string* path : facePaths) {
					path->clear();
				}
			}
		}

		// Close button for the popup
		if (ImGui::Button("Close")) {
			addSkybox = false;
			// Reset the paths
			for (string* path : facePaths) {
				path->clear();
			}
		}

		ImGui::End();
	}
}

// helper function to show the file dialog
void openFileDialog() {
	IGFD::FileDialogConfig config;
	config.path = "."; // default path
	// open file dialog
	ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", fileType.c_str(), config);
}

// helper function to be called in the main loop to show the file dialog
void showFileDialog() {
	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
		if (ImGuiFileDialog::Instance()->IsOk()) {
			// Get the selected file path
			*curFilePath = ImGuiFileDialog::Instance()->GetFilePathName();
		}
		showDialog = false;
		ImGuiFileDialog::Instance()->Close();
	}
}