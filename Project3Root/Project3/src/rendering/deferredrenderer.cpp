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
#include <Qt3DCore/QTransform>

// https://learnopengl.com/Advanced-Lighting/Deferred-Shading

DeferredRenderer::DeferredRenderer() :
    fboFinal(QOpenGLTexture::Target2D),
    fboEditor(QOpenGLTexture::Target2D),
    fboDepth(QOpenGLTexture::Target2D),
    gboPosition(QOpenGLTexture::Target2D),
    gboAlbedoSpec(QOpenGLTexture::Target2D),
    gboNormal(QOpenGLTexture::Target2D),
    fboEditorDepth(QOpenGLTexture::Target2D),
    fboOutlineMask(QOpenGLTexture::Target2D),
    stepBlur(QOpenGLTexture::Target2D)
{
    fbo = nullptr;
    gbo = nullptr;
    gridbgBO = nullptr;
    outlineBO = nullptr;
    blurDebugBO = nullptr;

    // List of Textures
    addTexture("Final Render");
    addTexture("Editor");
    addTexture("Position");
    addTexture("Normals");
    addTexture("AlbedoSpec");
    addTexture("Depth");
    addTexture("Editor Depth");
    addTexture("Outline Mask");
    addTexture("Mouse Picking");
    addTexture("White");

    // Prepare GL States so that it is less cumbersome to work with
    // Sometimes we will need more than 1 state for pass, generate them and use
    // them appropiately, to make it more readable
    // Specific states (like disabling or enabling test should go in just specific functions

    // Main State
    mainState.depthTest = false;        // No depth testing because by default we use quad rendering
    mainState.depthWrite = false;       // Only what you want affects depth buffers
    mainState.depthFunc = GL_LESS;      // We put Depth at 1., anything before it wins
    mainState.blending = false;         // No blend
    mainState.blendFuncSrc = GL_SRC_ALPHA;    // New Color wins
    mainState.blendFuncDst = GL_ONE_MINUS_SRC_ALPHA;   // Last color is fucked
    mainState.faceCulling = false;       // We don't want double face rendering
    mainState.faceCullingMode = GL_BACK;// Cull back faces because normally unseen

    // Geometry State
    // Always copy mainState then change what is STRICTLY NEEDED
    gpassState = mainState;
    gpassState.depthTest = true;        // We clear out unseen objects
    gpassState.depthFunc = GL_LEQUAL;   // Lequal instead of Less because Z-Fighting, next object wins
    gpassState.depthWrite = true;       // Depth buffer is used in later passes
    gpassState.faceCulling = true;

    // Lighting State

    lightingState = mainState;
    // Depth Test is default false, POINT lights don't occlude each other
    // Depth write obviously false because we don't want to tamper with depth buffer
    lightingState.blending = true;         // No blend
    lightingState.blendFuncSrc = GL_ONE;    // New Color wins
    lightingState.blendFuncDst = GL_ZERO;   // Last color is fucked
    lightingState.faceCulling = true;
    lightingState.faceCullingMode = GL_FRONT;   // We cull what is in front, because we write whats inside the sphere (not before)

    // Grid Background State
    gridbgState = mainState;
    // We don't want depth test, we will do custom test in shader
    // Same with depth write, we don't want that, clean depth buffer
    gridbgState.blending = true; // Mixing Ambient with the grid
    // We use main function because we will be mixing based on alpha

    // Outline State
    outlineState = mainState;
    outlineState.blending = true;

    // Mask State
    maskState = mainState;
    maskState.blending = true;
    maskState.blendFuncSrc = GL_ONE;
    maskState.blendFuncDst = GL_ZERO;
    maskState.depthTest = true;
    maskState.depthWrite = true;
    maskState.depthFunc = GL_LEQUAL;
    maskState.faceCulling = true;

    // Blur State
    blurState = mainState;
    //blurState.blending = true;

}

DeferredRenderer::~DeferredRenderer()
{
    delete fbo;
    delete gbo;
    delete gridbgBO;
    delete maskBO;
    delete outlineBO;
    delete blurDebugBO;
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

    // Grid Background Program
    gridbgProgram = resourceManager->createShaderProgram();
    gridbgProgram->name = "GridBg";
    gridbgProgram->vertexShaderFilename = "res/shaders/blit.vert";
    gridbgProgram->fragmentShaderFilename = "res/shaders/grid-background/bggrid.frag";
    gridbgProgram->includeForSerialization = false;

    // Mask Program
    maskProgram = resourceManager->createShaderProgram();
    maskProgram->name = "Mask";
    maskProgram->vertexShaderFilename = "res/shaders/outline/mask.vert";
    maskProgram->fragmentShaderFilename = "res/shaders/outline/mask.frag";
    maskProgram->includeForSerialization = false;

    // Outline Program
    outlineProgram = resourceManager->createShaderProgram();
    outlineProgram->name = "Outline";
    outlineProgram->vertexShaderFilename = "res/shaders/blit.vert";
    outlineProgram->fragmentShaderFilename = "res/shaders/outline/outline.frag";
    outlineProgram->includeForSerialization = false;

    // Blur Program

    blurProgram = resourceManager->createShaderProgram();
    blurProgram->name = "Blur";
    blurProgram->vertexShaderFilename = "res/shaders/blit.vert";
    blurProgram->fragmentShaderFilename = "res/shaders/blur/gaussian.frag";
    blurProgram->includeForSerialization = false;

    // Create the main FBO
    fbo = new FramebufferObject;
    fbo->create();

    gbo = new FramebufferObject;
    gbo->create();

    gridbgBO = new FramebufferObject;
    gridbgBO->create();

    maskBO = new FramebufferObject;
    maskBO->create();

    outlineBO = new FramebufferObject;
    outlineBO->create();

    blurDebugBO = new FramebufferObject;
    blurDebugBO->create();

    gl->glClearColor(0.,0.,0.,0.);
    gl->glClearDepth(1.0);
}

void DeferredRenderer::finalize()
{
    fbo->destroy();
    delete fbo;

    gbo->destroy();
    delete gbo;

    gridbgBO->destroy();
    delete gridbgBO;

    maskBO->destroy();
    delete maskBO;

    outlineBO->destroy();
    delete outlineBO;

    blurDebugBO->destroy();
    delete blurDebugBO;

}

void DeferredRenderer::fboPrep(int w, int h)
{
    // Main Render Targets (Color and Depth)
    if (fboFinal == 0) gl->glDeleteTextures(1, &fboFinal);
    gl->glGenTextures(1, &fboFinal);
    gl->glBindTexture(GL_TEXTURE_2D, fboFinal);
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

    // Attach textures to the FBO
    fbo->bind();
    // Here we are attaching all needed textures
    // This should be faster but more expensive in memory
    fbo->addColorAttachment(0, fboFinal);
    fbo->addDepthAttachment(fboDepth);
    fbo->checkStatus();
    fbo->release();
}

void DeferredRenderer::gboPrep(int w, int h)
{

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

    if (fboEditor == 0) gl->glDeleteTextures(1, &fboEditor);
    gl->glGenTextures(1, &fboEditor);
    gl->glBindTexture(GL_TEXTURE_2D, fboEditor);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    gbo->bind();
    gbo->addColorAttachment(0, fboFinal);
    gbo->addColorAttachment(1, gboPosition);
    gbo->addColorAttachment(2, gboNormal);
    gbo->addColorAttachment(3, gboAlbedoSpec);
    gbo->addColorAttachment(4, fboEditor);
    gbo->addDepthAttachment(fboDepth);
    unsigned int attachments[5] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    gl->glDrawBuffers(5, attachments);
    gbo->checkStatus();
    gbo->release();

}

void DeferredRenderer::gridbgPrep(int w, int h)
{
    if (fboEditorDepth == 0) gl->glDeleteTextures(1, &fboEditorDepth);
    gl->glGenTextures(1, &fboEditorDepth);
    gl->glBindTexture(GL_TEXTURE_2D, fboEditorDepth);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    // We reuse fboDepth without TESTING OR WRITING
    // We read GL_FragDepth because it is used for determining in fragment
    // if we hit grid or nah
    // If we have something being corrupted, we will use the fboEditorDepth and pass fboDepth for read

    gridbgBO->bind();
    gridbgBO->addColorAttachment(0, fboEditor);
    gridbgBO->addDepthAttachment(fboEditorDepth);
    gridbgBO->checkStatus();
    gridbgBO->release();
}

void DeferredRenderer::outlinePrep(int w, int h)
{
    if (fboOutlineMask == 0) gl->glDeleteTextures(1, &fboOutlineMask);
    gl->glGenTextures(1, &fboOutlineMask);
    gl->glBindTexture(GL_TEXTURE_2D, fboOutlineMask);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_FLOAT, nullptr);

    maskBO->bind();
    maskBO->addColorAttachment(0, fboOutlineMask);
    maskBO->addDepthAttachment(fboEditorDepth);
    maskBO->checkStatus();
    maskBO->release();

    outlineBO->bind();
    outlineBO->addColorAttachment(0, fboEditor);
    outlineBO->addDepthAttachment(fboEditorDepth);
    outlineBO->checkStatus();
    outlineBO->release();

}

void DeferredRenderer::blurDebugPrep(int w, int h)
{
    // We have a specific blurdebugBO so that debug of the blur is written
    // in the editor and the specific passes that use a blur, are written
    // in the desired texture
    // For that, PassBlur receives a FramebufferObject with a
    // - Write Texture / Depth Texture / Step Blur Texture
    // Everything else is passed as read

    if (stepBlur == 0) gl->glDeleteTextures(1, &stepBlur);
    gl->glGenTextures(1, &stepBlur);
    gl->glBindTexture(GL_TEXTURE_2D, stepBlur);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    blurDebugBO->bind();
    blurDebugBO->addColorAttachment(0, fboEditor);
    blurDebugBO->addColorAttachment(1, stepBlur);
    blurDebugBO->addDepthAttachment(fboEditorDepth);
    // In contrast to Geometry Pass, we don't decide here which buffer si being written
    // Or which buffers are written, it is done in 2 passes of blur shader
    blurDebugBO->checkStatus();
    blurDebugBO->release();
}

void DeferredRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("DeferredRenderer::resize()");

    fboPrep(w,h);
    gboPrep(w,h);

    gridbgPrep(w,h);

    outlinePrep(w,h);

    blurDebugPrep(w,h);
    // Debug Preps

}

void DeferredRenderer::CleanFirstBuffers()
{
    //  Here we perform a pre-clean on buffers that use textures that can't be cleaned
    //  when executing the shader as it is reusing them
    //  With this we perform a clean after they have been written to

    //  Does not work with buffers that do not reuse textures, which have to be cleaned at
    //  execution time,  I don't know why but that is the way OpenGL shows us issues

    if(shownTexture() == "Final Render")
    {
    fbo->bind();
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    fbo->release();
    }
    else
    {

    gridbgBO->bind();
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gridbgBO->release();

    outlineBO->bind();
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    outlineBO->release();

    blurDebugBO->bind();
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    blurDebugBO->release();
    }
}

void DeferredRenderer::render(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::render()");

    CleanFirstBuffers();

    // Passes
    passMeshes(camera);

    if(shownTexture() == "Final Render")
    {
        passLighting(camera);
    }
    else // I do else to get every debug things without worrying if it would be seen
    {
        PassGridBackground(camera);
        PassOutline(camera);
        PassBlur(blurDebugBO, fboEditor);
    }

    passBlit();
}

void DeferredRenderer::PassBlur(FramebufferObject* fb, GLuint Read, GLuint Mask)
{
    if(fb == nullptr) return;

    Mask = (Mask == UINT_MAX) ? resourceManager->texWhite->textureId() : Mask;

    OpenGLErrorGuard guard("DeferredRenderer::PassBlur()");

    mainState.apply();

    fb->bind();
    QOpenGLShaderProgram &program = blurProgram->program;
    if(program.bind())
    {
        // Prepare uniforms, we will bind textures later
        program.setUniformValue("colorMap", 0);
        program.setUniformValue("Mask", 1);
        program.setUniformValue("ratio", miscSettings->blurVal);

        // Pass 1 - Write to 1st Step of Blur (Vertical or Horizontal, does not matter)

        QVector2D dir(0.,1.);
        gl->glDrawBuffer(GL_COLOR_ATTACHMENT1);
        program.setUniformValue("dir", dir.x(), dir.y());

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, Read);

        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, Mask);

        resourceManager->quad->submeshes[0]->draw();

        // Pass 2 - Write to Expected (Attachment 0) and Read from the previous write
        // Also, direction should be transpose of the last one

        gl->glDrawBuffer(GL_COLOR_ATTACHMENT0);
        program.setUniformValue("dir", dir.y(), dir.x());

        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, stepBlur);

        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, Mask);

        resourceManager->quad->submeshes[0]->draw();



        fb->release();
    }


}

void DeferredRenderer::PassOutline(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::PassOutline()");

    if(selection->entities[0] == nullptr) return;

    maskState.apply();
    maskBO->bind();
    {
        gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        QOpenGLShaderProgram &program = maskProgram->program;
        if(program.bind())
        {
            program.setUniformValue("projectionMatrix", camera->projectionMatrix);

            for(int i = 0; selection->entities[i] != nullptr && i < MAX_SELECTED_ENTITIES; ++i)
            {
                Entity* et = selection->entities[i];
                if (et->meshRenderer != nullptr) {

                    QMatrix4x4 worldViewMatrix = camera->viewMatrix * et->transform->matrix();
                    program.setUniformValue("worldViewMatrix", worldViewMatrix);

                    for (auto submesh : et->meshRenderer->mesh->submeshes)
                    {

                                submesh->draw();
                    }
                }

            }
        }


        maskBO->release();
    }

    if(!miscSettings->renderOutline) return;

    outlineState.apply();
    outlineBO->bind();
    {
        QOpenGLShaderProgram &program = outlineProgram->program;
        if(program.bind())
        {
            program.setUniformValue("mask", 0);
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, fboOutlineMask);

            program.setUniformValue("alpha", miscSettings->OutlineAlpha);
            program.setUniformValue("width", miscSettings->OutlineWidth);

            resourceManager->quad->submeshes[0]->draw();
        }

        outlineBO->release();
    }
}

void DeferredRenderer::PassGridBackground(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::PassGridBackground()");

    if(!miscSettings->renderGrid) return;
    gridbgState.apply();

    gridbgBO->bind();

    // Color will be reused from fboEditor and blended in, so not cleared
    gl->glClear(GL_DEPTH_BUFFER_BIT);

    QOpenGLShaderProgram &program = gridbgProgram->program;
    if(program.bind())
    {
        program.setUniformValue("camParams", camera->getLeftRightBottomTop());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("worldMatrix", camera->worldMatrix);
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);


        program.setUniformValue("bgCol", miscSettings->backgroundColor.redF(), miscSettings->backgroundColor.greenF(),miscSettings->backgroundColor.blueF());
        program.setUniformValue("gridCol", miscSettings->gridColor.redF(), miscSettings->gridColor.greenF(),miscSettings->gridColor.blueF());

        program.setUniformValue("depth", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, fboDepth);

        resourceManager->quad->submeshes[0]->draw();
    }

    gridbgBO->release();
}

void DeferredRenderer::passMeshes(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::PassMeshes()");

    gpassState.apply();

    gbo->bind();

    QOpenGLShaderProgram &program = gpassProgram->program;

    // Clear before doing anything to a buffer
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(program.bind())
    {

        program.setUniformValue("outColor", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, fboFinal);

        program.setUniformValue("gboPosition", 1);
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, gboPosition);

        program.setUniformValue("gboNormal", 2);
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(GL_TEXTURE_2D, gboNormal);

        program.setUniformValue("gboAlbedoSpec", 3);
        gl->glActiveTexture(GL_TEXTURE3);
        gl->glBindTexture(GL_TEXTURE_2D, gboAlbedoSpec);

        program.setUniformValue("fboEditor", 4);
        gl->glActiveTexture(GL_TEXTURE4);
        gl->glBindTexture(GL_TEXTURE_2D, fboEditor);


        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);
        program.setUniformValue("AMBIENT", miscSettings->AMBIENT);
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

    gbo->release();
}

void DeferredRenderer::passLighting(Camera* camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::PassLighting()");

    fbo->bind();

    QOpenGLShaderProgram &program = deferredProgram->program;

    lightingState.apply();

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

        program.setUniformValue("ViewPort", camera->viewportWidth, camera->viewportHeight);
        program.setUniformValue("camPos", camera->position);
        program.setUniformValue("AMBIENT", miscSettings->AMBIENT);

        // Get all light indices
        for(auto entity : scene->entities)
        {
            if (entity->active && entity->lightSource != nullptr)
            {
                auto light = entity->lightSource;
                program.setUniformValue("lightPosition",  QVector3D(entity->transform->position));// * QVector4D(0.,0.,0.,1.)));
                program.setUniformValue("lightDirection", QVector3D(entity->transform->matrix() * QVector4D(0.0, 1.0, 0.0, 0.0)));


                program.setUniformValue("lightType", (GLint)light->type);
                program.setUniformValue("lightRange", light->range);
                program.setUniformValue("lightIntensity", light->intensity);




                program.setUniformValue("lightColor", QVector3D(light->color.redF(), light->color.greenF(), light->color.blueF()));

                // Draw Light onto the Buffers
                if(light->type == LightSource::Type::Point)
                {
                    gl->glDisable(GL_DEPTH_TEST);
                    gl->glEnable(GL_CULL_FACE);
                    QMatrix4x4 worldMatrix = entity->transform->matrix();

                    QMatrix4x4 scaleMatrix;
                    float rScale = light->range;
                    scaleMatrix.scale(rScale,rScale,rScale);
                    QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix * scaleMatrix;

                    program.setUniformValue("worldViewMatrix", worldViewMatrix);
                    program.setUniformValue("projectionMatrix", camera->projectionMatrix);

                    for (auto submesh : resourceManager->sphere->submeshes)
                        submesh->draw();
                }
                else if(light->type == LightSource::Type::Directional)
                {
                   gl->glEnable(GL_DEPTH_TEST);
                   gl->glDisable(GL_CULL_FACE);
                   program.setUniformValue("worldViewMatrix", QMatrix4x4());
                   program.setUniformValue("projectionMatrix", QMatrix4x4());

                   resourceManager->quad->submeshes[0]->draw();

                }
            }
        }

    }

    fbo->release();
}

void DeferredRenderer::passBlit()
{
    QOpenGLShaderProgram &program = blitProgram->program;
    // We clear default buffer at blit because now we clear at every buffer bind
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mainState.apply();

    if (program.bind())
    {
        program.setUniformValue("blitAlpha", false);
        program.setUniformValue("blitDepth", false);

        program.setUniformValue("colorTexture", 0);
        gl->glActiveTexture(GL_TEXTURE0);

        if (shownTexture() == "Final Render") {
            gl->glBindTexture(GL_TEXTURE_2D, fboFinal);
        }
        else if(shownTexture() == "Editor"){
            gl->glBindTexture(GL_TEXTURE_2D, fboEditor);
        }
        else if(shownTexture() == "Depth"){
            program.setUniformValue("blitDepth", true);
            gl->glBindTexture(GL_TEXTURE_2D, fboDepth);
        }
        else if(shownTexture() == "Editor Depth"){
            program.setUniformValue("blitDepth", true);
            gl->glBindTexture(GL_TEXTURE_2D, fboEditorDepth);
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
        else if(shownTexture() == "Outline Mask")
        {
            gl->glBindTexture(GL_TEXTURE_2D, fboOutlineMask);
        }
        else if(shownTexture() == "Mouse Picking")
        {
            gl->glBindTexture(GL_TEXTURE_2D, interaction->renderTexture);
        }
        else {
            gl->glBindTexture(GL_TEXTURE_2D, resourceManager->texWhite->textureId());
        }

        resourceManager->quad->submeshes[0]->draw();
    }

    gl->glEnable(GL_DEPTH_TEST);
}
