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
shared_ptr<Shape> sphere, bunny, tree;
vector<glm::mat4> bunnyModels;
vector<glm::vec2> dirs;
constexpr int MESHSIZE = 100;
constexpr int TREEMESHSIZE = 5;
constexpr int TREEDISTANCE = 5;
constexpr int NUMBUNNIES = 2;
constexpr int NUMSNOW = 10000;

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
	glm::vec4 dir;
	int w, a, s, d, i, m, f;
	camera()
	{
		w = a = s = d = i = m = f = 0;
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
		dir = glm::vec4(0, 0, speed, 1) * Rx * Ry;
		pos += glm::vec3(dir.x, dir.y, dir.z);
		dir = glm::vec4(0, 0, 1, 1) * Rx * Ry;
		glm::mat4 T = glm::translate(glm::mat4(1), pos);
		return Rx * Ry * T;
	}
};
camera mycam;

class Application : public EventCallbacks
{
public:

	WindowManager * windowManager = nullptr;

	std::shared_ptr<Program> bunnyprog, treeprog, heightprog, skyprog, bunnyEyesprog, snowprog;
	
	GLuint VertexArrayID, SnowArrayID, VertexBufferID;
	GLuint MeshPosID, MeshTexID, IndexBufferIDBox;//, insBufID;

	GLuint Texture, Texture2, Texture3;

	struct flashLight {
		vec3 pos;
		vec3 dir;
		float cutoff;
	};

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
		if (key == GLFW_KEY_F && action == GLFW_PRESS)
		{
			if (mycam.f == 1)
				mycam.f = 0;
			else
				mycam.f = 1;
		}
	}
	void mouseCallback(GLFWwindow *window, int button, int action, int mods) {}
	void resizeCallback(GLFWwindow *window, int in_width, int in_height)
	{
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
	}
	
	void init_bunny_mMatricies()
	{
		for (int i = 0; i < NUMBUNNIES; i++)
		{
			glm::mat4 TBunny = translate(glm::mat4(1), glm::vec3(rand() % 24 - 12, 0, rand() % 24 - 12));
			bunnyModels.push_back(TBunny);
			dirs.push_back(vec2(0, 0));
		}
	}
	void init_mesh()
	{
		glGenVertexArrays(1, &VertexArrayID);
		glBindVertexArray(VertexArrayID);

		//verticies
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
		glBindBuffer(GL_ARRAY_BUFFER, MeshTexID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * MESHSIZE * MESHSIZE * 4, tex, GL_STATIC_DRAW);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

		//indicies
		glGenBuffers(1, &IndexBufferIDBox);
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
	void initTex(const string &resourceDirectory)
	{
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

		GLuint Tex1Location = glGetUniformLocation(skyprog->pid, "tex");
		glUseProgram(skyprog->pid);
		glUniform1i(Tex1Location, 0);

		Tex1Location = glGetUniformLocation(treeprog->pid, "tex");
		glUseProgram(treeprog->pid);
		glUniform1i(Tex1Location, 0);

		Tex1Location = glGetUniformLocation(heightprog->pid, "tex");
		glUseProgram(heightprog->pid);
		glUniform1i(Tex1Location, 0);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void initSnow()
	{
		//VAO
		glGenVertexArrays(1, &SnowArrayID);
		glBindVertexArray(SnowArrayID);

		//verticies
		glGenBuffers(1, &VertexBufferID);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferID);

		vector<float> points;
		for (int i = 0; i < NUMSNOW; i++)
			points.push_back(randFloat(-1, 1));

		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * points.size(), points.data(), GL_DYNAMIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

		glBindVertexArray(0);
	}

	void initGeom()
	{
		string resourceDirectory = "../resources";
		init_mesh();

		sphere = make_shared<Shape>();
		sphere->loadMesh(resourceDirectory + "/sphere.obj");
		sphere->resize();
		sphere->init();
		
		tree = make_shared<Shape>();
		tree->loadMesh(resourceDirectory + "/tree.obj");
		tree->resize();
		//tree->initInstance(treeprog, TREEMESHSIZE, TREEDISTANCE);
		tree->init();
		
		//glBindVertexArray(tree->vaoID[0]);
		//glm::vec4* instBuf = new glm::vec4[TREEMESHSIZE * TREEMESHSIZE];

		//int i = 0;
		//for (int x = -TREEMESHSIZE / 2; x < TREEMESHSIZE / 2; x++)
		//	for (int z = -TREEMESHSIZE / 2; z < TREEMESHSIZE / 2; z++)
		//	{
		//		instBuf[i] = glm::vec4(x * TREEDISTANCE, 3, z * TREEDISTANCE, 1);
		//		i++;
		//	}
		//glGenBuffers(1, &insBufID);
		//glBindBuffer(GL_ARRAY_BUFFER, insBufID);
		//glBufferData(GL_ARRAY_BUFFER, TREEMESHSIZE * sizeof(glm::vec4), instBuf, GL_STATIC_DRAW);

		//int position_loc = glGetAttribLocation(treeprog->pid, "instancePos");
		//for (int i = 0; i < TREEMESHSIZE* TREEMESHSIZE; i++)
		//{
		//	glVertexAttribPointer(position_loc + i,              // Location
		//		4, GL_FLOAT, GL_FALSE,       // vec4
		//		sizeof(vec4),                // Stride
		//		(void*)(sizeof(vec4) * i));  // Start offset
		//	assert(glGetError() == GL_NO_ERROR);
		//	glEnableVertexAttribArray(position_loc + i);
		//	glVertexAttribDivisor(position_loc + i, 1);
		//}
		//glBindVertexArray(0);

		bunny = make_shared<Shape>();
		bunny->loadMesh(resourceDirectory + "/bunny.obj");
		bunny->resize();
		bunny->init();
		init_bunny_mMatricies();

		initSnow();

		initTex(resourceDirectory);
	}

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
		bunnyprog->addUniform("camdir");
		bunnyprog->addUniform("f");
		bunnyprog->addAttribute("vertPos");
		bunnyprog->addAttribute("vertNor");
		bunnyprog->addAttribute("vertTex");

		//bunny eyes
		bunnyEyesprog = std::make_shared<Program>();
		bunnyEyesprog->setVerbose(true);
		bunnyEyesprog->setShaderNames(resourceDirectory + "/eyes_vertex.glsl", resourceDirectory + "/eyes_fragment.glsl");
		if (!bunnyEyesprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		bunnyEyesprog->addUniform("P");
		bunnyEyesprog->addUniform("V");
		bunnyEyesprog->addUniform("M");
		bunnyEyesprog->addUniform("Mbunny");
		bunnyEyesprog->addUniform("campos");
		bunnyEyesprog->addUniform("camdir");
		bunnyEyesprog->addUniform("f");
		bunnyEyesprog->addAttribute("vertPos");
		bunnyEyesprog->addAttribute("vertNor");
		bunnyEyesprog->addAttribute("vertTex");

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
		treeprog->addUniform("camoff");
		treeprog->addUniform("campos");
		treeprog->addUniform("camdir");
		treeprog->addUniform("f");
		treeprog->addAttribute("vertPos");
		treeprog->addAttribute("vertNor");
		treeprog->addAttribute("vertTex");
		treeprog->addAttribute("instancePos");

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
		heightprog->addUniform("camdir");
		heightprog->addUniform("f");
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

		//snow
		snowprog = std::make_shared<Program>();
		snowprog->setVerbose(true);
		snowprog->setShaderNames(resourceDirectory + "/snow_vertex.glsl", resourceDirectory + "/snow_fragment.glsl");
		if (!snowprog->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1); //make a breakpoint here and check the output window for the error message!
		}
		snowprog->addUniform("P");
		snowprog->addUniform("V");
		snowprog->addUniform("M");
		snowprog->addUniform("camoff");
		snowprog->addUniform("campos");
		snowprog->addUniform("camdir");
		snowprog->addUniform("f");
		snowprog->addAttribute("vertPos");
	}
	void render()
	{
		int width, height;
		double frametime = get_last_elapsed_time();
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		float aspect = width/(float)height;
		glViewport(0, 0, width, height);

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
		sphere->draw(skyprog, FALSE);
		glEnable(GL_DEPTH_TEST);
		skyprog->unbind();

		//bunnies
		static double totalTimePassed = 0;
		static float initial_yspeed = 5;
		static float initial_xzspeed = 0.07;
		static bool jumping = false;
		glm::mat4 TBunny = glm::translate(glm::mat4(1), glm::vec3(0, 0.5f, 0));
		
		float changeInY = initial_yspeed * totalTimePassed + 0.5 * -9.8 * pow(totalTimePassed, 2);
		float changeInXZ = initial_xzspeed * totalTimePassed;
		totalTimePassed += frametime;
		for (int i = 0; i < NUMBUNNIES; i++)
		{
			M = bunnyModels[i] * TBunny;

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
				M[3].x += 0.05 + dirs[i][0] * changeInXZ;
				M[3].y += changeInY;
				M[3].z += 0.05 + dirs[i][1] * changeInXZ;
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
			bunnyprog->bind();
			glUniformMatrix4fv(bunnyprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(bunnyprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniform3f(bunnyprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
			glUniform3f(bunnyprog->getUniform("camdir"), mycam.dir.x, mycam.dir.y, mycam.dir.z);
			glUniform1i(bunnyprog->getUniform("f"), mycam.f);
			glUniformMatrix4fv(bunnyprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			
			bunny->draw(bunnyprog, FALSE);
			bunnyprog->unbind();
			//bunny eyes
			bunnyEyesprog->bind();
			glUniformMatrix4fv(bunnyEyesprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
			glUniformMatrix4fv(bunnyEyesprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
			glUniformMatrix4fv(bunnyEyesprog->getUniform("Mbunny"), 1, GL_FALSE, &M[0][0]);
			glUniform3f(bunnyEyesprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
			glUniform3f(bunnyEyesprog->getUniform("camdir"), mycam.dir.x, mycam.dir.y, mycam.dir.z);
			glUniform1i(bunnyEyesprog->getUniform("f"), mycam.f);			

			mat4 SE = scale(mat4(1.0f), vec3(0.1, 0.1, 0.1));
			mat4 TR = translate(mat4(1.0f), vec3(-.95, 0.3, 0.3));
			glm::mat4 M1 = M * TR;
			M = M1 * SE;
			glUniformMatrix4fv(bunnyEyesprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			sphere->draw(bunnyEyesprog, FALSE);
			mat4 TL = translate(mat4(1.0f), vec3(0.3, 0, 0.25));
			glm::mat4 M2 = M1 * TL;
			M = M2 * SE;
			glUniformMatrix4fv(bunnyEyesprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
			sphere->draw(bunnyEyesprog, FALSE);
			bunnyEyesprog->unbind();
		}

		//tree
		treeprog->bind();
		glUniformMatrix4fv(treeprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(treeprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3f(treeprog->getUniform("camoff"), ((int)(-mycam.pos.x/ TREEDISTANCE)* TREEDISTANCE), 0, ((int)(-mycam.pos.z / TREEDISTANCE) * TREEDISTANCE));
		glUniform3f(treeprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
		glUniform3f(treeprog->getUniform("camdir"), mycam.dir.x, mycam.dir.y, mycam.dir.z);
		glUniform1i(treeprog->getUniform("f"), mycam.f);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture3);
		for (int x = -TREEMESHSIZE/2; x < TREEMESHSIZE/2; x++)
			for (int z = -TREEMESHSIZE/2; z < TREEMESHSIZE/2; z++)
			{
				glm::mat4 STree = glm::scale(glm::mat4(1.0f), glm::vec3(3, 3, 3));
				glm::mat4 Trans = glm::translate(glm::mat4(1), glm::vec3(x * TREEDISTANCE, 3, z * TREEDISTANCE));
				M = Trans * STree;
				glUniformMatrix4fv(treeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
				tree->draw(treeprog, FALSE);
			}
		//glm::mat4 Trans = glm::translate(glm::mat4(1), glm::vec3(0, 0, -3));
		//M = Trans;
		//glUniformMatrix4fv(treeprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		//glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, (void*)0, TREEMESHSIZE*TREEMESHSIZE);
		//tree->drawInstanced(treeprog, FALSE);
		treeprog->unbind();

		//terrain
		heightprog->bind();
		glUniformMatrix4fv(heightprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(heightprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);

		glm::mat4 TransY = glm::translate(glm::mat4(1.0f), glm::vec3(-50.0f, 0, -50));
		M = TransY;

		glUniformMatrix4fv(heightprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glUniform3f(heightprog->getUniform("camoff"), (int)-mycam.pos.x, 0, (int)-mycam.pos.z);
		glUniform3f(heightprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
		glUniform3f(heightprog->getUniform("camdir"), mycam.dir.x, mycam.dir.y, mycam.dir.z);
		glUniform1i(heightprog->getUniform("f"), mycam.f);
		glBindVertexArray(VertexArrayID);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferIDBox);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		glDrawElements(GL_TRIANGLES, MESHSIZE*MESHSIZE*6, GL_UNSIGNED_SHORT, (void*)0);

		heightprog->unbind();

		snowprog->bind();
		glBindVertexArray(SnowArrayID);
		glUniformMatrix4fv(snowprog->getUniform("P"), 1, GL_FALSE, &P[0][0]);
		glUniformMatrix4fv(snowprog->getUniform("V"), 1, GL_FALSE, &V[0][0]);
		glUniform3f(snowprog->getUniform("campos"), -mycam.pos.x, -mycam.pos.y, -mycam.pos.z);
		glUniform3f(snowprog->getUniform("camdir"), mycam.dir.x, mycam.dir.y, mycam.dir.z);
		glUniform1i(snowprog->getUniform("f"), mycam.f);
		mat4 S12 = scale(mat4(1), vec3(100, 5, 100));
		mat4 T12 = translate(mat4(1), vec3(0, 3, 0));
		M = T12 * S12;

		glUniformMatrix4fv(snowprog->getUniform("M"), 1, GL_FALSE, &M[0][0]);
		glPointSize(3);
		glDrawArrays(GL_POINTS, 0, NUMSNOW);
		snowprog->unbind();
		glBindVertexArray(0);
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

	while(!glfwWindowShouldClose(windowManager->getHandle()))
	{
		application->render();
		glfwSwapBuffers(windowManager->getHandle());
		glfwPollEvents();
	}
	windowManager->shutdown();
	return 0;
}