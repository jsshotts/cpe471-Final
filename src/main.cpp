#define STB_IMAGE_IMPLEMENTATION
#define _USE_MATH_DEFINES
#include <iostream>
#include <glad/glad.h>
#include "stb_image.h"
#include "GLSL.h"
#include "Program.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Shape.h"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;
shared_ptr<Shape> skySphere, bunny, tree;
vector<glm::mat4> treeModels;
vector<glm::mat4> bunnyModels;
vector<glm::vec2> dirs;
constexpr int MESHSIZE = 100;
constexpr int NUMTREES = 75;
constexpr int NUMBUNNIES = 5;

float randFloat(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

double get_last_elapsed_time()
{
	static double lasttime = glfwGetTime();
	double actualtime =glfwGetTime();
	double difference = actualtime- lasttime;
	lasttime = actualtime;
	return difference;
}
class camera
{
public:
	glm::vec3 pos, rot;
	int w, a, s, d, i, m;
	camera()
	{
		w = a = s = d = i = m = 0;
		pos = rot = glm::vec3(0, 0, 0);
	}
	glm::mat4 process(double ftime)
	{
		float speed = 0;
		if (w == 1)
			speed = 10 * ftime;
		else if (s == 1)
			speed = -10 * ftime;
		float yangle = 0, xangle = 0;
		if (a == 1)
			yangle = -3 * ftime;
		else if (d == 1)
			yangle = 3 * ftime;
		if (i == 1)
			xangle = -3 * ftime;
		else if (m == 1)
			xangle = 3 * ftime;
		rot.y += yangle;
		rot.x += xangle;
		glm::mat4 Ry = glm::rotate(glm::mat4(1), rot.y, glm::vec3(0, 1, 0));
		glm::mat4 Rx = glm::rotate(glm::mat4(1), rot.x, glm::vec3(1, 0, 0));
		glm::vec4 dir = glm::vec4(0, 0, speed, 1);
		dir = dir * Rx * Ry;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return Rx * Ry * T;
	}
};
camera mycam;

class Application : public EventCallbacks
{

public:

	WindowManager * windowManager = nullptr;

	std::shared_ptr<Program> bunnyprog, treeprog, heightprog, skyprog;
	GLuint VertexArrayID;

	// Data necessary to give our box to OpenGL
	GLuint MeshPosID, MeshTexID, IndexBufferIDBox;

	GLuint Texture, Texture2, Texture3;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		if (key == GLFW_KEY_W && action == GLFW_PRESS)
		{
			mycam.w = 1;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE)
		{
			mycam.w = 0;
		}
		if (key == GLFW_KEY_S && action == GLFW_PRESS)
		{
			mycam.s = 1;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE)
		{
			mycam.s = 0;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS)
		{
			mycam.a = 1;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE)
		{
			mycam.a = 0;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS)
		{
			mycam.d = 1;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE)
		{
			mycam.d = 0;
		}
		if (key == GLFW_KEY_I && action == GLFW_PRESS)
		{
			mycam.i = 1;
		}
		if (key == GLFW_KEY_I && action == GLFW_RELEASE)
		{
			mycam.i = 0;
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			mycam.m = 1;
		}
		if (key == GLFW_KEY_M && action == GLFW_RELEASE)
		{
			mycam.m = 0;
		}
	}
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {}
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	
	void init_tree_mMatricies()
	{
		for (int i = 0; i < NUMTREES; i++)
		{
			glm::mat4 STree = glm::scale(glm::mat4(1.0f), glm::vec3(3 + randFloat(-0.4, 0.4), 3, 3 + randFloat(-0.4, 0.4)));
			glm::mat4 TTree = translate(glm::mat4(1), glm::vec3(rand() % 24 - 12, 2, rand() % 24 - 12));
			treeModels.push_back(TTree * STree);
		}
	}
	void init_bunny_mMatricies()
	{
		for (int i = 0; i < NUMTREES; i++)
		{
			glm::mat4 TBunny = translate(glm::mat4(1), glm::vec3(rand() % 24 - 12, 0, rand() % 24 - 12));
			bunnyModels.push_back(TBunny);
			dirs.push_back(vec2(0, 0));
		}
	}
	void init_mesh()
	{
		//generate the VAO
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//generate vertex buffer to hand off to OGL
		glGenBuffers(1, &MeshPosID);
		glBindBuffer(GL_ARRAY_BUFFER, MeshPosID);
		vec3 vertices[MESHSIZE * MESHSIZE * 4];
		for(int x=0;x<MESHSIZE;x++)
			for (int z = 0; z < MESHSIZE; z++)
			{
				vertices[x * 4 + z*MESHSIZE * 4 + 0] = vec3(0.0, 0.0, 0.0) + vec3(x, 0, z);
				vertices[x * 4 + z*MESHSIZE * 4 + 1] = vec3(1.0, 0.0, 0.0) + vec3(x, 0, z);
				vertices[x * 4 + z*MESHSIZE * 4 + 2] = vec3(1.0, 0.0, 1.0) + vec3(x, 0, z);
				vertices[x * 4 + z*MESHSIZE * 4 + 3] = vec3(0.0, 0.0, 1.0) + vec3(x, 0, z);
			}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec3) * MESHSIZE * MESHSIZE * 4, vertices, GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//tex coords
		float t = 1. / 100;
		vec2 tex[MESHSIZE * MESHSIZE * 4];
		for (int x = 0; x<MESHSIZE; x++)
			for (int y = 0; y < MESHSIZE; y++)
			{
				tex[x * 4 + y*MESHSIZE * 4 + 0] = vec2(0.0, 0.0)+ vec2(x, y)*t;
				tex[x * 4 + y*MESHSIZE * 4 + 1] = vec2(t, 0.0)+ vec2(x, y)*t;
				tex[x * 4 + y*MESHSIZE * 4 + 2] = vec2(t, t)+ vec2(x, y)*t;
				tex[x * 4 + y*MESHSIZE * 4 + 3] = vec2(0.0, t)+ vec2(x, y)*t;
			}
		glGenBuffers(1, &MeshTexID);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 4, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glGenBuffers(1, &IndexBufferIDBox);
		//set the current state to focus on our vertex buffer
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		GLushort elements[MESHSIZE * MESHSIZE * 6];
		int ind = 0;
		for (int i = 0; i<MESHSIZE * MESHSIZE * 6; i+=6, ind+=4)
		{
			elements[i + 0] = ind + 0;
			elements[i + 1] = ind + 1;
			elements[i + 2] = ind + 2;
			elements[i + 3] = ind + 0;
			elements[i + 4] = ind + 2;
			elements[i + 5] = ind + 3;
		}
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort)*MESHSIZE * MESHSIZE * 6, elements, GL_STATIC_DRAW);
		glBindVertexArray(0);
	}

	void initGeom()
	{
		string resourceDirectory = "../resources";
		init_mesh();

		skySphere = make_shared<Shape>();
		skySphere->loadMesh(resourceDirectory + "/sphere.obj");
		skySphere->resize();
		skySphere->init();

		tree = make_shared<Shape>();
		tree->loadMesh(resourceDirectory + "/tree.obj");
		tree->resize();
		tree->init();
		init_tree_mMatricies();

		bunny = make_shared<Shape>();
		bunny->loadMesh(resourceDirectory + "/bunny.obj");
		bunny->resize();
		bunny->init();
		init_bunny_mMatricies();

		int width, height, channels;
		char filepath[1000];

		//texture 1
		string str = resourceDirectory + "/grass.jpg";
		strcpy(filepath, str.c_str());
		unsigned char* data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 2
		str = resourceDirectory + "/sky_nightime.jpg";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		//texture 3
		str = resourceDirectory + "/tree.png";
		strcpy(filepath, str.c_str());
		data = stbi_load(filepath, &width, &height, &channels, 4);
		glGenTextures(1, &Texture3);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		////[TWOTEXTURES]
		////set the 2 textures to the correct samplers in the fragment shader:
		//GLuint Tex1Location = glGetUniformLocation(prog->pid, "tex");//tex, tex2... sampler in the fragment shader
		//GLuint Tex2Location = glGetUniformLocation(prog->pid, "tex2");
		//// Then bind the uniform samplers to texture units:
		//glUseProgram(prog->pid);
		//glUniform1i(Tex1Location, 0);
		//glUniform1i(Tex2Location, 1);

		//Tex1Location = glGetUniformLocation(heightprog->pid, "tex");//tex, tex2... sampler in the fragment shader
		//// Then bind the uniform samplers to texture units:
		//glUseProgram(heightprog->pid);
		//glUniform1i(Tex1Location, 0);

		//glEnable(GL_BLEND);
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	//General OGL initialization - set OGL state here
	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glEnable(GL_DEPTH_TEST);

		//bunny
		bunnyprog = std::make_shared<Program>();
		bunnyprog->setVerbose(true);
		bunnyprog->setShaderNames(resourceDirectory + "/bunny_vertex.glsl", resourceDirectory + "/bunny_fragment.glsl");
		if (!bunnyprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		bunnyprog->addUniform("P");
		bunnyprog->addUniform("V");
		bunnyprog->addUniform("M");
		bunnyprog->addUniform("campos");
		bunnyprog->addAttribute("vertPos");
		bunnyprog->addAttribute("vertNor");
		bunnyprog->addAttribute("vertTex");

		treeprog = std::make_shared<Program>();
		treeprog->setVerbose(true);
		treeprog->setShaderNames(resourceDirectory + "/tree_vertex.glsl", resourceDirectory + "/tree_fragment.glsl");
		if (!treeprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		treeprog->addUniform("P");
		treeprog->addUniform("V");
		treeprog->addUniform("M");
		treeprog->addUniform("campos");
		treeprog->addAttribute("vertPos");
		treeprog->addAttribute("vertNor");
		treeprog->addAttribute("vertTex");

		//height
		heightprog = std::make_shared<Program>();
		heightprog->setVerbose(true);
		heightprog->setShaderNames(resourceDirectory + "/height_vertex.glsl", resourceDirectory + "/height_frag.glsl");
		if (!heightprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		heightprog->addUniform("P");
		heightprog->addUniform("V");
		heightprog->addUniform("M");
		heightprog->addUniform("camoff");
		heightprog->addUniform("campos");
		heightprog->addAttribute("vertPos");
		heightprog->addAttribute("vertTex");

		//sky
		skyprog = std::make_shared<Program>();
		skyprog->setVerbose(true);
		skyprog->setShaderNames(resourceDirectory + "/sky_vertex.glsl", resourceDirectory + "/sky_fragment.glsl");
		if (!skyprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		skyprog->addUniform("P");
		skyprog->addUniform("V");
		skyprog->addUniform("M");
		skyprog->addAttribute("vertPos");
		skyprog->addAttribute("vertNor");
		skyprog->addAttribute("vertTex");
	}
	void render()
	{
		int width, height;
		double frametime = get_last_elapsed_time();
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

		//glClearColor(0.8f, 0.8f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glm::mat4 V, M, P;
		P = glm::perspective((float)(3.14159 / 4.), (float)((float)width/ (float)height), 0.1f, 1000.0f);
		V = mycam.process(frametime);

		//sky
		skyprog->bind();
		glUniformMatrix4fv(skyprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(skyprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		glm::mat4 SSky = glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f));
		glm::mat4 RSky = glm::rotate(glm::mat4(1.0f), (float)M_PI/2, glm::vec3(1, 0, 0));
		glm::mat4 TSky = translate(glm::mat4(1), -mycam.pos);
		M = TSky * RSky * SSky;
		glUniformMatrix4fv(skyprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture2);
		glDisable(GL_DEPTH_TEST);
		skySphere->draw(skyprog, FALSE);
		glEnable(GL_DEPTH_TEST);
		skyprog->unbind();

		//bunnies
		bunnyprog->bind();
		glUniformMatrix4fv(bunnyprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(bunnyprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3f(bunnyprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);

		static double totalTimePassed = 0;
		static float initial_yspeed = 5;
		static float initial_xzspeed = 0.07;
		static bool jumping = false;
		
		float changeInY = initial_yspeed * totalTimePassed + 0.5 * -9.8 * pow(totalTimePassed, 2);
		float changeInXZ = initial_xzspeed * totalTimePassed;
		totalTimePassed += frametime;
		for (int i = 0; i < NUMBUNNIES; i++)
		{
			M = bunnyModels[i];

			//if jumping, and we are back at where we are before offset (so we are done jumping)
			if (jumping && M[3].y + changeInY < M[3].y)
			{
				//jump is over
				jumping = false;
				totalTimePassed = 0;
			}
			//if jumping, and we should continue with our jump
			else if (jumping)
			{
				M[3].x += 0.05 + dirs[i][0]*changeInXZ;
				M[3].y += changeInY;
				M[3].z += 0.05 + dirs[i][1]*changeInXZ;
				bunnyModels[i][3].x = M[3].x;
				bunnyModels[i][3].z = M[3].z;
			}
			//not jumping, and waiting until we should jump
			else
			{
				if (totalTimePassed > 1)
				{
					jumping = true;
					totalTimePassed = 0;
					dirs[i] = normalize(vec2(-mycam.pos.x - M[3].x, -mycam.pos.z - M[3].z));
				}
			}
			glUniformMatrix4fv(bunnyprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			bunny->draw(bunnyprog, FALSE);
		}
		bunnyprog->unbind();

		//tree
		treeprog->bind();
		glUniformMatrix4fv(treeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(treeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3f(treeprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		for (int i = 0; i < NUMTREES; i++)
		{	
			M = treeModels[i];
			glUniformMatrix4fv(treeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			tree->draw(treeprog, FALSE);
		}
		treeprog->unbind();

		//terrain
		heightprog->bind();
		glUniformMatrix4fv(heightprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(heightprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, -0.91f, -50));
		M = TransY;
		glUniformMatrix4fv(heightprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		
		vec3 offset = mycam.pos;
		offset.y = 0;
		offset.x = (int)offset.x;
		offset.z = (int)offset.z;
		glUniform3fv(heightprog->getUniform("camoff"), 1, &offset[0]);
		glUniform3f(heightprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
		glBindVertexArray(VertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glDrawElements(GL_TRIANGLES, MESHSIZE*MESHSIZE*6, GL_UNSIGNED_SHORT, (void*)0);

		heightprog->unbind();
			   		 	  	  	   	




		//glm::mat4 TBunny = glm::translate(glm::mat4(1), glm::vec3(0, 0, -3));
		//

		//vec2 a = vec2(0, 1);
		//vec2 b = vec2(-mycam.pos.x - TBunny[3].x, -mycam.pos.z - TBunny[3].z);

		//float angl = acos(dot(a, b) / (length(a) * length(b)));

		//mat4 Vi = transpose(V);
		///*Vi[0][3] = 0;
		//Vi[1][3] = 0;
		//Vi[2][3] = 0;*/
		//// ((float)M_PI / 7.5f) + 
		//glm::mat4 RBunny = glm::rotate(glm::mat4(1), angl, glm::vec3(0, 1, 0));

		//M = TBunny * Vi;
		//glUniformMatrix4fv(bunnyprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//bunny->draw(bunnyprog, FALSE);
	}
};
//******************************************************************************************
int main(int argc, char **argv)
{
	std::string resourceDir = "../resources";
	if (argc >= 2)
		resourceDir = argv[1];
	srand(0);
	Application *application = new Application();
	WindowManager * windowManager = new WindowManager();
	windowManager->init(1600, 800);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	application->init(resourceDir);
	application->initGeom();

	while(! glfwWindowShouldClose(windowManager->getHandle()))
	{
		application->render();
		glfwSwapBuffers(windowManager->getHandle());
		glfwPollEvents();
	}
	windowManager->shutdown();
	return 0;
}