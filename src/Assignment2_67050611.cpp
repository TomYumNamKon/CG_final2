// Assignment 2: FUTURE ME / Orbital delivery
// Student IDs: 67050611, 67050569. Names: add to Classroom comment.
// Original OBJ assets: tools/generate_future_me.py. References: docs/FUTURE_ME.md.
// OpenGL 3.3 core: indexed OBJ geometry, GGX materials, PCF shadows, SSAO, HDR/bloom.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <algorithm>
#include "Libs/Mesh.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
namespace fs=std::filesystem;
constexpr int W=800,H=600,SHADOW=4096;
const fs::path root=fs::path(OPENGL_STARTER_SHADER_DIR).parent_path();
struct Program{
 GLuint id=0;
 Program(const char* vertex,const char* fragment){
  id=glCreateProgram();
  for(auto entry:{std::make_pair(vertex,GL_VERTEX_SHADER),std::make_pair(fragment,GL_FRAGMENT_SHADER)}){
   std::ifstream f(root/"Shaders/FutureMe"/entry.first);
   if(!f)throw std::runtime_error(std::string("Missing shader: ")+entry.first);
   std::string text((std::istreambuf_iterator<char>(f)),{});const char* p=text.c_str();
   GLuint s=glCreateShader(entry.second);glShaderSource(s,1,&p,nullptr);glCompileShader(s);
   GLint ok;glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
   if(!ok){char log[4096];glGetShaderInfoLog(s,4096,nullptr,log);glDeleteShader(s);throw std::runtime_error(log);}
   glAttachShader(id,s);glDeleteShader(s);
  }
  glLinkProgram(id);GLint ok;glGetProgramiv(id,GL_LINK_STATUS,&ok);
  if(!ok){char log[4096];glGetProgramInfoLog(id,4096,nullptr,log);throw std::runtime_error(log);}
 }
 ~Program(){glDeleteProgram(id);}
 void use(){glUseProgram(id);}
 GLint loc(const char* n){return glGetUniformLocation(id,n);}
 void integer(const char*n,int v){glUniform1i(loc(n),v);}
 void scalar(const char*n,float v){glUniform1f(loc(n),v);}
 void vec(const char*n,glm::vec3 v){glUniform3fv(loc(n),1,glm::value_ptr(v));}
 void mat(const char*n,const glm::mat4& v){glUniformMatrix4fv(loc(n),1,GL_FALSE,glm::value_ptr(v));}
};
struct Part{std::unique_ptr<Mesh> mesh;glm::vec3 color;float emission,specular;int kind;bool vehicle=true;};
GLuint loadTexture(const fs::path& path){
 int w,h,n;auto pixels=stbi_load(path.string().c_str(),&w,&h,&n,4);
 if(!pixels)throw std::runtime_error("Cannot load texture: "+path.string());
 GLuint t;glGenTextures(1,&t);glBindTexture(GL_TEXTURE_2D,t);
 glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA8,w,h,0,GL_RGBA,GL_UNSIGNED_BYTE,pixels);stbi_image_free(pixels);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);return t;
}
struct Target{
 GLuint fbo=0,tex=0,depth=0;
 Target(bool shadow,int scale=1){
  glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);
  glTexImage2D(GL_TEXTURE_2D,0,shadow?GL_DEPTH_COMPONENT24:GL_RGBA16F,shadow?SHADOW:W*scale,shadow?SHADOW:H*scale,0,shadow?GL_DEPTH_COMPONENT:GL_RGBA,GL_FLOAT,nullptr);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,shadow?GL_NEAREST:GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,shadow?GL_NEAREST:GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER,shadow?GL_DEPTH_ATTACHMENT:GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);
  if(shadow){glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);}
  else{
   glGenTextures(1,&depth);glBindTexture(GL_TEXTURE_2D,depth);
   glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,W*scale,H*scale,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
   glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,depth,0);
  }
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE)throw std::runtime_error("Framebuffer incomplete");
 }
 ~Target(){glDeleteFramebuffers(1,&fbo);glDeleteTextures(1,&tex);if(depth)glDeleteTextures(1,&depth);}
};
void capture(const fs::path& path){
 std::vector<unsigned char> pixels(W*H*3);glPixelStorei(GL_PACK_ALIGNMENT,1);
 glReadPixels(0,0,W,H,GL_RGB,GL_UNSIGNED_BYTE,pixels.data());
 stbi_flip_vertically_on_write(1);
 if(!stbi_write_png(path.string().c_str(),W,H,3,pixels.data(),W*3))throw std::runtime_error("Cannot save PNG");
 std::cout<<"Saved "<<path<<" (800 x 600)\n";
}
int main(int argc,char**argv){
 bool captureOnly=false,overlay=true,shadows=true,bloom=true,approach=false;
 fs::path output=root/"captures/Assignment2_67050611_67050569.png";
 for(int i=1;i<argc;i++){
  std::string a=argv[i];if(a=="--capture")captureOnly=true;
  else if(a=="--no-overlay")overlay=false;else if(a=="--no-shadows")shadows=false;else if(a=="--no-bloom")bloom=false;
  else if(a=="--approach")approach=true;
  else if(a=="--output"&&i+1<argc)output=argv[++i];
  else{std::cerr<<"Unknown or incomplete argument: "<<a<<'\n';return 1;}
 }
 Window window(W,H,3,3);if(window.initialise())return 1;
 glfwSetWindowTitle(window.getWindow(),"FUTURE ME | Space: fly to planet | F12 capture | H titles | B bloom | K shadows | R reset");
 if(captureOnly)glfwHideWindow(window.getWindow());
 try{
  std::cout<<"OpenGL "<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<'\n';
  Program scene("scene.vert","scene.frag"),depth("depth.vert","depth.frag"),sky("screen.vert","sky.frag"),post("screen.vert","post.frag");
  std::vector<Part> parts;std::ifstream manifest(root/"Models/FutureMe/scene.txt");
  if(!manifest)throw std::runtime_error("Missing Models/FutureMe/scene.txt; run tools/generate_future_me.py");
  std::string name;
  while(manifest>>name){Part p;p.mesh=std::make_unique<Mesh>();
   p.vehicle=name!="planet.obj"&&name!="ring.obj"&&name!="moon.obj"&&name!="stationMetal.obj"&&name!="stationLight.obj"&&name!="solar.obj";
   if(!(manifest>>p.color.r>>p.color.g>>p.color.b>>p.emission>>p.specular>>p.kind))throw std::runtime_error("Invalid scene manifest");
   if(!p.mesh->CreateMeshFromOBJ((root/"Models/FutureMe"/name).string()))throw std::runtime_error("OBJ failed: "+name);
   parts.push_back(std::move(p));
  }
  GLuint cargo=loadTexture(root/"Textures/FutureMe/cargo.png"),overlayTex=loadTexture(root/"Textures/FutureMe/overlay.png");
  Target shadow(true),hdr(false,2),finalImage(false);
  GLuint emptyVAO;glGenVertexArrays(1,&emptyVAO);
  glm::mat4 identity(1),lightVP=glm::ortho(-9.f,9.f,-9.f,9.f,1.f,40.f)*glm::lookAt(glm::vec3(-6,14,10),glm::vec3(0,0,0),glm::vec3(0,1,0));
  glm::vec3 eye(-1,5,16),target(0,1,0);
  float orbit=0,elevation=0;double oldX=0,oldY=0;bool dragging=false;
  bool prev[GLFW_KEY_LAST+1]={};
  auto pressed=[&](int key){bool down=glfwGetKey(window.getWindow(),key)==GLFW_PRESS;bool edge=down&&!prev[key];prev[key]=down;return edge;};
  int frames=0;bool flying=false;float distance=approach?4.f:0.f;double lastTime=glfwGetTime();
  while(!window.getShouldClose()){
   glfwPollEvents();if(pressed(GLFW_KEY_ESCAPE))break;
   if(pressed(GLFW_KEY_H))overlay=!overlay;if(pressed(GLFW_KEY_B))bloom=!bloom;if(pressed(GLFW_KEY_K))shadows=!shadows;
   if(pressed(GLFW_KEY_SPACE))flying=!flying;
   if(pressed(GLFW_KEY_R)){orbit=0;elevation=0;distance=0;flying=false;}
   double now=glfwGetTime();float dt=float(std::min(now-lastTime,.1));lastTime=now;
   if(flying&&!captureOnly)distance=std::min(distance+dt*.6f,7.f);
   glm::vec3 shipOffset=glm::normalize(glm::vec3(8,4.35f,-11))*distance;
   glm::mat4 shipModel=glm::translate(identity,shipOffset);
   double x,y;glfwGetCursorPos(window.getWindow(),&x,&y);
   bool drag=glfwGetMouseButton(window.getWindow(),GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
   if(drag&&dragging){orbit+=float(x-oldX)*.006f;elevation=std::clamp(elevation+float(y-oldY)*.03f,-3.f,8.f);}
   dragging=drag;oldX=x;oldY=y;
   eye=glm::vec3(-cos(orbit)+16*sin(orbit),5.f+elevation,16*cos(orbit)+sin(orbit));
   glm::mat4 view=glm::lookAt(eye,target,glm::vec3(0,1,0)),projection=glm::perspective(glm::radians(42.f),float(W)/H,.1f,100.f);
   glEnable(GL_DEPTH_TEST);glBindFramebuffer(GL_FRAMEBUFFER,shadow.fbo);glViewport(0,0,SHADOW,SHADOW);glClear(GL_DEPTH_BUFFER_BIT);
   depth.use();depth.mat("model",identity);depth.mat("lightVP",lightVP);
   for(auto& p:parts)if(p.emission<1&&p.kind!=1&&p.kind!=2){depth.mat("model",p.vehicle?shipModel:identity);p.mesh->RenderMesh();}
   glBindFramebuffer(GL_FRAMEBUFFER,hdr.fbo);glViewport(0,0,W*2,H*2);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glDisable(GL_DEPTH_TEST);sky.use();glBindVertexArray(emptyVAO);glDrawArrays(GL_TRIANGLES,0,3);glEnable(GL_DEPTH_TEST);
   scene.use();scene.mat("model",identity);scene.mat("view",view);scene.mat("projection",projection);scene.mat("lightVP",lightVP);scene.vec("eye",eye);scene.integer("shadows",shadows);
   scene.vec("shipOffset",shipOffset);
   glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,shadow.tex);scene.integer("shadowMap",0);
   glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,cargo);scene.integer("cargo",1);
   for(auto& p:parts){scene.mat("model",p.vehicle?shipModel:identity);scene.vec("baseColor",p.color);scene.scalar("emission",p.emission);scene.scalar("specular",p.specular);scene.integer("kind",p.kind);p.mesh->RenderMesh();}
   glBindFramebuffer(GL_FRAMEBUFFER,finalImage.fbo);glViewport(0,0,W,H);glDisable(GL_DEPTH_TEST);
   post.use();glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,hdr.tex);post.integer("sceneTex",0);
   glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,overlayTex);post.integer("overlayTex",1);post.integer("overlayEnabled",overlay);post.integer("bloomEnabled",bloom);
   glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D,hdr.depth);post.integer("depthTex",2);
   post.mat("projection",projection);post.mat("inverseProjection",glm::inverse(projection));
   glBindVertexArray(emptyVAO);glDrawArrays(GL_TRIANGLES,0,3);
   bool save=pressed(GLFW_KEY_F12)||(captureOnly&&frames==2);
   if(save){if(!output.parent_path().empty())fs::create_directories(output.parent_path());capture(output);}
   GLenum err=glGetError();if(err!=GL_NO_ERROR)throw std::runtime_error("OpenGL error "+std::to_string(err));
   int fw,fh;glfwGetFramebufferSize(window.getWindow(),&fw,&fh);
   glBindFramebuffer(GL_READ_FRAMEBUFFER,finalImage.fbo);glBindFramebuffer(GL_DRAW_FRAMEBUFFER,0);
   glClearColor(.005f,.008f,.015f,1);glClear(GL_COLOR_BUFFER_BIT);
   int pw=fw,ph=fw*H/W;if(ph>fh){ph=fh;pw=fh*W/H;}
   glBlitFramebuffer(0,0,W,H,(fw-pw)/2,(fh-ph)/2,(fw+pw)/2,(fh+ph)/2,GL_COLOR_BUFFER_BIT,GL_LINEAR);
   window.swapBuffers();frames++;if(captureOnly&&save)break;
  }
  glDeleteTextures(1,&cargo);glDeleteTextures(1,&overlayTex);glDeleteVertexArrays(1,&emptyVAO);
  std::cout<<"Render completed without OpenGL errors.\n";
 }catch(const std::exception&e){std::cerr<<"FUTURE ME: "<<e.what()<<'\n';return 1;}
 return 0;
}
