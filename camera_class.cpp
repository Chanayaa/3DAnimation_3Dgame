#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/filesystem.h>

#include <iostream>
#include <vector>
#include <cmath>

// ---------------- SETTINGS ----------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// ---------------- CAMERA ----------------
Camera camera(glm::vec3(0.0f, 0.0f, 12.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ---------------- DECLARATIONS ----------------
void framebuffer_size_callback(GLFWwindow*, int, int);
void mouse_callback(GLFWwindow*, double, double);
void scroll_callback(GLFWwindow*, double, double);
void processInput(GLFWwindow*);

void generateSuperellipsoid(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    float r, float s,
    int stacks, int slices);

void generate3DHeart(std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    int stacks, int slices);

// =================================================
// SUPERELLIPSOID
// =================================================

// s1 s1 power
float spow(float base, float exp)
{
    return copysign(pow(fabs(base), exp), base);
}

void generateSuperellipsoid(
    std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    float r, float s,
    int stacks, int slices)
{
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; i++)
    {
        float phi = -glm::half_pi<float>() +
            i * glm::pi<float>() / stacks;

        for (int j = 0; j <= slices; j++)
        {
            float theta = -glm::pi<float>() +
                j * 2 * glm::pi<float>() / slices;

            float x = r * spow(cos(phi), s) * spow(cos(theta), s);
            float y = r * spow(cos(phi), s) * spow(sin(theta), s);
            float z = r * spow(sin(phi), s);

            float u = (float)j / slices;
            float v = (float)i / stacks;

            vertices.insert(vertices.end(), { x,y,z,u,v });
        }
    }

    for (int i = 0; i < stacks; i++)
    {
        for (int j = 0; j < slices; j++)
        {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}


// =================================================
// TRUE 3D HEART SURFACE
// =================================================
void generate3DHeart(std::vector<float>& vertices,
    std::vector<unsigned int>& indices,
    int stacks, int slices)
{
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; i++)
    {
        float v = (float)i / stacks;
        float z = (v - 0.5f) * 2.0f;   // -1 to 1

        // Smooth rounded profile instead of linear
        float depthScale = sqrt(1.0f - z * z);

        for (int j = 0; j <= slices; j++)
        {
            float u = (float)j / slices * 2.0f * glm::pi<float>();

            float x2d = 16 * pow(sin(u), 3);
            float y2d = 13 * cos(u)
                - 5 * cos(2 * u)
                - 2 * cos(3 * u)
                - cos(4 * u);

            float x = x2d * 0.04f * depthScale;
            float y = y2d * 0.04f * depthScale;
            float zz = z * 0.8f;

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(zz);

            vertices.push_back((float)j / slices);
            vertices.push_back((float)i / stacks);
        }
    }

    for (int i = 0; i < stacks; i++)
    {
        for (int j = 0; j < slices; j++)
        {
            int first = i * (slices + 1) + j;
            int second = first + slices + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

// =================================================
// MAIN
// =================================================
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_CORE_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH, SCR_HEIGHT, "Final Heart", NULL, NULL);

    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glEnable(GL_DEPTH_TEST);

    Shader shader("7.4.camera.vs", "7.4.camera.fs");

    // ---------------- OUTLINE SUPERELLIPSOID ----------------
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSuperellipsoid(sphereVertices, sphereIndices, 0.10f, 1.0f, 10, 10);
    // give initial s value

	unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    //vertice update
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER,
        sphereVertices.size() * sizeof(float),
        &sphereVertices[0],
        GL_DYNAMIC_DRAW);

    //indice update
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sphereIndices.size() * sizeof(unsigned int),
        &sphereIndices[0],
        GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        sphereIndices.size() * sizeof(unsigned int),
        &sphereIndices[0],
        GL_DYNAMIC_DRAW);


    // ---------------- 3D HEART ----------------
    std::vector<float> heartVertices;
    std::vector<unsigned int> heartIndices;
    generate3DHeart(heartVertices, heartIndices, 30, 30);

    unsigned int heartVAO, heartVBO, heartEBO;
    glGenVertexArrays(1, &heartVAO);
    glGenBuffers(1, &heartVBO);
    glGenBuffers(1, &heartEBO);

    glBindVertexArray(heartVAO);

    glBindBuffer(GL_ARRAY_BUFFER, heartVBO);
    glBufferData(GL_ARRAY_BUFFER,
        heartVertices.size() * sizeof(float),
        &heartVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, heartEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        heartIndices.size() * sizeof(unsigned int),
        &heartIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
        5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // ---------------- OUTLINE POSITIONS ----------------
    std::vector<glm::vec3> outline;

    float scale = 4.0f;        // make outline bigger (move away from 3D heart)
    float minDistance = 0.4f; // spacing between superellipsoids

    glm::vec3 lastPoint;
    bool first = true;

    for (float t = 0.0f; t < glm::two_pi<float>(); t += 0.01f)
    {
        // -------- Remove ONE superellipsoid at top center dip --------
        if (fabs(t - glm::pi<float>()) < 0.08f)
            continue;

        // -------- Parametric heart equation --------
        float x = 16 * pow(sin(t), 3);
        float y = 13 * cos(t)
            - 5 * cos(2 * t)
            - 2 * cos(3 * t)
            - cos(4 * t);

        // -------- Scale up --------
        x *= 0.05f * scale;
        y *= 0.05f * scale;

        glm::vec3 current(x, y, 0.0f);

        // -------- Distance-based spacing --------
        if (first)
        {
            outline.push_back(current);
            lastPoint = current;
            first = false;
        }
        else
        {
            if (glm::distance(current, lastPoint) >= minDistance)
            {
                outline.push_back(current);
                lastPoint = current;
            }
        }
    }


    // ---------------- TEXTURES ----------------
    unsigned int texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(
        FileSystem::getPath("resources/textures/red.jpg").c_str(),
        &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, format,
            width, height, 0,
            format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    data = stbi_load(
        FileSystem::getPath("resources/textures/red.jpg").c_str(),
        &width, &height, &nrChannels, 0);

    if (data)
    {
        GLenum format;
        if (nrChannels == 1)
            format = GL_RED;
        else if (nrChannels == 3)
            format = GL_RGB;
        else if (nrChannels == 4)
            format = GL_RGBA;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, format,
            width, height, 0,
            format, GL_UNSIGNED_BYTE, data);

        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);

    shader.use();
    shader.setInt("texture1", 0);
    shader.setInt("texture2", 1);

    float shapeParam = 0.0f;
    bool increasing = true;
    float shapeTimer = 0.0f;

    // ================== RENDER LOOP ==================
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        /*glClearColor(0.05f, 0.0f, 0.1f, 1.0f);*/
        glClearColor(1.0f, 0.816f, 0.816f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        shader.use();

        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            (float)SCR_WIDTH / SCR_HEIGHT,
            0.1f, 100.0f);

        glm::mat4 view = camera.GetViewMatrix();

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // ----- Draw Outline (STATIC) -----
        // // accumulate time
        shapeTimer += deltaTime;

        // every 1 second
        if (shapeTimer >= 0.1f)
        {
            shapeTimer = 0.0f;

            if (increasing)
            {
                shapeParam += 0.1f;
                if (shapeParam >= 2.5f)
                {
                    shapeParam = 2.5f;
                    increasing = false;
                }
            }
            else
            {
                shapeParam -= 0.1f;
                if (shapeParam <= 0.1f)
                {
                    shapeParam = 0.1f;
                    increasing = true;
                }
            }
        }

        // Regenerate animated superellipsoid
        generateSuperellipsoid(
            sphereVertices,
            sphereIndices,
            0.08f,              // radius
            shapeParam,         // s parameter
            15, 30);

        // ===== Update GPU buffer =====
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
        glBufferData(GL_ARRAY_BUFFER,
            sphereVertices.size() * sizeof(float),
            &sphereVertices[0],
            GL_DYNAMIC_DRAW);

        // ===== Draw Outline =====
        shader.setInt("objectType", 0);

        for (auto& pos : outline)
        {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), pos);
            shader.setMat4("model", model);
            //glDrawArrays(GL_TRIANGLE_STRIP, 0, sphereVertices.size() / 5);
            glDrawElements(GL_TRIANGLES,
                sphereIndices.size(),
                GL_UNSIGNED_INT,
                0);

        }

        // ----- Draw 3D Heart (ROTATE Y) -----
        shader.setInt("objectType", 1);
        glBindVertexArray(heartVAO);
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model,
            (float)glfwGetTime(),
            glm::vec3(0, 1, 0));

        shader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES,
            heartIndices.size(),
            GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// =================================================
// CAMERA FUNCTIONS (WASD FIXED)
// =================================================
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void framebuffer_size_callback(GLFWwindow*, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow*, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow*, double, double yoffset)
{
    camera.ProcessMouseScroll((float)yoffset);
}