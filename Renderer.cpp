//
// Created by Nikita Kruk on 01.03.20.
//

#include "Renderer.hpp"

#include <iostream>
#include <sstream>
#include <list>
#include <numeric> // std::iota
#include <iomanip> // std::setprecision
#include <fstream>
#include <map>
#include <cmath>

void Jet2Rgb(float j, float &r, float &g, float &b);
void Hsv2Rgb(float h, float s, float v, float &r, float &g, float &b);

bool Renderer::stop_flag_ = true;
bool Renderer::pause_flag_ = true;
int Renderer::frame_speed_ = 1; // 1 - the basic frame rate
GLfloat Renderer::x_rot_ = 0.47f;
GLfloat Renderer::y_rot_ = -0.15f;
GLfloat Renderer::z_rot_ = -0.46f;
glm::vec3 Renderer::camera_pos_ = glm::vec3(0.0f, 0.0f, 2.2f);
glm::vec3 Renderer::camera_front_ = glm::normalize(glm::vec3(0.0f, 0.0f, -5.0f));
glm::vec3 Renderer::camera_up_ = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Renderer::light_pos_ = glm::vec3(1.0f, 0.0f, 0.0f);
float Renderer::delta_time_ = 0.0f;  // Time between current frame and last frame
float Renderer::last_frame_ = 0.0f; // Time of last frame
float Renderer::last_x_ = 512.0f;
float Renderer::last_y_ = 512.0f;
GLfloat Renderer::yaw_ = 0.0f;
GLfloat Renderer::pitch_ = 0.0f;
bool Renderer::first_mouse_ = true;
float Renderer::fov_ = 45.0f;
int Renderer::t_start_ = 0;
bool Renderer::show_time_ = true;
Real Renderer::time_stamp_to_show_ = 0.0f;
int Renderer::number_of_color_components_ = 3;
ParticleRepresentationType Renderer::particle_representation_type_ = ParticleRepresentationType::kSphere;

Renderer::Renderer(Model *model) :
    model_(model),
    ft_(),
    face_(),
    sphere_template_()
{
  std::cout << "Renderer Created" << std::endl;
  model_->SkipTimeUnits(t_start_, 1.0);
}

Renderer::~Renderer()
{
  std::cout << "Renderer Deleted" << std::endl;
}

void Renderer::Start()
{
  GLFWwindow *window;

  glfwSetErrorCallback(Renderer::ErrorCallback);

  if (!glfwInit())
  {
    std::cerr << "Initialization of GLFW failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_SAMPLES, 4); // MSAA

  window = glfwCreateWindow(1024, 1024, "Particle Dynamics", NULL, NULL);
  if (!window)
  {
    glfwTerminate();
    std::cerr << "Window opening failure" << std::endl;
    exit(EXIT_FAILURE);
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetKeyCallback(window, Renderer::KeyCallback);

  int major, minor, rev;
  major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
  minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
  rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
  std::cout << "OpenGL - " << major << "." << minor << "." << rev << std::endl;

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    std::cerr << "GLEW initialization failure" << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // contexts for border, colloidal particles, solvent particles, text
  GLuint vao[4] = {0};
  GLuint vbo[4] = {0};
  glGenVertexArrays(4, &vao[0]);
  glGenBuffers(4, &vbo[0]);

  GLuint shader_program[4] = {0};
  InitShaders(shader_program);

  RenewSystemState();
  while (!glfwWindowShouldClose(window))
  {
//    glUseProgram(shader_program[1]); // shader parameters must be initialized when the respective shader is active
//    SetShaderParameter(shader_program[1], x_shift_, "x_shift");
//    SetShaderParameter(shader_program[1], y_shift_, "y_shift");
//    SetShaderParameter(shader_program[1], z_scale_, "z_scale");
    DisplayFunc(window, vao, vbo, shader_program);

//		if (!stop_flag)
//    if (take_screenshot_flag_)
//    {
//      int width, height;
//      glfwGetFramebufferSize(window, &width, &height);
//			glfwGetWindowSize(window, &width, &height);
//			glReadBuffer(GL_BACK);
//			GLubyte *pixels = NULL;
//			TakeScreenshotPpm(width, height, screenshot_count++);
//      TakeScreenshotPng(width, height, screenshot_count_++);
//			free(pixels);
//      take_screenshot_flag_ = false;
//    }
//    if (t >= 100.0f)
//    {
//      glfwSetWindowShouldClose(window, GL_TRUE);
//    }

    ReadNewState();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  FinFunc();
  glfwDestroyWindow(window);
  glfwTerminate();
}

void Renderer::ErrorCallback(int error, const char *description)
{
  std::cerr << description << std::endl;
}

void Renderer::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  float camera_speed = 2.0f * delta_time_; // adjust accordingly

  if (GLFW_PRESS == action)
  {
    switch (key)
    {
      case GLFW_KEY_ESCAPE :
//			case GLFW_PRESS:
        glfwSetWindowShouldClose(window, GL_TRUE);
        break;

      case GLFW_KEY_R : stop_flag_ = !stop_flag_;
        break;

      case GLFW_KEY_P :
        if (stop_flag_)
        {
          pause_flag_ = !pause_flag_;
        }
        break;

//      case GLFW_KEY_I : take_screenshot_flag_ = true;
//        break;

      case GLFW_KEY_O : ++frame_speed_;
        break;

      case GLFW_KEY_L :
        if (frame_speed_ > 1)
        {
          --frame_speed_;
        }
        break;

      case GLFW_KEY_T : show_time_ = !show_time_;
        break;

      case GLFW_KEY_LEFT : z_rot_ -= 0.1f;
        break;

      case GLFW_KEY_RIGHT : z_rot_ += 0.1f;
        break;

      case GLFW_KEY_UP : x_rot_ -= 0.1f;
        break;

      case GLFW_KEY_DOWN : x_rot_ += 0.1f;
        break;

      case GLFW_KEY_PAGE_UP : y_rot_ -= 0.1f;
        break;

      case GLFW_KEY_PAGE_DOWN : y_rot_ += 0.1f;
        break;

      case GLFW_KEY_W : camera_pos_ += camera_speed * camera_front_;
        break;

      case GLFW_KEY_S : camera_pos_ -= camera_speed * camera_front_;
        break;

      case GLFW_KEY_A : camera_pos_ += glm::normalize(glm::cross(camera_up_, camera_front_)) * camera_speed;
        break;

      case GLFW_KEY_D : camera_pos_ -= glm::normalize(glm::cross(camera_up_, camera_front_)) * camera_speed;
        break;

      case GLFW_KEY_Q : camera_pos_ += camera_up_ * camera_speed;
        break;

      case GLFW_KEY_E : camera_pos_ -= camera_up_ * camera_speed;
        break;

      default : break;
    }
  }
}

void Renderer::InitShaders(GLuint *shader_program)
{
  std::string shader_folder("/Users/nikita/CLionProjects/SelfreplicatingColloidalClustersMovie/Shaders/");
  shader_program[0] = CreateProgramFromShader(shader_folder + std::string("border_vertex_shader_wired.glsl"),
                                              shader_folder + std::string("border_fragment_shader_wired.glsl"));
  if (ParticleRepresentationType::kPointMass == particle_representation_type_)
  {
    shader_program[1] =
        CreateProgramFromShader(shader_folder + std::string("colloidal_particle_vertex_shader_point_mass.glsl"),
                                shader_folder
                                    + std::string("colloidal_particle_geometry_shader_point_mass.glsl"),
                                shader_folder
                                    + std::string("colloidal_particle_fragment_shader_point_mass.glsl"));
  } else
  {
    shader_program[1] =
        CreateProgramFromShader(shader_folder + std::string("colloidal_particle_vertex_shader_sphere.glsl"),
                                shader_folder + std::string("colloidal_particle_geometry_shader_sphere.glsl"),
                                shader_folder + std::string("colloidal_particle_fragment_shader_sphere.glsl"));
  }
  SetShaderParameter(shader_program[1], (GLfloat) model_->GetColloidDiameter(), "colloid_diameter");
//  SetShaderParameter(shader_program[1],
//                     glm::vec3(model_->GetXSize(), model_->GetYSize(), model_->GetZSize()),
//                     "system_size");
  if (ParticleRepresentationType::kPointMass == particle_representation_type_)
  {
    shader_program[2] =
        CreateProgramFromShader(shader_folder + std::string("colloidal_particle_vertex_shader_point_mass.glsl"),
                                shader_folder
                                    + std::string("colloidal_particle_geometry_shader_point_mass.glsl"),
                                shader_folder
                                    + std::string("colloidal_particle_fragment_shader_point_mass.glsl"));
  } else
  {
    shader_program[2] =
        CreateProgramFromShader(shader_folder + std::string("colloidal_particle_vertex_shader_sphere.glsl"),
                                shader_folder + std::string("colloidal_particle_geometry_shader_sphere.glsl"),
                                shader_folder + std::string("solvent_particle_fragment_shader_sphere.glsl"));
  }
  SetShaderParameter(shader_program[2], (GLfloat) model_->GetSolventDiameter(), "colloid_diameter");
//  SetShaderParameter(shader_program[2],
//                     glm::vec3(model_->GetXSize(), model_->GetYSize(), model_->GetZSize()),
//                     "system_size");

  shader_program[3] = CreateProgramFromShader(shader_folder + std::string("text_vertex_shader.glsl"),
                                              shader_folder + std::string("text_fragment_shader.glsl"));
  if (FT_Init_FreeType(&ft_))
  {
    std::cerr << "Could not init freetype library" << std::endl;
  }
  if (FT_New_Face(ft_, "/System/Library/Fonts/HelveticaNeue.ttc", 0, &face_))
  {
    std::cerr << "Could not open font" << std::endl;
  }
  FT_Set_Pixel_Sizes(face_, 0, 48);
  GLuint texture;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);
  GLint tex_location = glGetUniformLocation(shader_program[3], "tex");
  if (tex_location != -1)
  {
    glUniform1i(tex_location, 0);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLint parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  glUseProgram(shader_program);
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  assert(-1 != location);
  glUniform1i(location, parameter_value);
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  GLfloat parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  glUseProgram(shader_program);
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  assert(-1 != location);
  glUniform1f(location, parameter_value);
}

void Renderer::SetShaderParameter(GLuint shader_program,
                                  const glm::vec3 &parameter_value,
                                  const std::string &parameter_name_in_shader)
{
  glUseProgram(shader_program);
  GLint location = glGetUniformLocation(shader_program, parameter_name_in_shader.c_str());
  assert(-1 != location);
  glUniform3fv(location, 1, glm::value_ptr(parameter_value));
}

void Renderer::FinFunc()
{
//  FreePpm();
//  FreePng();
}

void Renderer::RenewSystemState()
{
  model_->ReadNewState(time_stamp_to_show_);
//  GLfloat x_size = model_->GetXSize();
//  model_->ApplyPeriodicBoundaryConditions(x_size);

  /*const std::vector<Real> &colloidal_particles = model_->GetColloidalParticles();
  std::vector<GLfloat>
      colloidal_particle_coloring(number_of_color_components_ * model_->GetNumberOfColloidalParticles(), 0.0f);
  for (int i = 0; i < model_->GetNumberOfColloidalParticles(); ++i)
  {
    colloidal_particle_coloring[number_of_color_components_ * i] = 0.0f;
    colloidal_particle_coloring[number_of_color_components_ * i + 1] = 0.0f;
    colloidal_particle_coloring[number_of_color_components_ * i + 2] = 0.0f;
  } // i

  const std::vector<Real> &solvent_particles = model_->GetSolventParticles();
  std::vector<GLfloat>
      solvent_particle_coloring(number_of_color_components_ * model_->GetNumberOfSolventParticles(), 0.0f);
  for (int i = 0; i < model_->GetNumberOfSolventParticles(); ++i)
  {
    solvent_particle_coloring[number_of_color_components_ * i] = 0.0f;
    solvent_particle_coloring[number_of_color_components_ * i + 1] = 0.0f;
    solvent_particle_coloring[number_of_color_components_ * i + 2] = 1.0f;
  } // i*/
}

void Renderer::ReadNewState()
{
  if (!stop_flag_ || !pause_flag_)
  {
    RenewSystemState();

    // speed up or down the simulation output
    model_->SkipTimeUnits(frame_speed_ - 1, 1);
    pause_flag_ = true;
  }
}

void Renderer::CreateTransformationMatrices(int width,
                                            int height,
                                            glm::mat4 &model,
                                            glm::mat4 &view,
                                            glm::mat4 &projection)
{
  projection = glm::perspective(glm::radians(fov_), (float) width / (float) height, 0.1f, 100.0f);

  view = glm::lookAt(camera_pos_, camera_pos_ + camera_front_, camera_up_);

  model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
  model = glm::rotate(glm::mat4(1.0f), z_rot_, glm::vec3(0.0f, 1.0f, 0.0f))
      * glm::rotate(glm::mat4(1.0f), y_rot_, glm::vec3(0.0f, 0.0f, 1.0f))
      * glm::rotate(glm::mat4(1.0f), x_rot_, glm::vec3(1.0f, 0.0f, 0.0f))
      * model;// * glm::rotate(glm::mat4(1.0f), y_rot, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, glm::vec3(1.0f / model_->GetXSize(), 1.0f / model_->GetZSize(), 1.0f / model_->GetYSize()));
}

void Renderer::ImportTransformationMatrices(GLuint shader_program,
                                            const glm::mat4 &model,
                                            const glm::mat4 &view,
                                            const glm::mat4 &projection)
{
  glUseProgram(shader_program);
  GLint projection_id = glGetUniformLocation(shader_program, "projection");
  if (-1 != projection_id)
  {
    glUniformMatrix4fv(projection_id, 1, GL_FALSE, glm::value_ptr(projection));
  }
  GLint view_id = glGetUniformLocation(shader_program, "view");
  if (-1 != view_id)
  {
    glUniformMatrix4fv(view_id, 1, GL_FALSE, glm::value_ptr(view));
  }
  GLint model_id = glGetUniformLocation(shader_program, "model");
  if (-1 != model_id)
  {
    glUniformMatrix4fv(model_id, 1, GL_FALSE, glm::value_ptr(model));
  }
}

void Renderer::DisplayFunc(GLFWwindow *window, GLuint *vao, GLuint *vbo, GLuint *shader_program)
{
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
//  glDepthMask(GL_FALSE);

  glEnable(GL_BLEND);
//	glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
//	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
//  glBlendEquation(GL_FUNC_ADD);
//	glBlendEquation(GL_MAX);
//  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

  glShadeModel(GL_SMOOTH);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glEnable(GL_LINE_SMOOTH);
//	glLineWidth(0.10f);

//  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_MULTISAMPLE); // MSAA

  float current_frame = glfwGetTime();
  delta_time_ = current_frame - last_frame_;
  last_frame_ = current_frame;

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glm::mat4 projection;
  glm::mat4 view;
  glm::mat4 model;
  CreateTransformationMatrices(width, height, model, view, projection);
//  glm::mat4 mvp = projection * view * model;

  ImportTransformationMatrices(shader_program[0], model, view, projection);
  ImportTransformationMatrices(shader_program[1], model, view, projection);
  ImportTransformationMatrices(shader_program[2], model, view, projection);
  ImportTransformationMatrices(shader_program[3], model, view, projection);

  glUseProgram(shader_program[0]);
  RenderWiredBorder(vao[0], vbo[0], shader_program[0]);
//  RenderSolidBorder(vao[0], vbo[0], shader_program[0], model);

  glUseProgram(shader_program[1]);
  RenderColloidalParticles(vao[1], vbo[1], shader_program[1], model);
//  RenderColloidalParticles2(vao[1], vbo[1], shader_program[1]);

  glUseProgram(shader_program[2]);
  RenderSolventParticles(vao[2], vbo[2], shader_program[2], model);

  if (show_time_)
  {
    glCullFace(GL_FRONT);
    glUseProgram(shader_program[3]);
    GLfloat sx = 2.0f / GLfloat(width);
    GLfloat sy = 2.0f / GLfloat(height);
    std::ostringstream buffer;
    buffer << std::fixed << std::setprecision(5) << "t = " << time_stamp_to_show_;
    RenderText(buffer.str(), -1.0f + 8.0f * sx, 1.0f - 50.0f * sy, sx, sy, vao[3], vbo[3], shader_program[3]);
  }
}

void Renderer::RenderWiredBorder(GLuint vao, GLuint vbo, GLuint shader_program)
{
  glBindVertexArray(vao);

  GLfloat x_size = model_->GetXSize();
  GLfloat y_size = model_->GetYSize();
  GLfloat z_size = model_->GetZSize();
  const static GLfloat border_vertices[12 * 2 * 3] =
      {
          0.0f, 0.0f, 0.0f, x_size, 0.0f, 0.0f,
          x_size, 0.0f, 0.0f, x_size, y_size, 0.0f,
          x_size, y_size, 0.0f, 0.0f, y_size, 0.0f,
          0.0f, y_size, 0.0f, 0.0f, 0.0f, 0.0f,
          0.0f, 0.0f, z_size, x_size, 0.0f, z_size,
          x_size, 0.0f, z_size, x_size, y_size, z_size,
          x_size, y_size, z_size, 0.0f, y_size, z_size,
          0.0f, y_size, z_size, 0.0f, 0.0f, z_size,
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, z_size,
          x_size, 0.0f, 0.0f, x_size, 0.0f, z_size,
          x_size, y_size, 0.0f, x_size, y_size, z_size,
          0.0f, y_size, 0.0f, 0.0f, y_size, z_size
      };

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, 12 * 2 * 3 * sizeof(GLfloat), border_vertices, GL_STATIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, 0, (void *) 0);
  glEnableVertexAttribArray(position_attribute);

  glDrawArrays(GL_LINES, 0, 12 * 2);

  glDisableVertexAttribArray(position_attribute);
}

void Renderer::RenderSolidBorder(GLuint vao, GLuint vbo, GLuint shader_program, const glm::mat4 &model)
{
  GLfloat x_size = model_->GetXSize();
  GLfloat y_size = model_->GetYSize();
  GLfloat z_size = model_->GetZSize();
  SetShaderParameter(shader_program, glm::vec3(x_size, y_size, z_size), "system_size");
  const int n_faces = 6, n_vertices = 6, n_coordinates = 3, n_normal_coordinates = 3;
  GLfloat border_vertices[n_faces * n_vertices * (n_coordinates + n_normal_coordinates)] =
      {
          x_size, y_size, z_size, 0.0f, -1.0f, 0.0f, // G
          0.0f, y_size, z_size, 0.0f, -1.0f, 0.0f, // F
          0.0f, y_size, 0.0f, 0.0f, -1.0f, 0.0f, // B
          0.0f, y_size, 0.0f, 0.0f, -1.0f, 0.0f, // B
          x_size, y_size, 0.0f, 0.0f, -1.0f, 0.0f, // C
          x_size, y_size, z_size, 0.0f, -1.0f, 0.0f, // G

          0.0f, y_size, z_size, -1.0f, 0.0f, 0.0f, // F
          0.0f, 0.0f, z_size, -1.0f, 0.0f, 0.0f, // E
          0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // A
          0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // A
          0.0f, y_size, 0.0f, -1.0f, 0.0f, 0.0f, // B
          0.0f, y_size, z_size, -1.0f, 0.0f, 0.0f, // F

          x_size, y_size, 0.0f, 0.0f, 0.0f, -1.0f, // C
          0.0f, y_size, 0.0f, 0.0f, 0.0f, -1.0f, // B
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, // A
          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, // A
          x_size, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, // D
          x_size, y_size, 0.0f, 0.0f, 0.0f, -1.0f, // C

          x_size, y_size, z_size, 1.0f, 0.0f, 0.0f, // G
          x_size, y_size, 0.0f, 1.0f, 0.0f, 0.0f, // C
          x_size, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // D
          x_size, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // D
          x_size, 0.0f, z_size, 1.0f, 0.0f, 0.0f, // H
          x_size, y_size, z_size, 1.0f, 0.0f, 0.0f, // G

          0.0f, 0.0f, z_size, 0.0f, 0.0f, 1.0f, // E
          x_size, 0.0f, z_size, 0.0f, 0.0f, 1.0f, // H
          x_size, y_size, z_size, 0.0f, 0.0f, 1.0f, // G
          x_size, y_size, z_size, 0.0f, 0.0f, 1.0f, // G
          0.0f, y_size, z_size, 0.0f, 0.0f, 1.0f, // F
          0.0f, 0.0f, z_size, 0.0f, 0.0f, 1.0f, // E

          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // A
          x_size, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // D
          x_size, 0.0f, z_size, 0.0f, 1.0f, 0.0f, // H
          x_size, 0.0f, z_size, 0.0f, 1.0f, 0.0f, // H
          0.0f, 0.0f, z_size, 0.0f, 1.0f, 0.0f, // E
          0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f // A
      };
  std::map<GLfloat, int> depth_sorted_indices;
  for (int i = 0; i < n_faces; ++i)
  {
    glm::vec3 center_of_face(0.0f);
    for (int j = 0; j < 6; ++j)
    {
      center_of_face += glm::vec3(border_vertices[i * n_vertices * (n_coordinates + n_normal_coordinates)
                                      + j * (n_coordinates + n_normal_coordinates) + 0],
                                  border_vertices[i * n_vertices * (n_coordinates + n_normal_coordinates)
                                      + j * (n_coordinates + n_normal_coordinates) + 1],
                                  border_vertices[i * n_vertices * (n_coordinates + n_normal_coordinates)
                                      + j * (n_coordinates + n_normal_coordinates) + 2]) / 6.0f;

    } // j
    glm::vec3 vector_difference = camera_pos_ - glm::vec3(model * glm::vec4(center_of_face, 1.0f));
    GLfloat distance = glm::length(vector_difference);
    depth_sorted_indices[distance] = i;
  } // i

  SetShaderParameter(shader_program, light_pos_, "light_pos");
  SetShaderParameter(shader_program, camera_pos_, "view_pos");

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER,
               n_faces * n_vertices * (n_coordinates + n_normal_coordinates) * sizeof(GLfloat),
               border_vertices,
               GL_STATIC_DRAW);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  assert(-1 != position_attribute);
  glVertexAttribPointer(position_attribute,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        (n_coordinates + n_normal_coordinates) * sizeof(GLfloat),
                        (void *) 0);
  glEnableVertexAttribArray(position_attribute);
  GLint normal_attribute = glGetAttribLocation(shader_program, "normal");
  assert(-1 != normal_attribute);
  glVertexAttribPointer(normal_attribute,
                        3,
                        GL_FLOAT,
                        GL_FALSE,
                        (n_coordinates + n_normal_coordinates) * sizeof(GLfloat),
                        (void *) (n_coordinates * sizeof(GLfloat)));
  glEnableVertexAttribArray(normal_attribute);

  for (auto rit = depth_sorted_indices.rbegin(); rit != depth_sorted_indices.rend(); ++rit)
  {
    glDrawArrays(GL_TRIANGLES, rit->second * n_vertices, n_vertices);
  } // rit
//  glDrawArrays(GL_TRIANGLES, 0, n_faces * n_vertices);

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(normal_attribute);
}

void Renderer::RenderColloidalParticles(GLuint vao, GLuint vbo, GLuint shader_program, const glm::mat4 &model)
{
  int number_of_state_variables = model_->GetNumberOfStateVariables();
  int number_of_colloidal_particles = model_->GetNumberOfColloidalParticles();
  const std::vector<Real> &colloidal_particles = model_->GetColloidalParticles();
  const std::vector<int> &colloidal_particle_subtypes = model_->GetColloidalParticleSubtypes();
  std::map<GLfloat, int> depth_sorted_indices;
  for (int i = 0; i < number_of_colloidal_particles; ++i)
  {
    glm::vec3 vector_difference =
        camera_pos_ - glm::vec3(model * glm::vec4(colloidal_particles[number_of_state_variables * i],
                                                  colloidal_particles[number_of_state_variables * i + 1],
                                                  colloidal_particles[number_of_state_variables * i + 2], 1.0f));
    GLfloat distance = glm::length(vector_difference);
    depth_sorted_indices[distance] = i;
  } // i

  if (particle_representation_type_ == ParticleRepresentationType::kPointMass)
  {
    SetShaderParameter(shader_program, camera_pos_, "camera_pos");
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, number_of_state_variables * number_of_colloidal_particles * sizeof(GLfloat)
        + number_of_colloidal_particles * sizeof(GLint), NULL, GL_DYNAMIC_DRAW);
    for (int i = 0; i < number_of_colloidal_particles; ++i)
    {
      glBufferSubData(GL_ARRAY_BUFFER,
                      i * number_of_state_variables * sizeof(GLfloat),
                      number_of_state_variables * sizeof(GLfloat),
                      &(colloidal_particles[i * number_of_state_variables]));
    } // i
    std::vector<GLint> colloidal_particle_types(number_of_colloidal_particles, 1);
    glBufferSubData(GL_ARRAY_BUFFER,
                    number_of_state_variables * number_of_colloidal_particles * sizeof(GLfloat),
                    number_of_colloidal_particles * sizeof(GLint),
                    &(colloidal_particle_types[0]));

    GLint position_attribute = glGetAttribLocation(shader_program, "position");
    assert(position_attribute != -1);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, number_of_state_variables * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(position_attribute);

    GLint particle_type_attribute = glGetAttribLocation(shader_program, "particle_type");
    assert(particle_type_attribute != -1);
    glVertexAttribIPointer(particle_type_attribute,
                           1,
                           GL_INT,
                           sizeof(GLint),
                           (GLvoid *) (number_of_state_variables * number_of_colloidal_particles * sizeof(GLfloat)));
    glEnableVertexAttribArray(particle_type_attribute);

    for (int i = 0; i < number_of_colloidal_particles; ++i)
    {
      glDrawArrays(GL_POINTS, i, 1);
    } // i

    glDisableVertexAttribArray(position_attribute);
    glDisableVertexAttribArray(particle_type_attribute);
  } else if (particle_representation_type_ == ParticleRepresentationType::kSphere)
  {
    SetShaderParameter(shader_program, light_pos_, "light.position");
    SetShaderParameter(shader_program, camera_pos_, "view_pos");
    SetShaderParameter(shader_program, glm::vec3(0.6f, 0.6f, 0.6f), "material.specular");
    SetShaderParameter(shader_program, std::powf(2.0f, 1.0f), "material.shininess");
    SetShaderParameter(shader_program, glm::vec3(0.38f, 0.38f, 0.38f), "light.ambient");
    SetShaderParameter(shader_program, glm::vec3(0.5f, 0.5f, 0.5f), "light.diffuse");
    SetShaderParameter(shader_program, glm::vec3(1.0f, 1.0f, 1.0f), "light.specular");
    ComposeSphereTemplate(1.0f);
    const int number_of_patches = sphere_template_.size();
    const int number_of_points_per_patch = sphere_template_.front().size();
    const int dim = 3;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat)
                     + number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat),
                 NULL,
                 GL_DYNAMIC_DRAW);
    for (int patch_idx = 0; patch_idx < number_of_patches; ++patch_idx)
    {
      for (int point_idx = 0; point_idx < number_of_points_per_patch; ++point_idx)
      {
        glBufferSubData(GL_ARRAY_BUFFER,
                        (point_idx + patch_idx * number_of_points_per_patch) * dim * sizeof(GLfloat),
                        dim * sizeof(GLfloat),
                        &(sphere_template_[patch_idx][point_idx]));
        glBufferSubData(GL_ARRAY_BUFFER,
                        number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat)
                            + (point_idx + patch_idx * number_of_points_per_patch) * dim * sizeof(GLfloat),
                        dim * sizeof(GLfloat),
                        &(sphere_normals_[patch_idx][point_idx]));
      } // coordinate_idx
    } // patch_idx

    GLint position_attribute = glGetAttribLocation(shader_program, "position");
    assert(position_attribute != -1);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, dim * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(position_attribute);
    GLint normal_attribute = glGetAttribLocation(shader_program, "normal");
    assert(normal_attribute != -1);
    glVertexAttribPointer(normal_attribute,
                          3,
                          GL_FLOAT,
                          GL_TRUE,
                          dim * sizeof(GLfloat),
                          (void *) (number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat)));
    glEnableVertexAttribArray(normal_attribute);

//    for (auto rit = depth_sorted_indices.begin(); rit != depth_sorted_indices.end(); ++rit)
    for (auto rit = depth_sorted_indices.rbegin(); rit != depth_sorted_indices.rend(); ++rit)
    {
      int i = rit->second;
      glm::mat4 particle_translation = glm::translate(glm::mat4(1.0f),
                                                      glm::vec3(colloidal_particles[i * number_of_state_variables],
                                                                colloidal_particles[i * number_of_state_variables + 1],
                                                                colloidal_particles[i * number_of_state_variables
                                                                    + 2]));
      GLint particle_translation_id = glGetUniformLocation(shader_program, "particle_translation");
      assert(-1 != particle_translation_id);
      glUniformMatrix4fv(particle_translation_id, 1, GL_FALSE, glm::value_ptr(particle_translation));
      switch (static_cast<ParticleSubtype>(colloidal_particle_subtypes[i]))
      {
        case ParticleSubtype::kA ://SetShaderParameter(shader_program, glm::vec3(0.0f, 0.0f, 1.0f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.0f, 0.0f, 1.0f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.0f, 0.0f, 1.0f), "material.diffuse");
          break;
        case ParticleSubtype::kB ://SetShaderParameter(shader_program, glm::vec3(0.3f, 0.8f, 0.1f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.3f, 0.8f, 0.1f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.3f, 0.8f, 0.1f), "material.diffuse");
          break;
        case ParticleSubtype::kC ://SetShaderParameter(shader_program, glm::vec3(0.8f, 0.1f, 0.3f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.8f, 0.1f, 0.3f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.8f, 0.1f, 0.3f), "material.diffuse");
          break;
        case ParticleSubtype::kBStar ://SetShaderParameter(shader_program, glm::vec3(0.85f, 0.33f, 0.1f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.85f, 0.33f, 0.1f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.85f, 0.33f, 0.1f), "material.diffuse");
          break;
        case ParticleSubtype::kCStar ://SetShaderParameter(shader_program, glm::vec3(0.5f, 0.19f, 0.9f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.5f, 0.19f, 0.9f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.5f, 0.19f, 0.9f), "material.diffuse");
          break;
        case ParticleSubtype::kAPrime ://SetShaderParameter(shader_program,glm::vec3(0.3f, 0.75f, 0.93f),"object_color");
          SetShaderParameter(shader_program, glm::vec3(0.3f, 0.75f, 0.93f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.3f, 0.75f, 0.93f), "material.diffuse");
          break;
        case ParticleSubtype::kBPrime ://SetShaderParameter(shader_program, glm::vec3(1.0f, 1.0f, 0.0f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(1.0f, 1.0f, 0.0f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(1.0f, 1.0f, 0.0f), "material.diffuse");
          break;
        case ParticleSubtype::kCPrime ://SetShaderParameter(shader_program, glm::vec3(1.0f, 0.0f, 1.0f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(1.0f, 0.0f, 1.0f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(1.0f, 0.0f, 1.0f), "material.diffuse");
          break;
        case ParticleSubtype::kBStarPrime ://SetShaderParameter(shader_program,glm::vec3(0.43f, 0.31f, 0.31f),"object_color");
          SetShaderParameter(shader_program, glm::vec3(0.43f, 0.31f, 0.31f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.43f, 0.31f, 0.31f), "material.diffuse");
          break;
        case ParticleSubtype::kCStarPrime ://SetShaderParameter(shader_program,glm::vec3(0.3f, 0.09f, 0.4f),"object_color");
          SetShaderParameter(shader_program, glm::vec3(0.3f, 0.09f, 0.4f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.3f, 0.09f, 0.4f), "material.diffuse");
          break;
        case ParticleSubtype::k0 ://SetShaderParameter(shader_program, glm::vec3(0.9f, 0.9f, 0.9f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.9f, 0.9f, 0.9f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.9f, 0.9f, 0.9f), "material.diffuse");
          break;
        default ://SetShaderParameter(shader_program, glm::vec3(0.1f, 0.1f, 0.1f), "object_color");
          SetShaderParameter(shader_program, glm::vec3(0.1f, 0.1f, 0.1f), "material.ambient");
          SetShaderParameter(shader_program, glm::vec3(0.1f, 0.1f, 0.1f), "material.diffuse");
      }
      glDrawArrays(GL_TRIANGLES, 0, number_of_patches * number_of_points_per_patch);
    } // i

    glDisableVertexAttribArray(position_attribute);
    glDisableVertexAttribArray(normal_attribute);
  }
}

void Renderer::RenderColloidalParticles2(GLuint vao, GLuint vbo, GLuint shader_program)
{
  int number_of_state_variables = model_->GetNumberOfStateVariables();
  int number_of_colloidal_particles = model_->GetNumberOfColloidalParticles();
  const std::vector<Real> &colloidal_particles = model_->GetColloidalParticles();

  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, number_of_state_variables * 1 * sizeof(GLfloat)
      + 1 * sizeof(GLint), NULL, GL_DYNAMIC_DRAW);
  std::vector<GLfloat> vertex({0.0f, 0.0f, 0.0f});
  glBufferSubData(GL_ARRAY_BUFFER,
                  0,
                  number_of_state_variables * sizeof(GLfloat),
                  &vertex);
  GLint type = 0;
  glBufferSubData(GL_ARRAY_BUFFER,
                  0,
                  sizeof(GLint),
                  &type);

  GLint position_attribute = glGetAttribLocation(shader_program, "position");
  assert(position_attribute != -1);
  glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, number_of_state_variables * sizeof(GLfloat), 0);
  glEnableVertexAttribArray(position_attribute);

  GLint particle_type_attribute = glGetAttribLocation(shader_program, "particle_type");
  assert(particle_type_attribute != -1);
  glVertexAttribIPointer(particle_type_attribute,
                         1,
                         GL_INT,
                         sizeof(GLint),
                         0);
  glEnableVertexAttribArray(particle_type_attribute);

  for (int i = 0; i < number_of_colloidal_particles; ++i)
  {
    glm::mat4 particle_translation = glm::translate(glm::mat4(1.0f),
                                                    glm::vec3(colloidal_particles[i * number_of_state_variables],
                                                              colloidal_particles[i * number_of_state_variables + 1],
                                                              colloidal_particles[i * number_of_state_variables + 2]));
    GLint particle_translation_id = glGetUniformLocation(shader_program, "particle_translation");
    assert(-1 != particle_translation_id);
    glUniformMatrix4fv(particle_translation_id, 1, GL_FALSE, glm::value_ptr(particle_translation));
    glDrawArrays(GL_POINTS, 0, 1);
  } // i

  glDisableVertexAttribArray(position_attribute);
  glDisableVertexAttribArray(particle_type_attribute);
}

void Renderer::RenderSolventParticles(GLuint vao, GLuint vbo, GLuint shader_program, const glm::mat4 &model)
{
  int number_of_state_variables = model_->GetNumberOfStateVariables();
  int number_of_solvent_particles = model_->GetNumberOfSolventParticles();
  const std::vector<Real> &solvent_particles = model_->GetSolventParticles();
  std::map<GLfloat, int> depth_sorted_indices;
  for (int i = 0; i < number_of_solvent_particles; ++i)
  {
    glm::vec3 vector_difference =
        camera_pos_ - glm::vec3(model * glm::vec4(solvent_particles[number_of_state_variables * i],
                                                  solvent_particles[number_of_state_variables * i + 1],
                                                  solvent_particles[number_of_state_variables * i + 2], 1.0f));
    GLfloat distance = glm::length(vector_difference);
    depth_sorted_indices[distance] = i;
  } // i

  if (particle_representation_type_ == ParticleRepresentationType::kPointMass)
  {
    SetShaderParameter(shader_program, camera_pos_, "camera_pos");
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, number_of_state_variables * number_of_solvent_particles * sizeof(GLfloat)
        + number_of_solvent_particles * sizeof(GLint), NULL, GL_DYNAMIC_DRAW);
    for (int i = 0; i < number_of_solvent_particles; ++i)
    {
      glBufferSubData(GL_ARRAY_BUFFER,
                      i * number_of_state_variables * sizeof(GLfloat),
                      number_of_state_variables * sizeof(GLfloat),
                      &(solvent_particles[i * number_of_state_variables]));
    } // i
    std::vector<GLint> colloidal_particle_types(number_of_solvent_particles, 1);
    glBufferSubData(GL_ARRAY_BUFFER,
                    number_of_state_variables * number_of_solvent_particles * sizeof(GLfloat),
                    number_of_solvent_particles * sizeof(GLint),
                    &(colloidal_particle_types[0]));

    GLint position_attribute = glGetAttribLocation(shader_program, "position");
    assert(position_attribute != -1);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, number_of_state_variables * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(position_attribute);

    GLint particle_type_attribute = glGetAttribLocation(shader_program, "particle_type");
    assert(particle_type_attribute != -1);
    glVertexAttribIPointer(particle_type_attribute,
                           1,
                           GL_INT,
                           sizeof(GLint),
                           (GLvoid *) (number_of_state_variables * number_of_solvent_particles * sizeof(GLfloat)));
    glEnableVertexAttribArray(particle_type_attribute);

    for (int i = 0; i < number_of_solvent_particles; ++i)
    {
      glDrawArrays(GL_POINTS, i, 1);
    } // i

    glDisableVertexAttribArray(position_attribute);
    glDisableVertexAttribArray(particle_type_attribute);
  } else if (particle_representation_type_ == ParticleRepresentationType::kSphere)
  {
    SetShaderParameter(shader_program, light_pos_, "light.position");
    SetShaderParameter(shader_program, camera_pos_, "view_pos");
    SetShaderParameter(shader_program, glm::vec3(0.5f, 0.5f, 0.5f), "material.ambient");
    SetShaderParameter(shader_program, glm::vec3(0.5f, 0.5f, 0.5f), "material.diffuse");
    SetShaderParameter(shader_program, glm::vec3(0.6f, 0.6f, 0.6f), "material.specular");
    SetShaderParameter(shader_program, std::powf(2.0f, 1.0f), "material.shininess");
    SetShaderParameter(shader_program, glm::vec3(0.38f, 0.38f, 0.38f), "light.ambient");
    SetShaderParameter(shader_program, glm::vec3(0.5f, 0.5f, 0.5f), "light.diffuse");
    SetShaderParameter(shader_program, glm::vec3(1.0f, 1.0f, 1.0f), "light.specular");
    ComposeSphereTemplate(1.0f);
    const int number_of_patches = sphere_template_.size();
    const int number_of_points_per_patch = sphere_template_.front().size();
    const int dim = 3;

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat)
                     + number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat),
                 NULL,
                 GL_DYNAMIC_DRAW);
    for (int patch_idx = 0; patch_idx < number_of_patches; ++patch_idx)
    {
      for (int point_idx = 0; point_idx < number_of_points_per_patch; ++point_idx)
      {
        glBufferSubData(GL_ARRAY_BUFFER,
                        (point_idx + patch_idx * number_of_points_per_patch) * dim * sizeof(GLfloat),
                        dim * sizeof(GLfloat),
                        &(sphere_template_[patch_idx][point_idx]));
        glBufferSubData(GL_ARRAY_BUFFER,
                        number_of_patches * number_of_points_per_patch * dim * sizeof(GLfloat)
                            + (point_idx + patch_idx * number_of_points_per_patch) * dim * sizeof(GLfloat),
                        dim * sizeof(GLfloat),
                        &(sphere_normals_[patch_idx][point_idx]));
      } // coordinate_idx
    } // patch_idx

    GLint position_attribute = glGetAttribLocation(shader_program, "position");
    assert(position_attribute != -1);
    glVertexAttribPointer(position_attribute, 3, GL_FLOAT, GL_FALSE, dim * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(position_attribute);

//    for (auto rit = depth_sorted_indices.begin(); rit != depth_sorted_indices.end(); ++rit)
    for (auto rit = depth_sorted_indices.rbegin(); rit != depth_sorted_indices.rend(); ++rit)
    {
      int i = rit->second;
      glm::mat4 particle_translation = glm::translate(glm::mat4(1.0f),
                                                      glm::vec3(solvent_particles[i * number_of_state_variables],
                                                                solvent_particles[i * number_of_state_variables + 1],
                                                                solvent_particles[i * number_of_state_variables
                                                                    + 2]));
      GLint particle_translation_id = glGetUniformLocation(shader_program, "particle_translation");
      assert(-1 != particle_translation_id);
      glUniformMatrix4fv(particle_translation_id, 1, GL_FALSE, glm::value_ptr(particle_translation));
      glDrawArrays(GL_TRIANGLES, 0, number_of_patches * number_of_points_per_patch);
    } // i

    glDisableVertexAttribArray(position_attribute);
  }
}

void Renderer::ComposeSphereTemplate(float radius)
{
  sphere_template_.clear();
  sphere_normals_.clear();
  static const int n_phi = 10, n_theta = 10;
  static const GLfloat dphi = 2.0 * M_PI / n_phi, dtheta = M_PI / n_theta;
  GLfloat phi0 = 0.0f, phi1 = 0.0f, theta0 = 0.0f, theta1 = 0.0f;
  std::array<GLfloat, 3> r00 = {0.0f, 0.0f, 0.0f}, r01 = {0.0f, 0.0f, 0.0f}, r10 = {0.0f, 0.0f, 0.0f},
      r11 = {0.0f, 0.0f, 0.0f}, dr1 = {0.0f, 0.0f, 0.0f}, dr2 = {0.0f, 0.0f, 0.0f}, patch_normal = {0.0f, 0.0f, 0.0f};
  std::vector<std::array<GLfloat, 3>> sphere_patch; // todo: rewrite via glm::vec3
  for (int i = 0; i < n_phi; ++i)
  {
    phi0 = i * dphi;
    phi1 = (i + 1) * dphi;
    for (int j = 0; j < n_theta; ++j)
    {
      theta0 = j * dtheta;
      theta1 = (j + 1) * dtheta;
      GetSphericalCoordinate(radius, phi0, theta0, r00);
      GetSphericalCoordinate(radius, phi0, theta1, r01);
      GetSphericalCoordinate(radius, phi1, theta1, r11);
      GetSphericalCoordinate(radius, phi1, theta0, r10);
      sphere_patch.clear();
      sphere_patch.push_back(r00);
      sphere_patch.push_back(r01);
      sphere_patch.push_back(r11);
      sphere_patch.push_back(r11);
      sphere_patch.push_back(r10);
      sphere_patch.push_back(r00);
//      std::reverse(sphere_patch.begin(), sphere_patch.end());
      sphere_template_.push_back(sphere_patch);

      std::transform(r00.begin(), r00.end(), r01.begin(), dr1.begin(), [](GLfloat r1, GLfloat r2) { return r2 - r1; });
      std::transform(r01.begin(), r01.end(), r11.begin(), dr2.begin(), [](GLfloat r1, GLfloat r2) { return r2 - r1; });
      patch_normal[0] = dr1[1] * dr2[2] - dr1[2] * dr2[1];
      patch_normal[1] = dr1[2] * dr2[0] - dr1[0] * dr2[2];
      patch_normal[2] = dr1[0] * dr2[1] - dr1[1] * dr2[0];
      Real norm = std::sqrtf(
          patch_normal[0] * patch_normal[0] + patch_normal[1] * patch_normal[1] + patch_normal[2] * patch_normal[2]);
      std::for_each(patch_normal.begin(), patch_normal.end(), [&](Real &r) { r /= norm; });
      sphere_normals_.push_back({patch_normal, patch_normal, patch_normal, patch_normal, patch_normal, patch_normal});
    } // j
  } // i
}

void Renderer::RenderText(const std::string &text,
                          GLfloat x,
                          GLfloat y,
                          GLfloat sx,
                          GLfloat sy,
                          GLuint vao,
                          GLuint vbo,
                          GLuint shader_program)
{
  glBindVertexArray(vao);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);

  GLint coord_location = glGetAttribLocation(shader_program, "generic_coord");
  glVertexAttribPointer(coord_location, 4, GL_FLOAT, GL_FALSE, 0, 0);
  glEnableVertexAttribArray(coord_location);

  FT_GlyphSlot glyph_slot = face_->glyph;

  for (char letter : text)
  {
    if (FT_Load_Char(face_, letter, FT_LOAD_RENDER))
    {
      continue;
    }

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 glyph_slot->bitmap.width,
                 glyph_slot->bitmap.rows,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 glyph_slot->bitmap.buffer);

    GLfloat x2 = x + glyph_slot->bitmap_left * sx;
    GLfloat y2 = -y - glyph_slot->bitmap_top * sy;
    GLfloat w = glyph_slot->bitmap.width * sx;
    GLfloat h = glyph_slot->bitmap.rows * sy;

    GLfloat box[4][4] =
        {
            {x2, -y2, 0.0f, 0.0f},
            {x2 + w, -y2, 1.0f, 0.0f},
            {x2, -y2 - h, 0.0f, 1.0f},
            {x2 + w, -y2 - h, 1.0f, 1.0f}
        };

    glBufferData(GL_ARRAY_BUFFER, sizeof box, box, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    x += (glyph_slot->advance.x / 64) * sx;
    y += (glyph_slot->advance.y / 64) * sy;
  } // i
  glDisableVertexAttribArray(coord_location);
}

void Renderer::ReadShaderSource(const std::string &fname, std::vector<char> &buffer)
{
  std::ifstream in;
  in.open(fname, std::ios::binary | std::ios::in);

  if (in.is_open())
  {
    in.seekg(0, std::ios::end);
    size_t length = (size_t) in.tellg();

    in.seekg(0, std::ios::beg);

    buffer.resize(length + 1);
    in.read((char *) &buffer[0], length);
    buffer[length] = '\0';

    in.close();
  } else
  {
    std::cerr << "Unable to read the shader file \"" << fname << "\"" << std::endl;
    exit(EXIT_FAILURE);
  }
}

GLuint Renderer::LoadAndCompileShader(const std::string &fname, GLenum shader_type)
{
  std::vector<char> buffer;
  ReadShaderSource(fname, buffer);
  const char *src = &buffer[0];

  GLuint shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &src, NULL);
  glCompileShader(shader);

  GLint compilation_test;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compilation_test);
  if (!compilation_test)
  {
    std::cerr << "Shader compilation failed with the following message: " << std::endl;
    std::vector<char> compilation_log(512, '\0');
    glGetShaderInfoLog(shader, (GLsizei) compilation_log.size(), NULL, &compilation_log[0]);
    std::cerr << &compilation_log[0] << std::endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  return shader;
}

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path, const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}

GLuint Renderer::CreateProgramFromShader(const std::string &vertex_shader_path,
                                         const std::string &geometry_shader_path,
                                         const std::string &fragment_shader_path)
{
  GLuint vertex_shader = LoadAndCompileShader(vertex_shader_path, GL_VERTEX_SHADER);
  GLuint geometry_shader = LoadAndCompileShader(geometry_shader_path, GL_GEOMETRY_SHADER);
  GLuint fragment_shader = LoadAndCompileShader(fragment_shader_path, GL_FRAGMENT_SHADER);

  GLuint shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, geometry_shader);
  glAttachShader(shader_program, fragment_shader);

  glDeleteShader(vertex_shader);
  glDeleteShader(geometry_shader);
  glDeleteShader(fragment_shader);

  glLinkProgram(shader_program);
  glUseProgram(shader_program);

  return shader_program;
}