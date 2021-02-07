// Use C++ standard libaries
#include <cstdlib>
#include <iostream>
#include <time.h>
#include <chrono>
#include <vector>
using namespace std;

// Use glew
#include <GL/glew.h>

// Use SDL for windowing
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

// Use glm for matrix
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

// Add loadshaders file
#include "common/loadshaders.h"

// Add object loadshader
#define TINYOBJLOADER_IMPLEMENTATION
#include "common/tiny_obj_loader.h"

// Begin Structs
struct attributes {
    GLfloat coord3d[3];
    GLfloat v_color[3];
};

struct Vertex {
    vec3 pos;
    vec3 color;
    vec2 texCoord;
};

// Begin Globals
struct attributes triangle_attributes[] = {
  {{ 0.8,  0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{-0.8, -0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{ 0.8, -0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{ 0.8,  0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{-0.8, -0.8, 0.0}, {1.0, 1.0, 1.0}},
  {{-0.8,  0.8, 0.0}, {1.0, 1.0, 1.0}}
};

typedef struct ballStats
{
	GLfloat xpos = 0;
	GLfloat ypos = 0;
	GLfloat xSpeed = 0;
	GLfloat ySpeed = 0;
}ballStats;

const char* waveFileNames[] =
{
"AudioResources/Kick-Drum-1.wav",
"AudioResources/Electronic-Tom-1.wav",
};

const char* modelFileNames[] = {
  "models/wolf.obj",
};

Mix_Chunk* sound[2];
Vertex** model[1];

glm::mat4 View;
glm::mat4 Projection;
glm::mat4 Model;

mat4 MVP;
mat4 MVP2;
mat4 MVP3;
mat4 MVP4;

GLuint MatrixID;

mat4 p1translateMatrix;
mat4 p2translateMatrix;
mat4 balltranslateMatrix;
mat4 paddleScaleMatrix;
mat4 ballScaleMatrix;


GLfloat p1VertPosition = 0;
GLfloat p2VertPosition = 0;

// Begin GL and SDL
class pongGraphics{
  public:
    void run(){
      initOGL();
      graphicsLoop();
      cleanup();
    }

  private:
    SDL_Window* window;
    GLenum glew_status;
    GLuint program;

    GLuint paddleLeft, paddleRight;
    GLuint vboBall, vboBallColors;
    GLuint testmodel;
    GLint attribute_coord3d, attribute_v_color, attribute_texcoord;

    GLint uniform_fade;

    SDL_Event input;

    int audioID;

    bool runFlag;
    bool holdFlag;
    timespec* delayTime;

    ballStats *ball;
    int p1Score;
    int p2Score;

    GLuint texture_id, program_id;
    GLint uniform_mytexture;

    vector<glm::vec4> wolfVerts;
    vector<glm::vec3> wolfNormals;
    vector<GLushort> wolfElements;

    void initOGL(){
      initSDL();
      initGlew();
      initAudio();
      initTransparency();
      initShaders();
      //load_obj("models/wolf.obj", )
      loadModel();
      //loadTextures();
      createVertexBuffers();
      //initUniformVariables();
      initGlobals();
    }

    void initSDL(){
      /* SDL-related initialising functions */
    	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    	window = SDL_CreateWindow("Pong - Square Color Test",
    		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    		640, 480,
    		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

      if(window == NULL) {
        cerr << "Error: cannot create instance of window: " << SDL_GetError() << '\n';
        exit(1);
      }

      SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
      //SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
      SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 1);
    	if (SDL_GL_CreateContext(window) == NULL) {
    		cerr << "Error: SDL_GL_CreateContext: " << SDL_GetError() << endl;
    		exit(1);
    	}

    	SDL_GL_CreateContext(window);
    }

    void initGlew(){
      // Extension wrangler initialising
      glew_status = glewInit();
      if (glew_status != GLEW_OK && !GLEW_VERSION_2_0) {
      		cerr << "Error: glewInit: " << glewGetErrorString(glew_status) << endl;
      		exit(EXIT_FAILURE);
      	}
    }

    void initAudio(){
      // Set up the audio stream
      audioID = Mix_OpenAudio(44100, AUDIO_S16SYS, 2, 512);
      if( audioID < 0 )
      {
          fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
          exit(-1);
      }

      audioID = Mix_AllocateChannels(4);
      if( audioID < 0 )
      {
          fprintf(stderr, "Unable to allocate mixing channels: %s\n", SDL_GetError());
          exit(-1);
      }

      // Load waveforms
      for( int i = 0; i < (int)(sizeof(sound)/sizeof(sound[0])); i++ )
      {
          sound[i] = Mix_LoadWAV(waveFileNames[i]);
          if( sound[i] == NULL )
          {
              fprintf(stderr, "Unable to load wave file: %s\n", waveFileNames[i]);
          }
      }

    }

    void initShaders(){
      GLuint vs, fs;
      vs = glCreateShader(GL_VERTEX_SHADER);

      if ((vs = create_shader("shaders/vertexshader", GL_VERTEX_SHADER))   == 0) exit(1);
    	if ((fs = create_shader("shaders/fragmentshader", GL_FRAGMENT_SHADER)) == 0) exit(1);

      GLint link_ok = GL_FALSE;
      program = glCreateProgram();
    	glAttachShader(program, vs);
    	glAttachShader(program, fs);
    	glLinkProgram(program);
    	glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    	if (!link_ok) {
    		cerr << "Error in glLinkProgram" << endl;
        print_log(program);
    		exit(1);
    	}

      const char* attribute_name = "coord3d";
    	attribute_coord3d = glGetAttribLocation(program, attribute_name);
    	if (attribute_coord3d == -1) {
    		cerr << "Could not bind attribute " << attribute_name << endl;
    		exit(1);
    	}
    }

    void createVertexBuffers(){
      glGenBuffers(1, &vboBall);
    	glBindBuffer(GL_ARRAY_BUFFER, vboBall);

      glGenBuffers(1, &paddleLeft);
    	glBindBuffer(GL_ARRAY_BUFFER, paddleLeft);

      glGenBuffers(1, &paddleRight);
    	glBindBuffer(GL_ARRAY_BUFFER, paddleRight);

      glGenBuffers(1, &testmodel);
    	glBindBuffer(GL_ARRAY_BUFFER, testmodel);

      glGenBuffers(1, &vboBallColors);
      glBindBuffer(GL_ARRAY_BUFFER, vboBallColors);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

      const char* attribute_name2 = "v_color";
      attribute_v_color = glGetAttribLocation(program, attribute_name2);
      if (attribute_v_color == -1) {
        cerr << "Could not bind attribute " << attribute_name2 << endl;
        exit(1);
      }
    }

    void initTransparency(){
      // Enable Alpha
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void initUniformVariables(){
      const char* uniform_name;
      uniform_name = "fade";
      uniform_fade = glGetUniformLocation(program, uniform_name);
      if (uniform_fade == -1) {
        cerr << "Could not bind uniform_fade " << uniform_name << endl;
        exit(1);
      }
      uniform_fade = 1.0f;
    }

    void initGlobals(){
      View = glm::lookAt(
    		glm::vec3(0,0,5), // Camera is at (4,3,3), in World Space
    		glm::vec3(0,0,0), // and looks at the origin
    		glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
    	);
      // Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
      Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
      Model = glm::mat4(1.0f);

      ball = (ballStats *)malloc(sizeof(ballStats));

      ball -> xpos = 0;
    	ball -> ypos = 0;
    	ball -> xSpeed = 0;
    	ball -> ySpeed = 0;

      p1Score = 0;
      p2Score = 0;

      holdFlag = 0;
      delayTime = (timespec *)malloc(sizeof(timespec));
      delayTime -> tv_sec = 0;
      delayTime -> tv_nsec = 400000000;

      MatrixID = glGetUniformLocation(program, "MVP");
    }

    void loadModel() {
      tinyobj::attrib_t attrib;
      std::vector<tinyobj::shape_t> shapes;
      std::vector<tinyobj::material_t> materials;
      std::string warn, err;

      for(int i = 0; i < (int)(sizeof(model) / sizeof(model[0])); i++){
        for(int j = 0; j < (int)(sizeof(shapes) / sizeof(shapes[0])); j++){
          for(const auto& index : shapes[j].mesh.indices){
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelFileNames[i])) {
                cerr << "Could not load model: " << modelFileNames[i] << endl;
                exit(1);
            }

            //model[i] = (Vertex *)malloc(sizeof(Vertex));

            model[i][j] -> pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            model[i][j] -> texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            model[i][j] -> color = {1.0f, 1.0f, 1.0f};
          }
        }
      }
    }

    void load_obj(const char* filename, vector<glm::vec4> &vertices, vector<glm::vec3> &normals, vector<GLushort> &elements)
    {
        ifstream in(filename, ios::in);
        if (!in)
        {
            cerr << "Cannot open " << filename << endl; exit(1);
        }

        string line;
        while (getline(in, line))
        {
            if (line.substr(0,2) == "v ")
            {
                istringstream s(line.substr(2));
                glm::vec4 v; s >> v.x; s >> v.y; s >> v.z; v.w = 1.0f;
                vertices.push_back(v);
            }
            else if (line.substr(0,2) == "f ")
            {
                istringstream s(line.substr(2));
                GLushort a,b,c;
                s >> a; s >> b; s >> c;
                a--; b--; c--;
               elements.push_back(a); elements.push_back(b); elements.push_back(c);
            }
            else if (line[0] == '#')
            {
                /* ignoring this line */
            }
            else
            {
                /* ignoring this line */
            }
        }

        normals.resize(vertices.size(), glm::vec3(0.0, 0.0, 0.0));
        for (int i = 0; i < (int)elements.size(); i+=3)
        {
            GLushort ia = elements[i];
            GLushort ib = elements[i+1];
            GLushort ic = elements[i+2];
            glm::vec3 normal = glm::normalize(glm::cross(
            glm::vec3(vertices[ib]) - glm::vec3(vertices[ia]),
            glm::vec3(vertices[ic]) - glm::vec3(vertices[ia])));
            normals[ia] = normals[ib] = normals[ic] = normal;
        }
    }

    void loadTextures(){
      SDL_Surface* res_texture = IMG_Load("res_texture.png");
    	if (res_texture == NULL) {
    		cerr << "IMG_Load: " << SDL_GetError() << endl;
    		exit(1);
    	}

      GLfloat cube_texcoords[] = {
        // front
        0.0, 0.0,
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
      };

      glGenTextures(1, &texture_id);
    	glBindTexture(GL_TEXTURE_2D, texture_id);
      glBufferData(GL_ARRAY_BUFFER, sizeof(cube_texcoords), cube_texcoords, GL_STATIC_DRAW);
    	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    	glTexImage2D(GL_TEXTURE_2D, // target
    		0, // level, 0 = base, no minimap,
    		GL_RGBA, // internalformat
    		res_texture->w, // width
    		res_texture->h, // height
    		0, // border, always 0 in OpenGL ES
    		GL_RGBA, // format
    		GL_UNSIGNED_BYTE, // type
    		res_texture->pixels);
    	SDL_FreeSurface(res_texture);

      const char* attribute_name3 = "texcoord";
    	attribute_texcoord = glGetAttribLocation(program, attribute_name3);
    	if (attribute_texcoord == -1) {
    		cerr << "Could not bind attribute " << attribute_name3 << endl;
    		exit(1);
    	}
    }

    void cleanup(){
      glDeleteProgram(program);
      glDeleteBuffers(1, &vboBall);
      glDeleteBuffers(1, &paddleLeft);
      glDeleteBuffers(1, &paddleRight);
      glDeleteBuffers(1, &testmodel);
      //Activate when ready for textures
      //glDeleteTextures(1, &texture_id);
      Mix_CloseAudio();
      free(delayTime);
      free(ball);
    }

    void render(){

      // Clear Background to white;
      glClearColor(0.0, 0.0, 0.0, 1.0);
    	glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(program);

      /* For Texturing
      glActiveTexture(GL_TEXTURE0);
    	glUniform1i(uniform_mytexture, 0);
    	glBindTexture(GL_TEXTURE_2D, texture_id);
      */

      glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP4[0][0]);

      glEnableVertexAttribArray(attribute_coord3d);
      glEnableVertexAttribArray(attribute_v_color);
      glEnableVertexAttribArray(attribute_texcoord);

      glBindBuffer(GL_ARRAY_BUFFER, vboBall);
      glBufferData(GL_ARRAY_BUFFER, sizeof(triangle_attributes), triangle_attributes, GL_STATIC_DRAW);

      // Describe our vertices array to OpenGL
      glVertexAttribPointer(
    		attribute_coord3d, // attribute
    		3,                 // number of elements per vertex, here (x,y)
    		GL_FLOAT,          // the type of each element
    		GL_FALSE,          // take our values as-is
    		sizeof(struct attributes), // no extra data between each position
    		0  // pointer to the C
      );

      //Push each element in buffer_vertices to the vertex shader
      glDrawArrays(GL_TRIANGLES, 0, 6);
      //glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

      // Begin Color Buffering
  		glBindBuffer(GL_ARRAY_BUFFER, vboBallColors);

      glVertexAttribPointer(
        attribute_v_color, // attribute
        3,                 // number of elements per vertex, here (r,g,b)
        GL_FLOAT,          // the type of each element
        GL_FALSE,          // take our values as-is
        sizeof(struct attributes), // no extra data between each position
        (GLvoid*) offsetof(struct attributes, v_color) // offset of first element
      );

      glDisableVertexAttribArray(attribute_coord3d);
      glDisableVertexAttribArray(attribute_v_color);

      // Send off to SDL
      SDL_GL_SwapWindow(window);
    }

    void logic() {

      //ballControl();
      p1translateMatrix = translate(glm::mat4(1.0f), glm::vec3(-2.4f, p1VertPosition, 0.0f));
      p2translateMatrix = translate(glm::mat4(1.0f), glm::vec3(2.4f, p2VertPosition, 0.0f));
      balltranslateMatrix = translate(mat4(1.0f), vec3(ball -> xpos, ball -> ypos, 0.0f));
      paddleScaleMatrix = scale(mat4(1.0f), vec3(0.05f, 0.2f, 1.0f));
      ballScaleMatrix = scale(mat4(1.0f), vec3(0.05f, 0.05f, 1.0f));
    	// alpha 0->1->0 every 5 seconds
    	//float cur_fade = sinf(SDL_GetTicks() / 1000.0 * (2*3.14) / 5) / 2 + 0.5;
    	//glUseProgram(program);
    	//glUniform1f(uniform_fade, cur_fade);

      MVP = Projection * Model * (View * balltranslateMatrix * ballScaleMatrix);
      MVP2 = Projection * Model * (View * p1translateMatrix * paddleScaleMatrix);
      MVP3 = Projection * Model * (View * p2translateMatrix * paddleScaleMatrix);
      MVP4 = Projection * Model * View;
    }

    void getInput(){
      const Uint8* keystate = SDL_GetKeyboardState(NULL);

      GLfloat maxdistance = 1.85;

      if(keystate[SDL_SCANCODE_W] && p1VertPosition < maxdistance){p1VertPosition += 0.05;}
      if(keystate[SDL_SCANCODE_S] && p1VertPosition > maxdistance * -1.0){p1VertPosition -= 0.05;}
      if(keystate[SDL_SCANCODE_UP] && p2VertPosition < maxdistance * 1.0){p2VertPosition += 0.05;}
      if(keystate[SDL_SCANCODE_DOWN] && p2VertPosition > maxdistance * -1.0){p2VertPosition -= 0.05;}
      if(keystate[SDL_SCANCODE_ESCAPE]){runFlag = false;}

    }

    void ballControl(){
      if(p1Score > 4 || p2Score > 4){
        return;
      }
    	if(ball -> xSpeed){
    		if((ball -> ypos > 1.85f) | (ball -> ypos < -1.85f)){
    			ball -> ySpeed *= -1.0f;
    		}
    		if(ball -> xpos > 2.35f && ball -> xSpeed > 0){
    			if(ball -> xpos > 2.55f){
    				p1Score += 1;
    				ballReset();
            Mix_PlayChannel(-1, sound[1], 0);
            return;
    			} else if (ball -> ypos < p2VertPosition + 0.2f && ball -> ypos > p2VertPosition - 0.2f){
            float scuffedDirection = 0.3 * (float)(rand() % 15) / 50;
            ball -> xSpeed = (ball -> xSpeed * -1.0f) - 0.001f;
            ball -> ySpeed = scuffedDirection;
            Mix_PlayChannel(-1, sound[0], 0);
    				//fprintf(stderr, "%f\n", ball -> xSpeed);
    			} else {
    				ball -> xpos += ball -> xSpeed;
    				ball -> ypos += ball -> ySpeed;
    			}
    		} else if(ball -> xpos < -2.35f && ball -> xSpeed < 0) {
    			if(ball -> xpos < -2.55f){
    				p2Score += 1;
    				ballReset();
            Mix_PlayChannel(-1, sound[1], 0);
    			} else if (ball -> ypos < p1VertPosition + 0.2f && ball -> ypos > p1VertPosition - 0.2f){
            float scuffedDirection = 0.3 * (float)(rand() % 15) / 50;
    				ball -> xSpeed = (ball -> xSpeed * -1.0f) + 0.001f;
            ball -> ySpeed = scuffedDirection;
            Mix_PlayChannel(-1, sound[0], 0);
    				fprintf(stderr, "%f\n", ball -> xSpeed);
    			} else {
    				ball -> xpos += ball -> xSpeed;
    				ball -> ypos += ball -> ySpeed;
    			}
    		} else {
    			ball -> xpos += ball -> xSpeed;
    			ball -> ypos += ball -> ySpeed;
    		}
    	} else {
    		int scuffedDirection = rand() % 2;
    		if(scuffedDirection){
    			ball -> xSpeed = 0.03f + (0.005f * (float)(p1Score + p2Score));
    		} else {
    			ball -> xSpeed = -0.03f - (0.005f * (float)(p1Score + p2Score));
    		}
    		scuffedDirection = rand() % 10;
    		printf("%d\n", scuffedDirection);
    		ball -> ySpeed = (float)scuffedDirection / 200;
    		printf("%f\n", ball -> ySpeed);
    		if(rand() % 2){
    			ball -> ySpeed *= -1.0f;
    		}
    	}
    }

    void ballReset(){
    	if(p1Score < 5 && p2Score < 5){
    		ball -> xpos = 0;
    		ball -> ypos = 0;
    		ball -> xSpeed = 0;
    		ball -> ySpeed = 0;
        p1VertPosition = 0;
        p2VertPosition = 0;
        holdFlag = 1;
    	} else {
    		if(p1Score > 4){
    			printf("Player 1 Wins\n");
    		} else {
    			printf("Player 2 Wins\n");
    		}
    	}
    }

    void graphicsLoop(){
      runFlag = true;
      do{
        logic();
        render();
        getInput();
        SDL_PollEvent(&input);
      }while(input.type != SDL_QUIT && runFlag);
    }
};

int main(int argc, char* argv[]) {
  pongGraphics app;

  app.run();
  return 0;
}
