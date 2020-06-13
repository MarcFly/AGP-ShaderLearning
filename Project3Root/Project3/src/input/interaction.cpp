#include "interaction.h"
#include "globals.h"
#include "resources/mesh.h"
#include "rendering/framebufferobject.h"
#include "src/ecs/camera.h"
#include <QtMath>
#include <QVector2D>
#include "resources/shaderprogram.h"




void Interaction::init()
{
    frameBuffer = new FramebufferObject();
    frameBuffer->create();

    mpProgram = resourceManager->createShaderProgram();
    mpProgram->name = "Mouse Picking";
    mpProgram->vertexShaderFilename = "res/shaders/mouse_picking.vert";
    mpProgram->fragmentShaderFilename = "res/shaders/mouse_picking.frag";
    mpProgram->includeForSerialization = false;

}

bool Interaction::update()
{
    bool changed = false;

    switch (state)
    {
    case State::Idle:
        changed = idle();
        render = false;
        break;

    case State::Navigating:
        changed = navigate();
        break;

    case State::Focusing:
        changed = focus();
        break;
    case State::MousePicking:
        //changed = mousePicking();
        render = true;
        emit selection->onClick(selectedEntity);
    }

    return changed;
}

bool Interaction::idle()
{
    if (input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed)
    {
        nextState = State::Navigating;
    }
    else if (input->mouseButtons[Qt::LeftButton] == MouseButtonState::Press)
    {        
        nextState = State::MousePicking;
    }
    else if(selection->count > 0)
    {
        if (input->keys[Qt::Key_F] == KeyState::Press)
        {
            nextState = State::Focusing;
        }
    }

    return false;
}
void Interaction::generateBuffers(int width, int height)
{
    OpenGLErrorGuard guard("GenerateBuffers::render()");
    if (renderTexture >= 0)
    {
        gl->glDeleteTextures(1, &renderTexture);
    }
    gl->glGenTextures(1, &renderTexture);
    gl->glBindTexture(GL_TEXTURE_2D, renderTexture);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);

    if (depthTexture >= 0)
    {
        gl->glDeleteTextures(1, &depthTexture);
    }

    gl->glGenTextures(1, &depthTexture);
    gl->glBindTexture(GL_TEXTURE_2D, depthTexture);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    frameBuffer->bind();
    frameBuffer->addColorAttachment(0, renderTexture);
    frameBuffer->addDepthAttachment(depthTexture);
    frameBuffer->checkStatus();
    frameBuffer->release();
}

void Interaction::passMeshes()
{
    QOpenGLShaderProgram &program = mpProgram->program;

    if (program.bind())
    {        
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

                program.setUniformValue("worldViewMatrix", worldViewMatrix);                
                program.setUniformValue("SelectionCode", meshRenderer->entity->color);

                for (auto submesh : mesh->submeshes)
                {
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
                program.setUniformValue("worldViewMatrix", worldViewMatrix);                
                program.setUniformValue("SelectionCode", lightSource->entity->color);

                for (auto submesh : resourceManager->sphere->submeshes)
                {
                    submesh->draw();
                }
            }
        }

        program.release();
    }
}
bool Interaction::mousePicking()
{
    OpenGLErrorGuard guard("MousePicking::render()");
    selection->clear();
    selectedEntity = nullptr;
    frameBuffer->bind();
    // Enable Blending of Buffers
    gl->glDisable(GL_BLEND);

    // Backface culling and z-test
    gl->glDisable(GL_CULL_FACE);
    gl->glEnable(GL_DEPTH_TEST);

    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    passMeshes();
    GLfloat* pixel = (GLfloat*)malloc(sizeof(GLfloat)*3);
    glReadPixels(input->mousex, camera->viewportHeight - input->mousey, 1, 1, GL_RGB, GL_FLOAT, pixel);

    frameBuffer->release();

    for (int i = 0; i<scene->entities.length();i++)
    {
        if (scene->entities[i]->color.x() == pixel[0] &&
                scene->entities[i]->color.y() == pixel[1] &&
                scene->entities[i]->color.z() == pixel[2])
        {
            selection->onEntitySelectedFromSceneView(scene->entities[i]);
            selectedEntity = scene->entities[i];
        }
    }

    nextState = State::Idle;
    return true;
}

bool Interaction::navigate()
{
    static float v = 0.0f; // Instant speed
    static const float a = 5.0f; // Constant acceleration
    static const float t = 1.0/60.0f; // Delta time

    bool pollEvents = input->mouseButtons[Qt::RightButton] == MouseButtonState::Pressed;
    bool cameraChanged = false;

    // Mouse delta smoothing
    static float mousex_delta_prev[3] = {};
    static float mousey_delta_prev[3] = {};
    static int curr_mousex_delta_prev = 0;
    static int curr_mousey_delta_prev = 0;
    float mousey_delta = 0.0f;
    float mousex_delta = 0.0f;
    if (pollEvents) {
        mousex_delta_prev[curr_mousex_delta_prev] = (input->mousex - input->mousex_prev);
        mousey_delta_prev[curr_mousey_delta_prev] = (input->mousey - input->mousey_prev);
        curr_mousex_delta_prev = curr_mousex_delta_prev % 3;
        curr_mousey_delta_prev = curr_mousey_delta_prev % 3;
        mousex_delta += mousex_delta_prev[0] * 0.33;
        mousex_delta += mousex_delta_prev[1] * 0.33;
        mousex_delta += mousex_delta_prev[2] * 0.33;
        mousey_delta += mousey_delta_prev[0] * 0.33;
        mousey_delta += mousey_delta_prev[1] * 0.33;
        mousey_delta += mousey_delta_prev[2] * 0.33;
    }

    float &yaw = camera->yaw;
    float &pitch = camera->pitch;

    // Camera navigation
    if (mousex_delta != 0 || mousey_delta != 0)
    {
        cameraChanged = true;
        yaw -= 0.5f * mousex_delta;
        pitch -= 0.5f * mousey_delta;
        while (yaw < 0.0f) yaw += 360.0f;
        while (yaw > 360.0f) yaw -= 360.0f;
        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;
    }

    static QVector3D speedVector;
    speedVector *= 0.99;

    bool accelerating = false;
    if (input->keys[Qt::Key_W] == KeyState::Pressed) // Front
    {
        accelerating = true;
        speedVector += QVector3D(-sinf(qDegreesToRadians(yaw)) * cosf(qDegreesToRadians(pitch)),
                                        sinf(qDegreesToRadians(pitch)),
                                        -cosf(qDegreesToRadians(yaw)) * cosf(qDegreesToRadians(pitch))) * a * t;
    }
    if (input->keys[Qt::Key_A] == KeyState::Pressed) // Left
    {
        accelerating = true;
        speedVector -= QVector3D(cosf(qDegreesToRadians(yaw)),
                                        0.0f,
                                        -sinf(qDegreesToRadians(yaw))) * a * t;
    }
    if (input->keys[Qt::Key_S] == KeyState::Pressed) // Back
    {
        accelerating = true;
        speedVector -= QVector3D(-sinf(qDegreesToRadians(yaw)) * cosf(qDegreesToRadians(pitch)),
                                        sinf(qDegreesToRadians(pitch)),
                                        -cosf(qDegreesToRadians(yaw)) * cosf(qDegreesToRadians(pitch))) * a * t;
    }
    if (input->keys[Qt::Key_D] == KeyState::Pressed) // Right
    {
        accelerating = true;
        speedVector += QVector3D(cosf(qDegreesToRadians(yaw)),
                                        0.0f,
                                        -sinf(qDegreesToRadians(yaw))) * a * t;
    }

    if (!accelerating) {
        speedVector *= 0.9;
    }

    camera->position += speedVector * t;

    if (!(pollEvents ||
        speedVector.length() > 0.01f||
        qAbs(mousex_delta) > 0.1f ||
        qAbs(mousey_delta) > 0.1f))
    {
        nextState = State::Idle;
    }

    return true;
}

bool Interaction::focus()
{
    static bool idle = true;
    static float time = 0.0;
    static QVector3D initialCameraPosition;
    static QVector3D finalCameraPosition;
    if (idle) {
        idle = false;
        time = 0.0f;
        initialCameraPosition = camera->position;

        Entity *entity = selection->entities[0];

        float entityRadius = 0.5;
        if (entity->meshRenderer != nullptr && entity->meshRenderer->mesh != nullptr)
        {
            auto mesh = entity->meshRenderer->mesh;
            const QVector3D minBounds = entity->transform->matrix() * mesh->bounds.min;
            const QVector3D maxBounds = entity->transform->matrix() * mesh->bounds.max;
            entityRadius = (maxBounds - minBounds).length();
        }

        QVector3D entityPosition = entity->transform->position;
        QVector3D viewingDirection = QVector3D(camera->worldMatrix * QVector4D(0.0, 0.0, -1.0, 0.0));
        QVector3D displacement = - 1.5 * entityRadius * viewingDirection.normalized();
        finalCameraPosition = entityPosition + displacement;
    }

    const float focusDuration = 0.5f;
    time = qMin(focusDuration, time + 1.0f/60.0f); // TODO: Use frame delta time
    const float t = qPow(qSin(3.14159f * 0.5f * time / focusDuration), 0.5);

    camera->position = (1.0f - t) * initialCameraPosition + t * finalCameraPosition;

    if (t == 1.0f) {
        nextState = State::Idle;
        idle = true;;
    }

    return true;
}

void Interaction::postUpdate()
{
    state = nextState;
}
