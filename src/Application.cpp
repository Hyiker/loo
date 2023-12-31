/**
 * Application.hpp
 * Contributors:
 *      * Arthur Sonzogni (author)
 * Licence:
 *      * MIT
 */

#include "loo/Application.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <iostream>
#include <stdexcept>

#include "glog/logging.h"
#include "loo/glError.hpp"

namespace loo {

using namespace std;

Application* Application::context = nullptr;

Application::Application(int width, int height, const std::string& title)
    : width(width), height(height), title(title) {
    initGLFW();
    // glad load
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Fail to initialize GLAD");
    }
    initImGUI();
    setContext(this);
    Quad::initGlobalQuad();
}

GLFWwindow* Application::getWindow() const {
    return window;
}

void Application::exit() {
    glfwSetWindowShouldClose(window, true);
}

float Application::getFrameTimeFromStart() const {
    return frameTime;
}

float Application::getDeltaTime() const {
    return frameTime - lastFrameTime;
}
void Application::pauseTime() {
    pauseFlag = true;
}
void Application::resumeTime() {
    pauseFlag = false;
    glfwSetTime(frameTime);
}

void Application::run() {

    // Make the window's context current
    glfwMakeContextCurrent(window);
    // uncomment to disable vsync
    glfwSwapInterval(1);

    frameTime = glfwGetTime();
    lastFrameTime = glfwGetTime();

    while (!glfwWindowShouldClose(window) &&
           !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        // set glfwPointer to current window just to make sure callback get the
        // right context
        glfwSetWindowUserPointer(getWindow(), this);
        // compute new time and delta time
        if (!pauseFlag)
            frameTime = glfwGetTime();

        // detech window related changes
        // detectWindowDimensionChange();

        // execute the frame code
        this->loop();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        this->gui();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap Front and Back buffers (double buffering)
        glfwSwapBuffers(window);

        // Pool and process events
        glfwPollEvents();
        if (!pauseFlag) {
            lastFrameTime = frameTime;
            frameCount++;
        }
    }
    beforeCleanup();
    LOG(INFO) << "Cleaning up" << endl;
    // Cleanup
    cleanup();
    afterCleanup();
}

void Application::detectWindowDimensionChange() {
    int w, h;
    glfwGetWindowSize(getWindow(), &w, &h);
    dimensionChanged = (w != width || h != height);
    if (dimensionChanged) {
        width = w;
        height = h;
        glViewport(0, 0, width, height);
    }
}

void Application::loop() {}

void Application::cleanup() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    logPossibleGLError();

    glfwDestroyWindow(window);
    glfwTerminate();
}

#ifdef __APPLE__
int Application::getFramebufferWidth() {
    int width, _;
    glfwGetFramebufferSize(getWindow(), &width, &_);
    return width;
}

int Application::getFramebufferHeight() {
    int _, height;
    glfwGetFramebufferSize(getWindow(), &_, &height);
    return height;
}
int Application::getWindowWidth() {
    return width;
}

int Application::getWindowHeight() {
    return height;
}
#else
int Application::getWidth() const {
    return width;
}

int Application::getHeight() const {
    return height;
}
#endif

float Application::getWindowRatio() {
    return float(width) / float(height);
}

bool Application::windowDimensionChanged() {
    return dimensionChanged;
}
void Application::initGLFW() {
    // initialize the GLFW library
    LOG(INFO) << "Initializing GLFW..." << endl;
    if (!glfwInit()) {
        LOG(FATAL) << "Couldn't init GLFW" << endl;
    }

    // setting the opengl version
    int major = 4;
    int minor = 6;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // create the window
    window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
    if (!window) {
        glfwTerminate();
        LOG(FATAL) << "Couldn't create GLFW window" << endl;
    }

    glfwMakeContextCurrent(window);
}
void Application::initImGUI() {
    // ImGui setup
    LOG(INFO) << "Initializing Dear ImGUI..." << endl;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    logPossibleGLError();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    (void)io;
}

bool Application::keyForward() {
    return glfwGetKey(getWindow(), GLFW_KEY_W) == GLFW_PRESS ||
           glfwGetKey(getWindow(), GLFW_KEY_UP) == GLFW_PRESS;
}
bool Application::keyBackward() {
    return glfwGetKey(getWindow(), GLFW_KEY_S) == GLFW_PRESS ||
           glfwGetKey(getWindow(), GLFW_KEY_DOWN) == GLFW_PRESS;
}
bool Application::keyLeft() {
    return glfwGetKey(getWindow(), GLFW_KEY_A) == GLFW_PRESS ||
           glfwGetKey(getWindow(), GLFW_KEY_LEFT) == GLFW_PRESS;
}
bool Application::keyRight() {
    return glfwGetKey(getWindow(), GLFW_KEY_D) == GLFW_PRESS ||
           glfwGetKey(getWindow(), GLFW_KEY_RIGHT) == GLFW_PRESS;
}
int Application::viewport[4]{};

void Application::storeViewport() {
    glGetIntegerv(GL_VIEWPORT, viewport);
}
void Application::restoreViewport() {
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void Application::beginEvent(const std::string& message) {
    glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, message.size(),
                     message.c_str());
}
void Application::endEvent() {
    glPopDebugGroup();
}
}  // namespace loo
