#ifndef ACTOR_H
#define ACTOR_H

#include <QMatrix4x4>
#include <QImage>
#include <QVector3D>
#include <QVector2D>
#include <QString>
// Need GLuint and GLenum types
#include <QOpenGLFunctions_3_3_Core>

// Forward declarations
class QOpenGLShaderProgram;
class Model; // Assuming 'Model' is defined in model.h

// You need to forward declare MainView because the implementation will call
// MainView::imageToBytes, even though the declaration doesn't need it.
// It's good practice to keep the required includes in the header minimal.
// However, since the declaration is likely needed by classes using Actor,
// and to avoid confusion, we'll keep the full structure, though it's the
// implementation in actor.cpp that truly needs mainview.h.
// For this header, let's assume the necessary type headers are enough.

/**
 * @brief The Actor class represents a drawable 3D object in the scene.
 */
class Actor
{
public:
    // OpenGL Buffer IDs
    GLuint VAO, positionVBO, uvVBO, normalVBO, colorVBO;
    QMatrix4x4 transform;

    GLuint meshSize;

    // Texture handling
    GLuint texDiffuse;
    bool hasDiffuseTex = false;

    GLuint texEmission;
    bool hasEmissionTex = false;

    // Shader program reference
    QOpenGLShaderProgram &shaderProgram;

    /**
     * @brief Constructor for Actor.
     * @param filename Path to the model file.
     * @param program Reference to the shader program to use for rendering.
     */
    Actor(const QString &filename, QOpenGLShaderProgram &program);

    /**
     * @brief Sets the diffuse texture for the actor.
     * @param image The QImage to use as the texture.
     */
    void setDiffuseTexture(QImage image);

    void setEmissionTexture(QImage image);

    /**
     * @brief Renders the actor.
     * @param viewTransform The view matrix.
     * @param projectionTransform The projection matrix.
     * @param time Current scene time for animations.
     */
    void paint(
        QMatrix4x4 &viewTransform, QMatrix4x4 &projectionTransform, float time = 0.0F);
};

#endif // ACTOR_H