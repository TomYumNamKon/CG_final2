#undef GLFW_DLL

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <vector>
#include <iostream>

#include "Libs/Mesh.h"
#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "ProjectPaths.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_WINDOWS_UTF8 
#include <stb_image.h>

const GLint WIDTH = 800;
const GLint HEIGHT = 600;

std::vector<Mesh*> meshList;
std::vector<Shader*> shaderList;

const std::filesystem::path shaderDirectory =
    std::filesystem::u8path(OPENGL_STARTER_SHADER_DIR);
const std::filesystem::path textureDirectory =
    std::filesystem::u8path(OPENGL_STARTER_TEXTURE_DIR);
const std::filesystem::path modelDirectory =
    std::filesystem::u8path(OPENGL_STARTER_MODEL_DIR);

// ================= [ตัวแปรมุมกล้อง] =================
float yaw = -90.0f;
float pitch = 0.0f;

void CreateTriangle()
{
    GLfloat vertices[] =
    {   // x y z                //u v
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
         0.0f, -1.0f, 1.0f,     0.5f, 0.0f,
         1.0f, -1.0f, 0.0f,     1.0f, 0.0f,
         0.0f,  1.0f, 0.0f,     0.5f, 1.0f
    };

    unsigned int indices[] =
    {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    Mesh *obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 20, 12);
    meshList.push_back(obj1);
}

void CreateOBJ(const std::filesystem::path& modelPath)
{
    Mesh *obj = new Mesh();
    bool loaded = obj->CreateMeshFromOBJ(modelPath.u8string().c_str());

    if (!loaded)
    {
        std::cerr << "Failed to load model : " << modelPath <<std::endl;
        return;
    }

    meshList.push_back(obj);
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(
        shaderDirectory / "shader.vert",
        shaderDirectory / "shader.frag"
    );
    shaderList.push_back(shader1);

    Shader* shader2 = new Shader();
    shader2->CreateFromFiles(
        shaderDirectory / "lightShader.vert",
        shaderDirectory / "lightShader.frag"
    );
    shaderList.push_back(shader2);
}

GLuint LoadTexture(const std::filesystem::path& texturePath)
{
    int width = 0;
    int height = 0;
    int channels = 0;

    unsigned char* data = stbi_load(texturePath.u8string().c_str(),
                &width, &height, &channels, 0);

    if (data == nullptr)
    {
        std::cerr << "Failed to load texture: " << texturePath << " Reason: " << stbi_failure_reason() << "\n";
        return 0;
    }

    GLenum format = GL_RGB;

    switch (channels)
    {
        case 4:
            format = GL_RGBA;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 2:
            format = GL_RG;
            break;
        case 1:
            format = GL_RED;
            break;
        default:
            std::cerr << "Unsupported texture format\n";
            stbi_image_free(data);
            return 0;
    }

    GLuint texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);

    return texture;
}

void Cleanup()
{
    for (Mesh* mesh : meshList)
        delete mesh;
    meshList.clear();

    for (Shader* shader : shaderList)
        delete shader;
    shaderList.clear();
}

void UpdateMouseLook(Window &window)
{
    double mouseX, mouseY;
    glfwGetCursorPos(window.getWindow(), &mouseX, &mouseY);

    static double lastX = mouseX;
    static double lastY = mouseY;

    float xOffset = static_cast<float>(mouseX - lastX);
    float yOffset = static_cast<float>(mouseY - lastY);

    // update
    lastX = mouseX;
    lastY = mouseY;

    float sensitivity = 0.1f;
    xOffset *= sensitivity;
    yOffset *= sensitivity;

    yaw += xOffset;
    pitch -= yOffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

int main()
{
Window mainWindow(WIDTH, HEIGHT, 3, 3);
    if (mainWindow.initialise() != 0)
        return 1;

    CreateOBJ(modelDirectory / "DORAEMON.obj");
    CreateShaders();

    CreateOBJ(modelDirectory / "cube.obj");
    

    GLuint texture_data = LoadTexture(textureDirectory / "uvmap.png");

    if (texture_data == 0) {
        std::cerr << "Warning: Failed to load one or more textures.\n";
    }

    GLuint uniformModel = 0, uniformView = 0, uniformProjection = 0, uniformTextureData = 0;

    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(),
        0.1f,
        100.0f
    );
    // glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 100.0f);

    // ================= [กำหนดค่ากล้องและการเคลื่อนที่เริ่มต้น] =================
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    float moveSpeed = 5.0f;
    float lastTime = static_cast<float>(glfwGetTime());

    glm::vec3 lightPos = glm::vec3(5.0f,5.0f,0.0f);

    while (!mainWindow.getShouldClose())
    {
        // 1. คำนวณเวลา
        float currentTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;

        // 2. รับ Event และอัปเดตมุมมองกล้องจากเมาส์
        glfwPollEvents();
        UpdateMouseLook(mainWindow);

        // 3. คำนวณแกนทิศทางกล้องจาก yaw และ pitch ล่าสุด
        glm::vec3 cameraDirection;
        cameraDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection.y = sin(glm::radians(pitch));
        cameraDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraDirection = glm::normalize(cameraDirection);

        glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
        glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

        // 4. ตรวจสอบการกดปุ่มเพื่อขยับตำแหน่งกล้อง
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos += cameraDirection * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos -= cameraDirection * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A) == GLFW_PRESS) {
            cameraPos -= cameraRight * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D) == GLFW_PRESS) {
            cameraPos += cameraRight * moveSpeed * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS) {
            yaw = -90;
            pitch = 0;
            cameraPos = glm::vec3(0.0f,0.0f,0.0f);
        }

        lightPos.x = 1.0f + sin(glfwGetTime()) * 2.0f;
        lightPos.y = sin(glfwGetTime() / 2.0f);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shaderList[0]->UseShader();
        uniformModel = shaderList[0]->GetUniformLocation("model");
        uniformView = shaderList[0]->GetUniformLocation("view");
        uniformProjection = shaderList[0]->GetUniformLocation("projection");
        uniformTextureData = shaderList[0]->GetUniformLocation("texture_data");

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_data);
        glUniform1i(uniformTextureData, 0);

        glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);

        glUniform3fv(shaderList[0]->GetUniformLocation("lightColour"),
        1,
        (GLfloat *) &lightColour);

        glUniform3fv(shaderList[0]->GetUniformLocation("lightPos"),
        1,
        (GLfloat *) &lightPos);
        glUniform3fv(shaderList[0]->GetUniformLocation("viewPos"),
        1,
        (GLfloat *) &cameraPos);

        // 5. คำนวณ View Matrix
        glm::mat4 cameraPosMat(1.0f);
        cameraPosMat[3][0] = -cameraPos.x;
        cameraPosMat[3][1] = -cameraPos.y;
        cameraPosMat[3][2] = -cameraPos.z;

        glm::mat4 cameraRotateMat(1.0f);
        cameraRotateMat[0] = glm::vec4(cameraRight.x, cameraUp.x, -cameraDirection.x, 0.0f);
        cameraRotateMat[1] = glm::vec4(cameraRight.y, cameraUp.y, -cameraDirection.y, 0.0f);
        cameraRotateMat[2] = glm::vec4(cameraRight.z, cameraUp.z, -cameraDirection.z, 0.0f);

        glm::mat4 view = cameraRotateMat * cameraPosMat;

        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

        // ================= [ส่วนของวัตถุคงเดิม] =================
        glm::vec3 pyramidPositions[] =
        {
            glm::vec3( 0.0f,  0.0f,  -2.5f),
            glm::vec3( 2.0f,  5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f,  -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3( 2.4f, -0.4f,  -3.5f),
            glm::vec3(-1.7f,  3.0f,  -7.5f),
            glm::vec3( 1.3f, -2.0f,  -2.5f),
            glm::vec3( 1.5f,  2.0f,  -2.5f),
            glm::vec3( 1.5f,  0.2f,  -1.5f),
            glm::vec3(-1.3f,  1.0f,  -1.5f)
        };
        // วนลูปวาดพีระมิดทั้ง 10 ตำแหน่ง
        for (int i = 0; i < 10; i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, pyramidPositions[i]);
            model = glm::rotate(model, glm::radians(2.0f * i), glm::vec3(1.0f, 0.3f, 0.5f));
            model = glm::scale(model, glm::vec3(1.5f, 1.5f, 1.5f));

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            meshList[0]->RenderMesh();
        }   

        shaderList[1]->UseShader();
        uniformModel = shaderList[1]->GetUniformLocation("model");
        uniformView = shaderList[1]->GetUniformLocation("view");
        uniformProjection = shaderList[1]->GetUniformLocation("projection");

        glm::mat4 model(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

        
        glUniform3fv(shaderList[1]->GetUniformLocation("lightColour"),
        1,
        (GLfloat *) &lightColour);
        
        meshList[1]->RenderMesh();

        mainWindow.swapBuffers();
    }

    if (texture_data != 0)
        glDeleteTextures(1, &texture_data);


    // Delete OpenGL objects before the Window destructor destroys the context.
    Cleanup();
    return 0;
}