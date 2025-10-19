#include "mainview.h"

#include <QDateTime>

MainView::MainView(QWidget *parent) : QOpenGLWidget(parent)
{
  qDebug() << "MainView constructor";

  connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
}

MainView::~MainView()
{
  qDebug() << "MainView destructor";

  makeCurrent();

  destroyModelBuffers();
}

// --- OpenGL initialization

void MainView::loadShaders(QOpenGLShaderProgram &program, const QString &vertPath,
                           const QString &fragPath)
{
  program.addShaderFromSourceFile(QOpenGLShader::Vertex, vertPath);
  program.addShaderFromSourceFile(QOpenGLShader::Fragment, fragPath);
  program.link();
}

/**
 * @brief MainView::initializeGL Called upon OpenGL initialization
 * Attaches a debugger and calls other init functions.
 */
void MainView::initializeGL()
{
  qDebug() << ":: Initializing OpenGL";
  initializeOpenGLFunctions();

  connect(&debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
          SLOT(onMessageLogged(QOpenGLDebugMessage)), Qt::DirectConnection);

  if (debugLogger.initialize())
  {
    qDebug() << ":: Logging initialized";
    debugLogger.startLogging(QOpenGLDebugLogger::SynchronousLogging);
  }

  QString glVersion{reinterpret_cast<const char *>(glGetString(GL_VERSION))};
  qDebug() << ":: Using OpenGL" << qPrintable(glVersion);

  // Enable depth buffer
  glEnable(GL_DEPTH_TEST);

  // Enable backface culling
  glEnable(GL_CULL_FACE);

  // Default is GL_LESS
  glDepthFunc(GL_LEQUAL);

  // Set the color to be used by glClear. This is, effectively, the background
  // color.
  glClearColor(0.04f, 0.05f, 0.07f, 0.0f);

  loadShaders(basicShader, ":/shaders/vertshader.glsl", ":/shaders/fragshader.glsl");
  loadShaders(waterShader, ":/shaders/watervert.glsl", ":/shaders/g_buffer_frag.glsl");

  loadShaders(gBufferShader, ":/shaders/g_buffer_vert.glsl",
              ":/shaders/g_buffer_frag.glsl");
  loadShaders(lightingShader, ":/shaders/quad_vert.glsl",
              ":/shaders/lighting_frag.glsl");

  setupGBuffer(realWidth(), realHeight());

  Actor cat(":/models/cat.obj", gBufferShader);
  cat.transform.setToIdentity();
  cat.transform.translate(0.0F, 0.0F, -10.0F);
  // cat.transform.rotate(QQuaternion::fromEulerAngles(rotation));
  // cat.transform.scale(scale);
  // requires QImage
  cat.setDiffuseTexture(QImage(":/textures/cat_diff.png"));
  // actors.push_back(cat);

  Actor water(":/models/water.obj", waterShader);
  water.transform.setToIdentity();
  // water.transform.translate(0.0F, -3.0F, -10.0F);
  // water.transform.rotate(QQuaternion::fromEulerAngles(0, 0, 0));
  // water.transform.scale(15.0f);
  actors.push_back(water);

  Actor sceneObj(":/models/sceneobj.obj", gBufferShader);
  sceneObj.transform.setToIdentity();
  sceneObj.setDiffuseTexture(QImage(":/textures/concrete_wall.png"));
  // sceneObj.transform.translate(0.0F, -1.0F, -5.0F);
  // sceneObj.transform.rotate(QQuaternion::fromEulerAngles(0, -90, 0));
  // sceneObj.transform.scale(0.2f);

  Actor sign(":/models/sign.obj", gBufferShader);
  sign.transform.setToIdentity();
  sign.setDiffuseTexture(QImage(":/textures/sign_diffuse.png"));
  sign.setEmissionTexture(QImage(":/textures/sign_emission.png"));
  actors.push_back(sign);

  actors.push_back(sceneObj);

  Actor lamps(":/models/lamps.obj", gBufferShader);
  lamps.transform.setToIdentity();
  lamps.setDiffuseTexture(QImage(":/textures/lamp_diffuse.png"));
  lamps.setEmissionTexture(QImage(":/textures/lamp_emission.png"));
  actors.push_back(lamps);

  Actor apart(":/models/apart.obj", gBufferShader);
  apart.transform.setToIdentity();
  apart.setDiffuseTexture(QImage(":/textures/apart_diffuse.png"));
  apart.setEmissionTexture(QImage(":/textures/apart_emission.png"));

  actors.push_back(apart);

  startTimer.restart();

  // Initialize transformations
  updateProjectionTransform();

  update();
}

/**
 * @brief MainView::paintGL Actual function used for drawing to the screen.
 *
 */
void MainView::paintGL()
{

  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  float elapsedSeconds = static_cast<float>(startTimer.elapsed()) / 1000.0f;

  // gBufferShader.bind();

  // print elapsed time

  for (auto &actor : actors)
  {
    actor.paint(viewTransform, projectionTransform, elapsedSeconds);
  }

  // gBufferShader.release();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  glViewport(0, 0, realWidth(), realHeight());
  glDrawBuffer(GL_BACK);
  glClear(GL_COLOR_BUFFER_BIT);

  lightingShader.bind();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  lightingShader.setUniformValue("gPosition", 0);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  lightingShader.setUniformValue("gNormal", 1);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
  lightingShader.setUniformValue("gAlbedoSpec", 2);

  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, gEmission);
  lightingShader.setUniformValue("gEmission", 3);

  lightingShader.setUniformValue("projection", projectionTransform);

  // Render screen quad
  renderQuad();

  lightingShader.release();

  update();
}

void MainView::renderQuad()
{
  // VBO data for a quad in NDC space
  float quadVertices[] = {
      // positions (x, y) // texture coords (u, v)
      -1.0f,
      1.0f,
      0.0f,
      1.0f,

      -1.0f,
      -1.0f,
      0.0f,
      0.0f,

      1.0f,
      -1.0f,
      1.0f,
      0.0f,

      -1.0f,
      1.0f,
      0.0f,
      1.0f,

      1.0f,
      -1.0f,
      1.0f,
      0.0f,

      1.0f,
      1.0f,
      1.0f,
      1.0f,
  };

  if (quadVAO == 0)
  {
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0); // position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float))); // texture coord
  }
  glBindVertexArray(quadVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

/**
 * @brief MainView::resizeGL Called upon resizing of the screen.
 *
 * @param newWidth The new width of the screen in pixels.
 * @param newHeight The new height of the screen in pixels.
 */
void MainView::resizeGL(int newWidth, int newHeight)
{
  Q_UNUSED(newWidth)
  Q_UNUSED(newHeight)
  updateProjectionTransform();

  setupGBuffer(realWidth(), realHeight());
}

/**
 * @brief MainView::updateProjectionTransform Updates the projection transform
 * matrix taking into consideration the current aspect ratio.
 */
void MainView::updateProjectionTransform()
{
  float aspectRatio =
      static_cast<float>(realWidth()) / static_cast<float>(realHeight());
  projectionTransform.setToIdentity();
  projectionTransform.perspective(50.0F, aspectRatio, 0.2F, 1000.0F);

  viewTransform.setToIdentity();
}

/**
 * @brief MainView::updateModelTransforms Updates the model transform matrix of
 * the mesh to reflect the current rotation and scale values.
 */
void MainView::updateModelTransforms()
{
  // for (auto &actor : actors)
  // {
  //   actor.transform.setToIdentity();
  //   actor.transform.translate(0.0F, 0.0F, -10.0F);
  //   actor.transform.rotate(QQuaternion::fromEulerAngles(rotation));
  //   actor.transform.scale(scale);
  // }

  update();
}

/**
 * @brief MainView::destroyModelBuffers Cleans up the memory used by OpenGL.
 */
void MainView::destroyModelBuffers()
{
}

/**
 * @brief MainView::setRotation Changes the rotation of the displayed objects.
 * @param rotateX Number of degrees to rotate around the x axis.
 * @param rotateY Number of degrees to rotate around the y axis.
 * @param rotateZ Number of degrees to rotate around the z axis.
 */
void MainView::setRotation(int rotateX, int rotateY, int rotateZ)
{
  rotation = {static_cast<float>(rotateX), static_cast<float>(rotateY),
              static_cast<float>(rotateZ)};
  updateModelTransforms();
}

/**
 * @brief MainView::setScale Changes the scale of the displayed objects.
 * @param scale The new scale factor. A scale factor of 1.0 should scale the
 * mesh to its original size.
 */
void MainView::setScale(float newScale)
{
  scale = newScale;
  updateModelTransforms();
}

void MainView::setShadingMode(ShadingMode shading)
{
  qDebug() << "Changed shading to" << shading;
  Q_UNIMPLEMENTED();
}

/**
 * @brief MainView::onMessageLogged OpenGL logging function, do not change.
 *
 * @param Message The message to be logged.
 */
void MainView::onMessageLogged(QOpenGLDebugMessage Message)
{
  qDebug() << " → Log:" << Message;
}

void MainView::setupGBuffer(int width, int height)
{
  // 1. Generate FBO (only needs to be done once)
  if (gBuffer == 0)
  {
    glGenFramebuffers(1, &gBuffer);
    glGenTextures(1, &gPosition);
    glGenTextures(1, &gNormal);
    glGenTextures(1, &gAlbedoSpec);
    glGenTextures(1, &gDepth);
    glGenTextures(1, &gEmission);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

  // 2. Attachments (Dimensions must match the viewport)

  // Position Color Buffer (e.g., View-space coordinates)
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

  // Normal Color Buffer (e.g., View-space normals)
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

  // Albedo + Specular Color Buffer (Albedo: RGB, Specular: A)
  glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

  // 3. Depth Buffer (Used for Depth Testing in Geometry Pass)
  glBindTexture(GL_TEXTURE_2D, gDepth);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, gDepth, 0);
  // You can also use GL_DEPTH24_STENCIL8 if you need a stencil buffer.

  glBindTexture(GL_TEXTURE_2D, gEmission);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, gEmission, 0);

  // 4. Tell OpenGL which color attachments we'll use (Color Attachments 0-2)
  GLenum attachments[4] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
  glDrawBuffers(4, attachments);

  // 5. Check for FBO completeness
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    qWarning() << "G-Buffer FBO not complete!";

  // 6. Bind default framebuffer again
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

int MainView::realWidth() const
{
  return width() * devicePixelRatioF();
}

int MainView::realHeight() const
{
  return height() * devicePixelRatioF();
}
