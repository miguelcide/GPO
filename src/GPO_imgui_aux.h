#include <GpO.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

void init_imgui(GLFWwindow* window);
void terminate_imgui(void);
void imgui_newframe(void);
void imgui_renderframe(void);

bool imgui_renderShaderSelect(int* nProg);
bool imgui_renderCameraPos(float* d, float* az, float* el);
bool imgui_renderLightVec(float* az, float* el);
