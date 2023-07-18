#include "platform.h"
#include <GLFW/glfw3.h>
#include <assert.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <stdio.h>
#include <string>

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

struct PlatformImpl {
  GLFWwindow *Window = nullptr;
  std::string GlslVersion;

  PlatformImpl() {

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
      assert(false);
  }

  ~PlatformImpl() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    glfwDestroyWindow(Window);
    glfwTerminate();
  }

  GLFWwindow *CreateWindow() {
    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char *glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char *glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);           // Required on Mac
#else
    // GL 3.0 + GLSL 130
    GlslVersion = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
    // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

    // Create window with graphics context
    Window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example",
                              nullptr, nullptr);
    if (!Window) {
      return nullptr;
    }

    glfwMakeContextCurrent(Window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(Window, true);
    ImGui_ImplOpenGL3_Init(GlslVersion.c_str());

    return Window;
  }

  std::optional<Frame> BeginFrame() {
    if (glfwWindowShouldClose(Window)) {
      return {};
    }

    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application, or clear/overwrite your copy of the
    // keyboard data. Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.
    glfwPollEvents();

    Frame frame;
    glfwGetFramebufferSize(Window, &frame.Width, &frame.Height);

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    return frame;
  }

  void EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we
    // save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call
    //  glfwMakeContextCurrent(window) directly)
    auto &io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      UpdateViewports([]() {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
      });
    }

    glfwSwapBuffers(Window);
  }

  void UpdateViewports(const std::function<void()> &callback) {
    GLFWwindow *backup_current_context = glfwGetCurrentContext();
    callback();
    glfwMakeContextCurrent(backup_current_context);
  }
};

//
// Platform
//
Platform::Platform() : m_impl(new PlatformImpl) {}

Platform::~Platform() { delete m_impl; }

GLFWwindow *Platform::CreateWindow() { return m_impl->CreateWindow(); }

std::optional<Frame> Platform::BeginFrame() { return m_impl->BeginFrame(); }

void Platform::EndFrame() { m_impl->EndFrame(); }

const char *Platform::GlslVersion() const {
  return m_impl->GlslVersion.c_str();
}

void Platform::UpdateViewports(const std::function<void()> &callback) {
  m_impl->UpdateViewports(callback);
}
