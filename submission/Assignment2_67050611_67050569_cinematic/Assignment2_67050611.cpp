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
#ifndef MESH____H
#define MESH____H

#include <string>
#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <GL/glew.h>


// Interleaved vertex: position (3), normal (3), uv (2)
struct VertexPNU 
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 uv{};
};

class Mesh 
{
    public:
        Mesh();
        ~Mesh();

        // Create from client-provided interleaved vertex array (8 floats per vertex: P(3) N(3) UV(2))
        // Common in many tutorials; kept for compatibility.
        void CreateMesh(const GLfloat* vertices,
                const unsigned int* indices,
                unsigned int numOfFloats,
                unsigned int numOfIndices);

        // Create from interleaved vector of VertexPNU (preferred)
        void CreateMesh(const std::vector<VertexPNU>& vertices,
        const std::vector<unsigned int>& indices);

        // Load from .obj (supports v, vt, vn; faces in v/vt/vn, v//vn, v/vt, or v)
        // Handles negative indices; triangulates n-gons via fan.
        // flipV = true to flip the V coordinate (uv.y = 1 - uv.y)
        bool CreateMeshFromOBJ(const std::string& path, bool flipV = false);

        void RenderMesh() const;
        void ClearMesh();

        // Accessors
        inline GLsizei GetIndexCount() const { return indexCount; }
        inline GLuint GetVAO() const { return VAO; }

    private:
        // GL resources
        GLuint VAO{0};
        GLuint VBO{0};
        GLuint IBO{0};

        GLsizei indexCount{0};

        // Internal helper used by OBJ loader
        void uploadInterleaved_(const std::vector<VertexPNU>& vertices,
        const std::vector<unsigned int>& indices);
};

#endif


#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <cstring>

// Lock the attribute locations we’ll use everywhere
static constexpr GLuint ATTR_POS = 0;
static constexpr GLuint ATTR_UV  = 1;  // << UV at 1
static constexpr GLuint ATTR_NRM = 2;  // << normal at 2

// ----------------------------
// Mesh: lifecycle
// ----------------------------
Mesh::Mesh() {}

Mesh::~Mesh() { ClearMesh(); }

// ----------------------------
// Create from raw arrays (8 floats per vertex)
// ----------------------------
void Mesh::CreateMesh(const GLfloat* vertices,
                    const unsigned int* indices,
                    unsigned int numOfFloats,
                    unsigned int numOfIndices) 
{
    // Build vectors and forward to preferred path
    std::vector<VertexPNU> vtx;

    if (numOfFloats % 8 == 0) 
    {
        // Layout: P(3) N(3) UV(2)
        unsigned int numVerts = numOfFloats / 8;
        vtx.reserve(numVerts);
        for (unsigned int i = 0; i < numVerts; ++i) 
        {
            const GLfloat* base = vertices + i * 8;
            VertexPNU v{};
            v.position = { base[0], base[1], base[2] };
            v.normal   = { base[3], base[4], base[5] };
            v.uv       = { base[6], base[7] };
            vtx.push_back(v);
        }
    }
    else if (numOfFloats % 5 == 0) 
    {
        // Layout: P(3) UV(2) — normals default to (0,0,1)
        unsigned int numVerts = numOfFloats / 5;
        vtx.reserve(numVerts);
        for (unsigned int i = 0; i < numVerts; ++i) 
        {
            const GLfloat* base = vertices + i * 5;
            VertexPNU v{};
            v.position = { base[0], base[1], base[2] };
            v.normal   = { 0.0f, 0.0f, 1.0f };
            v.uv       = { base[3], base[4] };
            vtx.push_back(v);
        }
    }
    else 
    {
        std::cerr << "[Mesh] Error: unsupported vertex float count " 
                  << numOfFloats << "\\n";
        return;
    }

    std::vector<unsigned int> idx(indices, indices + numOfIndices);
    CreateMesh(vtx, idx);
}

// ----------------------------
// Create from std::vector (preferred)
// ----------------------------
void Mesh::CreateMesh(const std::vector<VertexPNU>& vertices,
                    const std::vector<unsigned int>& indices) 
{
    uploadInterleaved_(vertices, indices);
}

// ----------------------------
// Upload helper (interleaved VBO + EBO)
// ----------------------------
void Mesh::uploadInterleaved_(const std::vector<VertexPNU>& vertices,
                            const std::vector<unsigned int>& indices) 
{
    // Clean previous
    ClearMesh();

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(vertices.size() * sizeof(VertexPNU)),
                vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                indices.data(), GL_STATIC_DRAW);

    const GLsizei stride = static_cast<GLsizei>(sizeof(VertexPNU));

    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(VertexPNU,position)); glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(VertexPNU,uv));       glEnableVertexAttribArray(1);
    glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,stride,(void*)offsetof(VertexPNU,normal));   glEnableVertexAttribArray(2);


    indexCount = static_cast<GLsizei>(indices.size());

    // unbind VAO (EBO remains associated with VAO)
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// ----------------------------
// .OBJ loader (robust tuples + dedup)
// ----------------------------
namespace 
{
    struct Triple 
    { 
        int v, vt, vn;  // already fixed: 0-based or -1
        bool operator==(const Triple& o) const { return v==o.v && vt==o.vt && vn==o.vn; }
    };
    struct TripleHash 
    {
        size_t operator()(const Triple& t) const 
        {
            return size_t(t.v+1)*size_t(73856093) ^ size_t(t.vt+1)*size_t(19349663) ^ size_t(t.vn+1)*size_t(83492791);
        }
    };


    // Convert OBJ 1-based (and negative) index to 0-based
    inline int fixIndex(int idx, int count) 
    {
        return (idx > 0) ? (idx - 1) : (count + idx); // negative index: -1 = last
    }
}

bool Mesh::CreateMeshFromOBJ(const std::string& path, bool flipV) 
{
    std::ifstream in(path);
    if (!in) 
    {
        std::cerr << "[Mesh] Failed to open OBJ: " << path << "\n";
        return false;
    }

    std::vector<glm::vec3> V; V.reserve(1000);
    std::vector<glm::vec2> VT; VT.reserve(1000);
    std::vector<glm::vec3> VN; VN.reserve(1000);

    std::vector<VertexPNU> interleaved; interleaved.reserve(2000);
    std::vector<unsigned int> indices; indices.reserve(6000);
    std::unordered_map<Triple, unsigned int, TripleHash> dedup;

    std::string line;
    while (std::getline(in, line)) 
    {
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        std::istringstream ss(line);
        std::string tag; ss >> tag;
        if (tag == "v") 
        {
            glm::vec3 p{}; ss >> p.x >> p.y >> p.z; V.push_back(p);
        } 
        else if (tag == "vt") 
        {
            glm::vec2 t{}; ss >> t.x >> t.y; if (flipV) t.y = 1.0f - t.y; VT.push_back(t);
        } 
        else if (tag == "vn") 
        {
            glm::vec3 n{}; ss >> n.x >> n.y >> n.z; VN.push_back(n);
        } 
        else if (tag == "f") 
        {
            // collect the rest of tokens for this face
            std::vector<std::string> fverts; fverts.reserve(8);
            std::string tok;
            while (ss >> tok) { fverts.push_back(tok); }
            if (fverts.size() < 3) continue; // not a valid face

            auto parseOne = [&](const std::string& s, int& iv, int& ivt, int& ivn) -> bool 
            {
                iv = ivt = ivn = -1;
                int parts[3] = {0,0,0}, p = 0;
                std::string cur;
                for (size_t i=0;i<=s.size();++i) 
                {
                    if (i==s.size() || s[i]=='/') 
                    {
                        if (!cur.empty()) { try { parts[p] = std::stoi(cur); } catch (...) { parts[p] = 0; } }
                        cur.clear(); if (i!=s.size() && p<2) ++p;
                    } 
                    else cur.push_back(s[i]);
                }
                auto fixIndex = [](int idx, int count){ if (idx>0) return idx-1; if (idx<0) return count+idx; return -1; };
                iv  = fixIndex(parts[0], (int)V.size());
                ivt = (p>=1) ? fixIndex(parts[1], (int)VT.size()) : -1;
                ivn = (p>=2) ? fixIndex(parts[2], (int)VN.size()) : -1;
                return iv >= 0 && iv < (int)V.size()
                    && (parts[1] == 0 || (ivt >= 0 && ivt < (int)VT.size()))
                    && (parts[2] == 0 || (ivn >= 0 && ivn < (int)VN.size()));
            };

            auto addFaceVertex = [&](int iv, int ivt, int ivn)
            {
                Triple key{iv, ivt, ivn};
                auto it = dedup.find(key);
                if (it != dedup.end()) { indices.push_back(it->second); return; }

                VertexPNU out{};
                out.position = V[iv];
                if (ivt >= 0) out.uv = VT[ivt];
                if (ivn >= 0) out.normal = VN[ivn];

                unsigned int id = (unsigned)interleaved.size();
                interleaved.push_back(out);
                dedup.emplace(key, id);
                indices.push_back(id);
            };

            // Triangulate via fan: (v0, vi, vi+1)
            // First, parse all vertices of this polygon to a temp list of indices
            struct FaceV { int v, vt, vn; };
            std::vector<FaceV> fv; fv.reserve(fverts.size());
            for (const auto& s : fverts) 
            {
                int iv=0, ivt=0, ivn=0;
                if (!parseOne(s, iv, ivt, ivn))
                {
                    std::cerr << "[Mesh] Invalid face index in " << path << "\n";
                    return false;
                }
                fv.push_back({iv, ivt, ivn});
            }
            for (size_t i = 1; i + 1 < fv.size(); ++i) 
            {
                // triangle: fv[0], fv[i], fv[i+1]
                addFaceVertex(fv[0].v, fv[0].vt, fv[0].vn);
                addFaceVertex(fv[i].v, fv[i].vt, fv[i].vn);
                addFaceVertex(fv[i+1].v, fv[i+1].vt, fv[i+1].vn);
            }
        }
    }

    if (interleaved.empty() || indices.empty()) 
    {
        std::cerr << "[Mesh] OBJ produced no geometry: " << path << "\n";
        return false;
    }

    uploadInterleaved_(interleaved, indices);
    return true;
}

// ----------------------------
// Render
// ----------------------------
void Mesh::RenderMesh() const 
{
    if (VAO == 0 || indexCount == 0) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

// ----------------------------
// Clear GL resources
// ----------------------------
void Mesh::ClearMesh() 
{
    if (IBO != 0) 
    {
        glDeleteBuffers(1, &IBO);
        IBO = 0;
    }
    if (VBO != 0) 
    {
        glDeleteBuffers(1, &VBO);
        VBO = 0;
    }
    if (VAO != 0) 
    {
        glDeleteVertexArrays(1, &VAO);
        VAO = 0;
    }
    indexCount = 0;
}

#ifndef WINDOW____H
#define WINDOW____H

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
    Window();
    Window(GLint windowWidth, GLint windowHeight, GLint majorVersion, GLint minorVersion);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    int initialise();

    GLint getBufferWidth() const { return bufferWidth; }
    GLint getBufferHeight() const { return bufferHeight; }

    bool getShouldClose() const
    {
        return mainWindow == nullptr || glfwWindowShouldClose(mainWindow);
    }

    void swapBuffers()
    {
        if (mainWindow != nullptr)
            glfwSwapBuffers(mainWindow);
    }

    GLFWwindow* getWindow() const { return mainWindow; }

private:
    GLFWwindow* mainWindow = nullptr;
    GLint glfwMajorVersion = 3;
    GLint glfwMinorVersion = 3;
    GLint width = 800;
    GLint height = 600;
    GLint bufferWidth = 0;
    GLint bufferHeight = 0;
};

#endif



#include <cstdio>

Window::Window() = default;

Window::Window(GLint windowWidth, GLint windowHeight, GLint majorVersion, GLint minorVersion)
    : glfwMajorVersion(majorVersion),
      glfwMinorVersion(minorVersion),
      width(windowWidth),
      height(windowHeight)
{
}

Window::~Window()
{
    if (mainWindow != nullptr)
    {
        glfwDestroyWindow(mainWindow);
        mainWindow = nullptr;
    }

    glfwTerminate();
}

int Window::initialise()
{
    if (!glfwInit())
    {
        std::fprintf(stderr, "GLFW initialisation failed.\n");
        return 1;
    }

    // Request a modern OpenGL core context. The starter uses OpenGL 3.3.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, glfwMajorVersion);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glfwMinorVersion);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    // Required by macOS for OpenGL 3.2+ core contexts.
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    mainWindow = glfwCreateWindow(width, height, "OpenGL Starter", nullptr, nullptr);

    if (mainWindow == nullptr)
    {
        std::fprintf(stderr, "GLFW window creation failed.\n");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(mainWindow);
    glfwSwapInterval(1);

    // GLEW must be initialised after an OpenGL context becomes current.
    glewExperimental = GL_TRUE;
    const GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK)
    {
        std::fprintf(stderr, "GLEW initialisation failed: %s\n", glewGetErrorString(glewStatus));
        glfwDestroyWindow(mainWindow);
        mainWindow = nullptr;
        glfwTerminate();
        return 1;
    }

    // Some GLEW/core-profile combinations may leave a harmless GL_INVALID_ENUM.
    glGetError();

    glEnable(GL_DEPTH_TEST);

    glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);
    glViewport(0, 0, bufferWidth, bufferHeight);

    // Keep the viewport correct on window resize and high-DPI displays.
    glfwSetFramebufferSizeCallback(mainWindow, [](GLFWwindow*, int newWidth, int newHeight)
    {
        glViewport(0, 0, newWidth, newHeight);
    });

    return 0;
}


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
namespace fs=std::filesystem;
constexpr int W=800,H=600,SHADOW=4096;
const fs::path root=fs::path(__FILE__).parent_path();
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
 glGenerateMipmap(GL_TEXTURE_2D);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
 glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);return t;
}
struct Target{
 GLuint fbo=0,tex=0,depth=0;
 Target(bool shadow,int scale=1,int divisor=1){
  glGenFramebuffers(1,&fbo);glBindFramebuffer(GL_FRAMEBUFFER,fbo);
  glGenTextures(1,&tex);glBindTexture(GL_TEXTURE_2D,tex);
  glTexImage2D(GL_TEXTURE_2D,0,shadow?GL_DEPTH_COMPONENT24:GL_RGBA16F,shadow?SHADOW:W*scale/divisor,shadow?SHADOW:H*scale/divisor,0,shadow?GL_DEPTH_COMPONENT:GL_RGBA,GL_FLOAT,nullptr);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,shadow?GL_NEAREST:GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,shadow?GL_NEAREST:GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER,shadow?GL_DEPTH_ATTACHMENT:GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,tex,0);
  if(shadow){glDrawBuffer(GL_NONE);glReadBuffer(GL_NONE);}
  else{
   glGenTextures(1,&depth);glBindTexture(GL_TEXTURE_2D,depth);
   glTexImage2D(GL_TEXTURE_2D,0,GL_DEPTH_COMPONENT24,W*scale/divisor,H*scale/divisor,0,GL_DEPTH_COMPONENT,GL_FLOAT,nullptr);
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
 bool captureOnly=false,overlay=false,shadows=true,bloom=true,approach=false;
 fs::path output=root/"captures/Assignment2_67050611_67050569.png";
 for(int i=1;i<argc;i++){
  std::string a=argv[i];if(a=="--capture")captureOnly=true;
  else if(a=="--no-overlay")overlay=false;else if(a=="--no-shadows")shadows=false;else if(a=="--no-bloom")bloom=false;
  else if(a=="--approach")approach=true;
  else if(a=="--overlay")overlay=true;
  else if(a=="--output"&&i+1<argc)output=argv[++i];
  else{std::cerr<<"Unknown or incomplete argument: "<<a<<'\n';return 1;}
 }
 Window window(W,H,3,3);if(window.initialise())return 1;
 glfwSetWindowTitle(window.getWindow(),"FUTURE ME | Space: fly to planet | F12 capture | H titles | B bloom | K shadows | R reset");
 if(captureOnly)glfwHideWindow(window.getWindow());
 try{
  std::cout<<"OpenGL "<<glGetString(GL_VERSION)<<" / "<<glGetString(GL_RENDERER)<<'\n';
  Program scene("scene.vert","scene.frag"),depth("depth.vert","depth.frag"),sky("screen.vert","sky.frag"),post("screen.vert","post.frag"),blur("screen.vert","bloom.frag");
  std::vector<Part> parts;std::ifstream manifest(root/"Models/FutureMe/scene.txt");
  if(!manifest)throw std::runtime_error("Missing Models/FutureMe/scene.txt; run tools/generate_future_me.py");
  std::string name;
  while(manifest>>name){Part p;p.mesh=std::make_unique<Mesh>();
   p.vehicle=name!="planet.obj"&&name!="ring.obj"&&name!="moon.obj"&&name!="rock.obj"&&name!="atmosphere.obj";
   if(!(manifest>>p.color.r>>p.color.g>>p.color.b>>p.emission>>p.specular>>p.kind))throw std::runtime_error("Invalid scene manifest");
   if(!p.mesh->CreateMeshFromOBJ((root/"Models/FutureMe"/name).string()))throw std::runtime_error("OBJ failed: "+name);
   parts.push_back(std::move(p));
  }
  GLuint cargo=loadTexture(root/"Textures/FutureMe/cargo.png"),overlayTex=loadTexture(root/"Textures/FutureMe/overlay.png"),grabLogo=loadTexture(root/"Textures/FutureMe/grab-official.png");
  GLuint clouds=loadTexture(root/"Textures/FutureMe/planet-clouds.jpg");
  glBindTexture(GL_TEXTURE_2D,clouds);glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
  Target shadow(true),hdr(false,2),finalImage(false),bloomA(false,1,4),bloomB(false,1,4);
  GLuint emptyVAO;glGenVertexArrays(1,&emptyVAO);
  const glm::vec3 sunDirection=glm::normalize(glm::vec3(.2f,.12f,-1.f));
  glm::mat4 identity(1),lightVP=glm::ortho(-11.f,11.f,-11.f,11.f,1.f,75.f)*glm::lookAt(sunDirection*35.f,glm::vec3(0,0,0),glm::vec3(0,1,0));
  glm::vec3 eye(-6,4.4f,14),target(1,1,0);
  glm::vec3 forward=glm::normalize(glm::vec3(22,4,-10));
  glm::vec3 side=glm::normalize(glm::vec3(-forward.z,0,forward.x));
  glm::mat4 shipBasis(1);shipBasis[0]=glm::vec4(forward*1.15f,0);shipBasis[1]=glm::vec4(glm::cross(side,forward)*1.15f,0);shipBasis[2]=glm::vec4(side*1.15f,0);shipBasis[3]=glm::vec4(-.5f,.7f,2,1);
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
   glm::vec3 shipOffset=forward*distance;
   glm::mat4 shipModel=glm::translate(identity,shipOffset);
   double x,y;glfwGetCursorPos(window.getWindow(),&x,&y);
   bool drag=glfwGetMouseButton(window.getWindow(),GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS;
   if(drag&&dragging){orbit+=float(x-oldX)*.006f;elevation=std::clamp(elevation+float(y-oldY)*.03f,-3.f,8.f);}
   dragging=drag;oldX=x;oldY=y;
   eye=glm::vec3(-6*cos(orbit)+14*sin(orbit),4.4f+elevation,14*cos(orbit)+6*sin(orbit));
   glm::mat4 view=glm::lookAt(eye,target,glm::vec3(0,1,0)),projection=glm::perspective(glm::radians(42.f),float(W)/H,.1f,100.f);
   glEnable(GL_DEPTH_TEST);glBindFramebuffer(GL_FRAMEBUFFER,shadow.fbo);glViewport(0,0,SHADOW,SHADOW);glClear(GL_DEPTH_BUFFER_BIT);
   depth.use();depth.mat("model",identity);depth.mat("lightVP",lightVP);
   for(auto& p:parts)if(p.emission<1&&p.kind!=1&&p.kind!=2&&p.kind!=14){depth.mat("model",p.vehicle?shipModel:identity);p.mesh->RenderMesh();}
   glBindFramebuffer(GL_FRAMEBUFFER,hdr.fbo);glViewport(0,0,W*2,H*2);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
   glDisable(GL_DEPTH_TEST);sky.use();sky.mat("inverseView",glm::inverse(view));sky.mat("inverseProjection",glm::inverse(projection));
   sky.vec("sunDirection",sunDirection);glBindVertexArray(emptyVAO);glDrawArrays(GL_TRIANGLES,0,3);glEnable(GL_DEPTH_TEST);
   scene.use();scene.mat("model",identity);scene.mat("view",view);scene.mat("projection",projection);scene.mat("lightVP",lightVP);scene.vec("eye",eye);scene.integer("shadows",shadows);
   scene.vec("shipOffset",shipOffset);
   scene.vec("sunDirection",sunDirection);
   glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,shadow.tex);scene.integer("shadowMap",0);
   glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,cargo);scene.integer("cargo",1);
   glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D,grabLogo);scene.integer("grabLogo",2);
   glActiveTexture(GL_TEXTURE4);glBindTexture(GL_TEXTURE_2D,clouds);scene.integer("cloudMap",4);
   for(auto& p:parts)if(p.kind!=14){scene.mat("model",p.vehicle?shipModel:identity);scene.vec("baseColor",p.color);scene.scalar("emission",p.emission);scene.scalar("specular",p.specular);scene.integer("kind",p.kind);p.mesh->RenderMesh();}
   glEnable(GL_BLEND);glBlendFunc(GL_SRC_ALPHA,GL_ONE);glDepthMask(GL_FALSE);
   for(auto& p:parts)if(p.kind==14){scene.mat("model",identity);scene.integer("kind",14);p.mesh->RenderMesh();}
   glDepthMask(GL_TRUE);glDisable(GL_BLEND);
   // HDR highlight extraction and separable Gaussian diffusion at quarter size.
   glDisable(GL_DEPTH_TEST);glViewport(0,0,W/4,H/4);blur.use();blur.integer("source",0);
   glBindVertexArray(emptyVAO);glActiveTexture(GL_TEXTURE0);
   glBindFramebuffer(GL_FRAMEBUFFER,bloomA.fbo);glBindTexture(GL_TEXTURE_2D,hdr.tex);blur.integer("mode",0);glDrawArrays(GL_TRIANGLES,0,3);
   for(int pass=0;pass<8;pass++){
    glBindFramebuffer(GL_FRAMEBUFFER,pass%2==0?bloomB.fbo:bloomA.fbo);
    glBindTexture(GL_TEXTURE_2D,pass%2==0?bloomA.tex:bloomB.tex);blur.integer("mode",pass%2+1);glDrawArrays(GL_TRIANGLES,0,3);
   }
   glBindFramebuffer(GL_FRAMEBUFFER,finalImage.fbo);glViewport(0,0,W,H);glDisable(GL_DEPTH_TEST);
   post.use();glActiveTexture(GL_TEXTURE0);glBindTexture(GL_TEXTURE_2D,hdr.tex);post.integer("sceneTex",0);
   glActiveTexture(GL_TEXTURE1);glBindTexture(GL_TEXTURE_2D,overlayTex);post.integer("overlayTex",1);post.integer("overlayEnabled",overlay);post.integer("bloomEnabled",bloom);
   glActiveTexture(GL_TEXTURE2);glBindTexture(GL_TEXTURE_2D,hdr.depth);post.integer("depthTex",2);
   glActiveTexture(GL_TEXTURE3);glBindTexture(GL_TEXTURE_2D,bloomA.tex);post.integer("bloomTex",3);
   post.mat("projection",projection);post.mat("inverseProjection",glm::inverse(projection));
   post.mat("inverseView",glm::inverse(view));post.mat("inverseShip",glm::inverse(shipModel*shipBasis));
   post.scalar("time",captureOnly?0.f:float(now));
   glm::vec4 sunClip=projection*view*glm::vec4(eye+sunDirection*80.f,1);
   post.vec("sunScreen",glm::vec3(sunClip.x/sunClip.w*.5f+.5f,sunClip.y/sunClip.w*.5f+.5f,sunClip.w));
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
  glDeleteTextures(1,&cargo);glDeleteTextures(1,&overlayTex);glDeleteTextures(1,&grabLogo);glDeleteTextures(1,&clouds);glDeleteVertexArrays(1,&emptyVAO);
  std::cout<<"Render completed without OpenGL errors.\n";
 }catch(const std::exception&e){std::cerr<<"FUTURE ME: "<<e.what()<<'\n';return 1;}
 return 0;
}
