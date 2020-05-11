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

// https://learnopengl.com/Advanced-Lighting/Deferred-Shading

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
    delete gbo;
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

    gbo = new FramebufferObject;
    gbo->create();
}

void DeferredRenderer::finalize()
{
    fbo->destroy();
    delete fbo;

    gbo->destroy();
    delete gbo;
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
    fbo->addDepthAttachment(fboDepth);
    fbo->checkStatus();
    fbo->release();

    gbo->bind();
    gbo->addColorAttachment(0, gboPosition);
    gbo->addColorAttachment(1, gboNormal);
    gbo->addColorAttachment(2, gboAlbedoSpec);
    gbo->addDepthAttachment(fboDepth);
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    gl->glDrawBuffers(3, attachments);
    gbo->checkStatus();
    gbo->release();
}

void DeferredRenderer::render(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::render()");

    // Bind the framebuffer used to do the computations
    gbo->bind();

    // Clear color
    gl->glClearDepth(1.0);

    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Passes
    passMeshes(camera);

    fbo->bind();
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

        program.setUniformValue("gboPosition", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, gboPosition);

        program.setUniformValue("gboNormal", 1);
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, gboNormal);

        program.setUniformValue("gboAlbedoSpec", 2);
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(GL_TEXTURE_2D, gboAlbedoSpec);

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

void DeferredRenderer::passLighting()
{
    if(shownTexture() != "Final Render")
        return;

    QOpenGLShaderProgram &program = deferredProgram->program;

    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glCullFace(GL_FRONT);

    if(program.bind())
    {
        program.setUniformValue("gboPosition", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, gboPosition);

        program.setUniformValue("gboNormal", 1);
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, gboNormal);

        program.setUniformValue("gboAlbedoSpec", 2);
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(GL_TEXTURE_2D, gboAlbedoSpec);

        program.setUniformValue("ViewPort", QVector2D(camera->viewportWidth, camera->viewportHeight));
        // Get all light indices
        for(auto entity : scene->entities)
        {
            if (entity->active && entity->lightSource != nullptr)
            {
                auto light = entity->lightSource;
                program.setUniformValue("lightPosition", QVector3D(entity->transform->matrix() * QVector4D(0.0, 0.0, 0.0, 1.0)));
                program.setUniformValue("lightDirection", QVector3D(entity->transform->matrix() * QVector4D(0.0, 1.0, 0.0, 0.0)));

                program.setUniformValue("lightType", (GLint)light->type);
                program.setUniformValue("lightRange", light->range);
                program.setUniformValue("lightIntensity", light->intensity);
                program.setUniformValue("Kc", light->kc);
                program.setUniformValue("Kl", light->kl);
                program.setUniformValue("Kq", light->kq);

                program.setUniformValue("lightColor", QVector3D(light->color.redF(), light->color.greenF(), light->color.blueF()));

                // Draw Light onto the Buffers
                if(light->type == LightSource::Type::Point)
                {
                    QMatrix4x4 worldMatrix = entity->transform->matrix();

                    QMatrix4x4 scaleMatrix;
                    float rScale = light->range * 2.;
                    scaleMatrix.scale(rScale,rScale,rScale);
                    QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix * scaleMatrix;

                    program.setUniformValue("worldViewMatrix", worldViewMatrix);
                    program.setUniformValue("projectionMatrix", camera->projectionMatrix);

                    resourceManager->sphere->submeshes[0]->draw();
                    for (auto submesh : resourceManager->sphere->submeshes)
                        submesh->draw();
                }
                else if(light->type == LightSource::Type::Directional)
                {
                   program.setUniformValue("worldViewMatrix", QMatrix4x4());
                   program.setUniformValue("projectionMatrix", QMatrix4x4());

                   resourceManager->quad->submeshes[0]->draw();

                }
            }
        }

    }

    gl->glCullFace(GL_BACK);
    gl->glDisable(GL_BLEND);
    gl->glEnable(GL_DEPTH_TEST);
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
        else if(shownTexture() == "Normals"){
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
