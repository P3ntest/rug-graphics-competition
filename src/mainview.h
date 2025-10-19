#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QKeyEvent>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>
#include <QElapsedTimer>

// for GLuint
#include <QOpenGLFunctions_3_3_Core>

#include "model.h"
#include "shadingmode.h"

#include "actor.h"

/**
 * @brief The MainView class is resonsible for the actual content of the main
 * window.
 */
class MainView : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
  Q_OBJECT

public:
  MainView(QWidget *parent = nullptr);
  ~MainView() override;

  // Functions for widget input events
  void setRotation(int rotateX, int rotateY, int rotateZ);
  void setScale(float scale);
  void setShadingMode(ShadingMode shading);
  static QVector<quint8> imageToBytes(const QImage &image);

protected:
  void initializeGL() override;
  void resizeGL(int newWidth, int newHeight) override;
  void paintGL() override;

  int realWidth() const;
  int realHeight() const;

  // Functions for keyboard input events
  void keyPressEvent(QKeyEvent *ev) override;
  void keyReleaseEvent(QKeyEvent *ev) override;

  // Function for mouse input events
  void mouseDoubleClickEvent(QMouseEvent *ev) override;
  void mouseMoveEvent(QMouseEvent *ev) override;
  void mousePressEvent(QMouseEvent *ev) override;
  void mouseReleaseEvent(QMouseEvent *ev) override;
  void wheelEvent(QWheelEvent *ev) override;

private slots:
  void onMessageLogged(QOpenGLDebugMessage Message);

private:
  void destroyModelBuffers();
  void updateProjectionTransform();
  void updateModelTransforms();
  void loadShaders(QOpenGLShaderProgram &program, const QString &vertPath,
                   const QString &fragPath);

  void setupGBuffer(int width, int height);

  void renderQuad();

  QOpenGLDebugLogger debugLogger;
  QTimer timer; // timer used for animation

  GLuint gBuffer;
  GLuint gPosition, gNormal, gAlbedoSpec, gEmission;
  GLuint gDepth;

  // Shaders for the two passes
  QOpenGLShaderProgram gBufferShader;  // For Geometry Pass
  QOpenGLShaderProgram lightingShader; // For Lighting Pass (Post-Process Quad)

  // // A simple VAO/VBO for drawing the screen quad
  GLuint quadVAO = 0;
  GLuint quadVBO = 0;

  QOpenGLShaderProgram basicShader;
  QOpenGLShaderProgram waterShader;
  QVector<Actor> actors = {};

  QElapsedTimer startTimer;

  // Transforms
  float scale = 1.0F;
  QVector3D rotation;
  QMatrix4x4 projectionTransform;
  QMatrix4x4 viewTransform;
};

#endif // MAINVIEW_H
