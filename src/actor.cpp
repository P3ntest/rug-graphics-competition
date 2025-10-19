#include "actor.h"
#include <QOpenGLShaderProgram>
#include "mainview.h"
#include <iostream>

Actor::Actor(const QString &filename, QOpenGLShaderProgram &program)
    : shaderProgram(program)
{
    Model model(filename);
    QVector<QVector3D> meshCoords = model.getCoords();
    QVector<QVector3D> meshColors = model.getRandomColors();
    QVector<QVector2D> meshUVs = model.getTextureCoords();
    QVector<QVector3D> meshNormals = model.getNormals();

    meshSize = meshCoords.size();

    // Generate VAO
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // Generate VBOs
    glGenBuffers(1, &positionVBO);
    glGenBuffers(1, &colorVBO);
    glGenBuffers(1, &uvVBO);
    glGenBuffers(1, &normalVBO);

    // Positions
    glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
    glBufferData(GL_ARRAY_BUFFER, meshCoords.size() * sizeof(QVector3D),
                 meshCoords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(0);

    // Copy the colors
    glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
    glBufferData(GL_ARRAY_BUFFER, meshColors.size() * sizeof(QVector3D),
                 meshColors.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(1);

    // UV
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, meshUVs.size() * sizeof(QVector2D),
                 meshUVs.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(QVector2D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(2);

    // Normals
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, meshNormals.size() * sizeof(QVector3D),
                 meshNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(QVector3D),
                          reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(3);

    // Unbind VBO and VAO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void Actor::setDiffuseTexture(QImage image)
{
    hasDiffuseTex = true;

    glGenTextures(1, &texDiffuse);
    glBindTexture(GL_TEXTURE_2D, texDiffuse);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    QVector<quint8> imageData = MainView::imageToBytes(image);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, imageData.data());
}

void Actor::setEmissionTexture(QImage image)
{
    hasEmissionTex = true;

    glGenTextures(1, &texEmission);
    glBindTexture(GL_TEXTURE_2D, texEmission);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    QVector<quint8> imageData = MainView::imageToBytes(image);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image.width(), image.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, imageData.data());
}

void Actor::paint(
    QMatrix4x4 &viewTransform, QMatrix4x4 &projectionTransform, float time)
{
    shaderProgram.bind();
    shaderProgram.setUniformValue("view", viewTransform);
    shaderProgram.setUniformValue("projection", projectionTransform);
    shaderProgram.setUniformValue("model", transform);
    shaderProgram.setUniformValue("normalMatrix",
                                  transform.normalMatrix());
    shaderProgram.setUniformValue("time", time);

    if (hasDiffuseTex)
    {
        shaderProgram.setUniformValue("hasDiffuseTex", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texDiffuse);
        shaderProgram.setUniformValue("texDiffuse", 0);
    }
    else
    {
        shaderProgram.setUniformValue("hasDiffuseTex", false);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (hasEmissionTex)
    {
        shaderProgram.setUniformValue("hasEmissionTex", true);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texEmission);
        shaderProgram.setUniformValue("texEmission", 1);
    }
    else
    {
        shaderProgram.setUniformValue("hasEmissionTex", false);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, meshSize);
    shaderProgram.release();
}