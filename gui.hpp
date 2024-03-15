#pragma once

#include "entity.hpp"
#include "light.hpp"

extern void showMainMenuBar();
void showObjList();
void showMaterialList();
void showLightList();
void showMaterialProperties(Material&);
void showObjectProperties(Entity&);
void showLightProperties(Light&);
void showAddShader();
void showAddSkybox();
extern void showFileDialog();
extern void showObjList();
