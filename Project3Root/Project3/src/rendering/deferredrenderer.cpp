#include "deferredrenderer.h"
#include "miscsettings.h"
#include "ecs/scene.h"
#include "ecs/camera.h"
#include "resources/material.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/shaderprogram.h"
#include "resources/resourcemanager.h"
#include "framebufferobject.h"
#include "gl.h"
#include "globals.h"
#include <QVector>
#include <QVector3D>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>

DeferredRenderer::DeferredRenderer() :
    fboColor(QOpenGLTexture::Target2D),
    fboDepth(QOpenGLTexture::Target2D),
    gboPosition(QOpenGLTexture::Target2D),
    gboAlbedoSpec(QOpenGLTexture::Target2D),
    gboNormal(QOpenGLTexture::Target2D)
{
    fbo = nullptr;

    // List of Textures
    addTexture("Final Render");
    addTexture("Position");
    addTexture("Normals");
    addTexture("AlbedoSpec");
    addTexture("Depth");
}

DeferredRenderer::~DeferredRenderer()
{
    delete fbo;
}

void DeferredRenderer::initialize()
{
    OpenGLErrorGuard guard("DeferredRenderer::initialize()");

    gpassProgram = resourceManager->createShaderProgram();
    gpassProgram->name = "Geometry Pass";
    gpassProgram->vertexShaderFilename = "res/shaders/gpass.vert";
    gpassProgram->fragmentShaderFilename = "res/shaders/gpass.frag";
    gpassProgram->includeForSerialization = false;

    // Deferred Program Creation
    // Vertex Shader seems to be the same as Blit Program, receive a QUAD and draw accordingly
    deferredProgram = resourceManager->createShaderProgram();
    deferredProgram->name = "Deferred Shading";
    deferredProgram->vertexShaderFilename = "res/shaders/deferred_shading.vert";
    deferredProgram->fragmentShaderFilename = "res/shaders/deferred_shading.frag";
    deferredProgram->includeForSerialization = false;

    // Blit Program remains unchanged, you will only send a 2D texture and it will draw it
    blitProgram = resourceManager->createShaderProgram();
    blitProgram->name = "Blit";
    blitProgram->vertexShaderFilename = "res/shaders/blit.vert";
    blitProgram->fragmentShaderFilename = "res/shaders/blit.frag";
    blitProgram->includeForSerialization = false;

    // Create the main FBO
    fbo = new FramebufferObject;
    fbo->create();
}

void DeferredRenderer::finalize()
{
    fbo->destroy();
    delete fbo;
}

void DeferredRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("DeferredRenderer::resize()");

    // Regenerate Render Targets

    // Main Render Targets (Color and Depth)
    if (fboColor == 0) gl->glDeleteTextures(1, &fboColor);
    gl->glGenTextures(1, &fboColor);
    gl->glBindTexture(GL_TEXTURE_2D, fboColor);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    if (fboDepth == 0) gl->glDeleteTextures(1, &fboDepth);
    gl->glGenTextures(1, &fboDepth);
    gl->glBindTexture(GL_TEXTURE_2D, fboDepth);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    // Custom Render Targets for GPass
    if (gboPosition == 0) gl->glDeleteTextures(1, &gboPosition);
    gl->glGenTextures(1, &gboPosition);
    gl->glBindTexture(GL_TEXTURE_2D, gboPosition);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_FLOAT, nullptr);

    if (gboNormal == 0) gl->glDeleteTextures(1, &gboNormal);
    gl->glGenTextures(1, &gboNormal);
    gl->glBindTexture(GL_TEXTURE_2D, gboNormal);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_FLOAT, nullptr);

    if (gboAlbedoSpec == 0) gl->glDeleteTextures(1, &gboAlbedoSpec);
    gl->glGenTextures(1, &gboAlbedoSpec);
    gl->glBindTexture(GL_TEXTURE_2D, gboAlbedoSpec);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // Attach textures to the FBO
    fbo->bind();
    // Here we are attaching all needed textures
    // This should be faster but more expensive in memory
    fbo->addColorAttachment(0, fboColor);
    fbo->addColorAttachment(1, gboPosition);
    fbo->addColorAttachment(2, gboNormal);
    fbo->addColorAttachment(3, gboAlbedoSpec);
    fbo->addDepthAttachment(fboDepth);
    fbo->checkStatus();
    fbo->release();
}

void DeferredRenderer::render(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::render()");

    // Bind the framebuffer used to do the computations
    fbo->bind();

    // Clear color
    gl->glClearDepth(1.0);
    gl->glClearColor(miscSettings->backgroundColor.redF(),
                     miscSettings->backgroundColor.greenF(),
                     miscSettings->backgroundColor.blueF(),
                     1.0);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Passes
    passMeshes(camera);

    passLighting();

    fbo->release();

    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    passBlit();
}
void DeferredRenderer::passMeshes(Camera *camera)
{
    QOpenGLShaderProgram &program = gpassProgram->program;

    if(program.bind())
    {
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

        QVector<MeshRenderer*> meshRenderers;
        QVector<LightSource*> lightSources;
        // Get components
        for (auto entity : scene->entities)
        {
            if (entity->active)
            {
                if (entity->meshRenderer != nullptr) { meshRenderers.push_back(entity->meshRenderer); }
                if (entity->lightSource != nullptr) { lightSources.push_back(entity->lightSource); }
            }
        }

        // Meshes
        for (auto meshRenderer : meshRenderers)
        {
            auto mesh = meshRenderer->mesh;

            if (mesh != nullptr)
            {
                QMatrix4x4 worldMatrix = meshRenderer->entity->transform->matrix();
                QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix;
                QMatrix3x3 normalMatrix = worldViewMatrix.normalMatrix();

                program.setUniformValue("worldMatrix", worldMatrix);
                program.setUniformValue("worldViewMatrix", worldViewMatrix);
                program.setUniformValue("normalMatrix", normalMatrix);

                int materialIndex = 0;
                for (auto submesh : mesh->submeshes)
                {
                    // Get material from the component
                    Material *material = nullptr;
                    if (materialIndex < meshRenderer->materials.size()) {
                        material = meshRenderer->materials[materialIndex];
                    }
                    if (material == nullptr) {
                        material = resourceManager->materialWhite;
                    }
                    materialIndex++;

                    // NIPUTAIDEA
#define SEND_TEXTURE(uniformName, tex1, tex2, texUnit) \
    program.setUniformValue(uniformName, texUnit); \
    if (tex1 != nullptr) { \
    tex1->bind(texUnit); \
                } else { \
    tex2->bind(texUnit); \
                }

                    // Send the material to the shader
                    program.setUniformValue("albedo", material->albedo);
                    program.setUniformValue("emissive", material->emissive);
                    program.setUniformValue("specular", material->specular);
                    program.setUniformValue("smoothness", material->smoothness);
                    program.setUniformValue("bumpiness", material->bumpiness);
                    program.setUniformValue("tiling", material->tiling);
                    SEND_TEXTURE("albedoTexture", material->albedoTexture, resourceManager->texWhite, 0);
                    SEND_TEXTURE("emissiveTexture", material->emissiveTexture, resourceManager->texBlack, 1);
                    SEND_TEXTURE("specularTexture", material->specularTexture, resourceManager->texBlack, 2);
                    SEND_TEXTURE("normalTexture", material->normalsTexture, resourceManager->texNormal, 3);
                    SEND_TEXTURE("bumpTexture", material->bumpTexture, resourceManager->texWhite, 4);

                    submesh->draw();
                }
            }
        }

        // Light spheres
        if (miscSettings->renderLightSources)
        {
            for (auto lightSource : lightSources)
            {
                QMatrix4x4 worldMatrix = lightSource->entity->transform->matrix();
                QMatrix4x4 scaleMatrix; scaleMatrix.scale(0.1f, 0.1f, 0.1f);
                QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix * scaleMatrix;
                QMatrix3x3 normalMatrix = worldViewMatrix.normalMatrix();
                program.setUniformValue("worldMatrix", worldMatrix);
                program.setUniformValue("worldViewMatrix", worldViewMatrix);
                program.setUniformValue("normalMatrix", normalMatrix);

                for (auto submesh : resourceManager->sphere->submeshes)
                {
                    // Send the material to the shader
                    Material *material = resourceManager->materialLight;
                    program.setUniformValue("albedo", material->albedo);
                    program.setUniformValue("emissive", material->emissive);
                    program.setUniformValue("smoothness", material->smoothness);

                    submesh->draw();
                }
            }
        }

        program.release();
    }
}

static void sendLightsToProgram(QOpenGLShaderProgram &program)
{
    QVector<int> lightType;
    QVector<QVector3D> lightPosition;
    QVector<QVector3D> lightDirection;
    QVector<QVector3D> lightColor;
    for (auto entity : scene->entities)
    {
        if (entity->active && entity->lightSource != nullptr)
        {
            auto light = entity->lightSource;
            lightType.push_back(int(light->type));
            // Light Data in Camera Space
            //lightPosition.push_back(QVector3D(viewMatrix * entity->transform->matrix() * QVector4D(0.0, 0.0, 0.0, 1.0)));
            //lightDirection.push_back(QVector3D(viewMatrix * entity->transform->matrix() * QVector4D(0.0, 1.0, 0.0, 0.0)));

            // Light Data in World Space
            lightPosition.push_back(QVector3D(entity->transform->matrix() * QVector4D(0.0, 0.0, 0.0, 1.0)));
            lightDirection.push_back(QVector3D(entity->transform->matrix() * QVector4D(0.0, 1.0, 0.0, 0.0)));

            QVector3D color(light->color.redF(), light->color.greenF(), light->color.blueF());
            lightColor.push_back(color * light->intensity);
        }
    }
    if (lightPosition.size() > 0)
    {
        program.setUniformValueArray("lightType", &lightType[0], lightType.size());
        program.setUniformValueArray("lightPosition", &lightPosition[0], lightPosition.size());
        program.setUniformValueArray("lightDirection", &lightDirection[0], lightDirection.size());
        program.setUniformValueArray("lightColor", &lightColor[0], lightColor.size());


    }
    program.setUniformValue("lightCount", lightPosition.size());

    // Send Camera Position
    program.setUniformValue("camPos", camera->position);
}

void DeferredRenderer::passLighting()
{
    if(shownTexture() != "Final Render")
        return;

    gl->glDisable(GL_DEPTH_TEST);
    QOpenGLShaderProgram &program = deferredProgram->program;

    if(program.bind())
    {
        sendLightsToProgram(program);
        program.setUniformValue("colorTexture", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, fboColor);

        resourceManager->quad->submeshes[0]->draw();
    }
}

void DeferredRenderer::passBlit()
{
    gl->glDisable(GL_DEPTH_TEST);

    QOpenGLShaderProgram &program = blitProgram->program;

    if (program.bind())
    {
        program.setUniformValue("colorTexture", 0);
        gl->glActiveTexture(GL_TEXTURE0);

        if (shownTexture() == "Final Render") {
            gl->glBindTexture(GL_TEXTURE_2D, fboColor);
        }
        else if(shownTexture() == "Depth"){
            gl->glBindTexture(GL_TEXTURE_2D, fboDepth);
        }
        else if(shownTexture() == "Position"){
            gl->glBindTexture(GL_TEXTURE_2D, gboPosition);
        }
        else if(shownTexture() == "Normal"){
            gl->glBindTexture(GL_TEXTURE_2D, gboNormal);
        }
        else if(shownTexture() == "AlbedoSpec"){
            gl->glBindTexture(GL_TEXTURE_2D, gboAlbedoSpec);
        }
        else {
            gl->glBindTexture(GL_TEXTURE_2D, resourceManager->texWhite->textureId());
        }

        resourceManager->quad->submeshes[0]->draw();
    }

    gl->glEnable(GL_DEPTH_TEST);
}
