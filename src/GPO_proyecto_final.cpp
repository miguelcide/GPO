/************************  GPO_01 ************************************
ATG, 2019
******************************************************************************/

#include <GpO.h>
#include "GPO_imgui_aux.h"
#include "GPO_assimp_aux.h"

// TAMA�O y TITULO INICIAL de la VENTANA
int ANCHO = 800, ALTO = 600;  // Tama�o inicial ventana
const char* prac = "OpenGL (GpO)";   // Nombre de la practica (aparecera en el titulo de la ventana).

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////   RENDER CODE AND DATA
//////////////////////////////////////////////////////////////////////////////////////////////////////////////

GLFWwindow* window;
GLuint prog;
objeto obj;
struct escena isla;
struct escena torre;
struct escena* escenaActual = &isla;

// Dibuja objeto indexado
void dibujar_indexado(objeto obj) {
	glBindVertexArray(obj.VAO);              // Activamos VAO asociado al objeto
	glDrawElements(GL_TRIANGLES,obj.Ni,obj.tipo_indice,(void*)0);  // Dibujar (indexado)
	glBindVertexArray(0);
}

void dibujar_escena() {
	for (unsigned int i = 0; i < escenaActual->nInstancias; i++) {
		const unsigned int j = escenaActual->instIdx[i];
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, escenaActual->mats[j]);
		dibujar_indexado(escenaActual->objs[j]);
	}
}

// Preparaci�n de los datos de los objetos a dibujar, envialarlos a la GPU
// Compilaci�n programas a ejecutar en la tarjeta gr�fica:  vertex shader, fragment shaders
// Opciones generales de render de OpenGL
void init_scene()
{
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Mandar programas a GPU, compilar y crear programa en GPU
	char* vertex = leer_codigo_de_fichero("data/prog.vs");
	char* fragment = leer_codigo_de_fichero("data/prog.fs");
	prog = Compile_Link_Shaders(vertex, fragment);
	delete []vertex;
	delete []fragment;

	glUseProgram(prog);

	obj = cargar_modelo("data/buda_n.bix");
	isla = cargar_modelo_assimp("data/Island/Island.obj");
	torre = cargar_modelo_assimp("data/Tower/Tower.obj");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

vec3 pos_obs = vec3(8.0f,0.0f,0.0f);
vec3 target = vec3(0.0f,0.0f,0.0f);
vec3 up = vec3(0,1,0);
float fov = 35.0f, aspect = 4.0f / 3.0f;

vec3 luz = vec3(1, 0, 0);
vec3 colorLuz = vec3(1, 1, 1);
vec4 coeficientes = vec4(0.1, 0.6, 0.3, 16);

float grosorBorde = 0.2f;
vec3 colorBorde = vec3(1, 1, 1);

// Actualizar escena: cambiar posici�n objetos, nuevos objetros, posici�n c�mara, luces, etc.
void render_scene()
{
	glClearColor(0.0f,0.0f,0.0f,1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float t = (float)glfwGetTime();  // Contador de tiempo en segundos 

	///////// Actualizacion matriz MVP  /////////
	mat4 P = perspective(glm::radians(fov), aspect, 0.5f, 32.0f);
	mat4 V = lookAt(pos_obs, target, up);
	mat4 PV = P * V;
	mat4 M = translate(vec3(0.0f, -1.0f, 0.0f));

	transfer_mat4("PV", PV);
	//transfer_mat4("M", M);
	transfer_vec3("camera", pos_obs);
	transfer_vec3("luz", luz);
	transfer_vec3("colorLuz", colorLuz);
	transfer_vec4("coeficientes", coeficientes);
	transfer_float("grosorBorde", grosorBorde);
	transfer_vec3("colorBorde", colorBorde);

	// ORDEN de dibujar
	//dibujar_indexado(obj);
	transfer_mat4("M", mat4(1.0f)); //Las escenas ya tienen sus matrices de transformación aplicadas
	transfer_int("unit", 0);
	dibujar_escena();
}

//////////  FUNCION PARA PROCESAR VALORES DE IMGUI  //////////
void render_imgui(void) {
	static bool useBlinn = false;
	static bool useToon = false;
	static int nScene = 0;
	static struct {
		float d = 8.0f;
		float az = 0.0f;
		float el = 0.0f;
	} camara;
	static struct {
		float az = 0.0f;
		float el = 0.0f;
	} luzGlobal;

	ImGui::Begin("Controls");

	if (imgui_renderShaderSelect(&useBlinn, &useToon)) {
		transfer_int("blinn", useBlinn);
		transfer_int("toon", useToon);
	}
	if (imgui_renderSceneSelect(&nScene)) {
		switch (nScene) {
			case 0:
				escenaActual = &isla;
				break;
			case 1:
				escenaActual = &torre;
				break;
		}
	}
	if (imgui_renderCameraPos(&camara.d, &camara.az, &camara.el))
		pos_obs = camara.d * vec3(cos(camara.az) * cos(camara.el), sin(camara.el), sin(camara.az) * cos(camara.el));
	if (imgui_renderLightVec(&luzGlobal.az, &luzGlobal.el))
		luz = vec3(cos(luzGlobal.az) * cos(luzGlobal.el), sin(luzGlobal.el), sin(luzGlobal.az) * cos(luzGlobal.el));
	imgui_renderLightColor(&colorLuz);
	imgui_renderCoefficients(&coeficientes);
	imgui_renderBorderSettings(&colorBorde, &grosorBorde);

	ImGui::End();
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// PROGRAMA PRINCIPAL
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
	init_GLFW();            // Inicializa lib GLFW
	window = Init_Window(prac);  // Crea ventana usando GLFW, asociada a un contexto OpenGL	X.Y

	init_imgui(window);

	load_Opengl();         // Carga funciones de OpenGL, comprueba versi�n.
	init_scene();          // Prepara escena
	
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		imgui_newframe();

		render_scene();

		//Controles de IMGUI
		render_imgui();

		imgui_renderframe();
		glfwSwapBuffers(window);
		show_info();
	}

	limpiar_escena(&isla);
	limpiar_escena(&torre);
	terminate_imgui();
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

//////////  FUNCION PARA MOSTRAR INFO OPCIONAL EN EL TITULO DE VENTANA  //////////
void show_info()
{
	static int fps = 0;
	static double last_tt = 0;
	double elapsed, tt;
	char nombre_ventana[128];   // buffer para modificar titulo de la ventana

	fps++; tt = glfwGetTime();  // Contador de tiempo en segundos 

	elapsed = (tt - last_tt);
	if (elapsed >= 0.5)  // Refrescar cada 0.5 segundo
	{
		sprintf_s(nombre_ventana, 128, "%s: %4.0f FPS @ %d x %d", prac, fps / elapsed, ANCHO, ALTO);
		glfwSetWindowTitle(window, nombre_ventana);
		last_tt = tt; fps = 0;
	}

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////  ASIGNACON FUNCIONES CALLBACK
///////////////////////////////////////////////////////////////////////////////////////////////////////////


// Callback de cambio tama�o de ventana
void ResizeCallback(GLFWwindow* window, int width, int height)
{
	glfwGetFramebufferSize(window, &width, &height); 
	glViewport(0, 0, width, height);
	ALTO = height;	ANCHO = width;

	aspect = (float)ANCHO / ALTO;
}

// Callback de pulsacion de tecla
static void KeyCallback(GLFWwindow* window, int key, int code, int action, int mode)
{
	switch (key) {
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, true);
		break;
	}
}


void asigna_funciones_callback(GLFWwindow* window)
{
	glfwSetWindowSizeCallback(window, ResizeCallback);
	glfwSetKeyCallback(window, KeyCallback);
}



