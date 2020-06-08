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
    fboColor(QOpenGLTexture::Target2D),
    fboDepth(QOpenGLTexture::Target2D),
    gboPosition(QOpenGLTexture::Target2D),
    gboAlbedoSpec(QOpenGLTexture::Target2D),
    gboNormal(QOpenGLTexture::Target2D),
    fboMask(QOpenGLTexture::Target2D)
{
    fbo = nullptr;
    gbo = nullptr;
    lbo = nullptr;
    mbo = nullptr;
    obo = nullptr;

    // List of Textures
    addTexture("Final Render");
    addTexture("Position");
    addTexture("Normals");
    addTexture("AlbedoSpec");
    addTexture("Depth");
    addTexture("Ambient");
    addTexture("LightSpheres");
    addTexture("MaskSelected");
    addTexture("blurDebug");
    addTexture("Mouse Picking");
    addTexture("DOFMask");
}

DeferredRenderer::~DeferredRenderer()
{
    delete fbo;
    delete gbo;
    delete lbo;
    delete mbo;
    delete obo;
    delete gbbo;
    delete gblurbo;
    delete dofbo;
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

    // Debug Light Program, Draw white diffuminated by expected attenuation to get a sense of what the lights are doing
    debugSpheres = resourceManager->createShaderProgram();
    debugSpheres->name = "DebugLights";
    debugSpheres->vertexShaderFilename = "res/shaders/deferred_shading.vert";
    debugSpheres->fragmentShaderFilename = "res/shaders/debug_light.frag";
    debugSpheres->includeForSerialization = false;


    // Masking Program, tests selected geometry and wirtes a white texture where they are
    maskProgram = resourceManager->createShaderProgram();
    maskProgram->name = "Mask";
    maskProgram->vertexShaderFilename = "res/shaders/outline/mask.vert";
    maskProgram->fragmentShaderFilename = "res/shaders/outline/mask.frag";
    maskProgram->includeForSerialization = false;

    // Masking Program, tests selected geometry and wirtes a white texture where they are
    outlineProgram = resourceManager->createShaderProgram();
    outlineProgram->name = "Outline";
    outlineProgram->vertexShaderFilename = "res/shaders/outline/outline.vert";
    outlineProgram->fragmentShaderFilename = "res/shaders/outline/outline.frag";
    outlineProgram->includeForSerialization = false;

    // Background & Grid program, writes over final scene the grid, testing by depth
    bggridProgram = resourceManager->createShaderProgram();
    bggridProgram->name = "Grid";
    bggridProgram->vertexShaderFilename = "res/shaders/grid-background/bggrid.vert";
    bggridProgram->fragmentShaderFilename = "res/shaders/grid-background/bggrid.frag";
    bggridProgram->includeForSerialization = false;

    // Gaussian Blur Program, blur defined by a 11x11 kernel (fixed for now)
    // Would be great to have a custom kernel blur system, but not for now

    gaussianblurProgram = resourceManager->createShaderProgram();
    gaussianblurProgram->name = "Grid";
    gaussianblurProgram->vertexShaderFilename = "res/shaders/blur/blur.vert";
    gaussianblurProgram->fragmentShaderFilename = "res/shaders/blur/gaussian.frag";
    gaussianblurProgram->includeForSerialization = false;

    // Depth of Field Program
    // Have to add "Real" Camera controls, depending on fov, focus distance and falloff
    // A great addition would be to pass kernel values for the blur (bigger kernel = softer blur and such);
    depthOfFieldProgram = resourceManager->createShaderProgram();
    depthOfFieldProgram->name = "DOF";
    depthOfFieldProgram->vertexShaderFilename = "res/shaders/depth-of-field/dof.vert";
    depthOfFieldProgram->fragmentShaderFilename = "res/shaders/depth-of-field/dof.frag";
    depthOfFieldProgram->includeForSerialization = false;

    // Create the main FBO
    fbo = new FramebufferObject;
    fbo->create();

    gbo = new FramebufferObject;
    gbo->create();

    lbo = new FramebufferObject;
    lbo->create();

    mbo = new FramebufferObject;
    mbo->create();

    obo = new FramebufferObject;
    obo->create();

    gbbo = new FramebufferObject;
    gbbo->create();

    gblurbo = new FramebufferObject;
    gblurbo->create();

    dofbo = new FramebufferObject;
    dofbo->create();
}

void DeferredRenderer::finalize()
{
    fbo->destroy();
    delete fbo;

    gbo->destroy();
    delete gbo;

    lbo->destroy();
    delete lbo;

    mbo->destroy();
    delete mbo;

    obo->destroy();
    delete obo;

    gbbo->destroy();
    delete gbbo;

    gblurbo->destroy();
    delete gblurbo;

    dofbo->destroy();
    delete dofbo;
}

void DeferredRenderer::gblurboPrep(int w, int h)
{

    if (blurDebug == 0) gl->glDeleteTextures(1, &blurDebug);
    gl->glGenTextures(1, &blurDebug);
    gl->glBindTexture(GL_TEXTURE_2D, blurDebug);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);


    // Reads from a specific texture to later write in to defined output
    gblurbo->bind();
    // This is initialized as such as we just want to add color attachments,
    // It will depend on the texture used to generate the blur and the output
    // With 2nd Texture, we can pass masks that affect the blur intensity
    gblurbo->addColorAttachment(0, blurDebug);
    gblurbo->addColorAttachment(1, fboColor);
    gblurbo->addColorAttachment(2, fboMask);
    gblurbo->addDepthAttachment(fboDepth);
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,GL_COLOR_ATTACHMENT2};
    gl->glDrawBuffers(3, attachments);
    gblurbo->checkStatus();
    gblurbo->release();
}

void DeferredRenderer::gbboPrep(int w, int h)
{
    gbbo->bind();
    gbbo->addColorAttachment(0, fboColor);
    gbbo->addDepthAttachment(fboDepth);
    gbbo->checkStatus();
    gbbo->release();
}

void DeferredRenderer::fboPrep(int w, int h)
{
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

    // Attach textures to the FBO
    fbo->bind();
    // Here we are attaching all needed textures
    // This should be faster but more expensive in memory
    fbo->addColorAttachment(0, fboColor);
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

    if (fboAmbient == 0) gl->glDeleteTextures(1, &fboAmbient);
    gl->glGenTextures(1, &fboAmbient);
    gl->glBindTexture(GL_TEXTURE_2D, fboAmbient);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    gbo->bind();
    gbo->addColorAttachment(0, fboColor);
    gbo->addColorAttachment(1, gboPosition);
    gbo->addColorAttachment(2, gboNormal);
    gbo->addColorAttachment(3, gboAlbedoSpec);
    gbo->addColorAttachment(4, fboAmbient);
    gbo->addDepthAttachment(fboDepth);
    unsigned int attachments[5] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4};
    gl->glDrawBuffers(5, attachments);
    gbo->checkStatus();
    gbo->release();

}

void DeferredRenderer::lboPrep(int w, int h)
{

    if (lightSpheres == 0) gl->glDeleteTextures(1, &lightSpheres);
    gl->glGenTextures(1, &lightSpheres);
    gl->glBindTexture(GL_TEXTURE_2D, lightSpheres);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    lbo->bind();
    lbo->addColorAttachment(0, lightSpheres);
    lbo->addDepthAttachment(fboDepth);
    lbo->checkStatus();
    lbo->release();

}

void DeferredRenderer::mboPrep(int w, int h)
{
    if(fboMask == 0) gl->glDeleteTextures(1, &fboMask);
    gl->glGenTextures(1, &fboMask);
    gl->glBindTexture(GL_TEXTURE_2D, fboMask);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    if (fboDepthMask == 0) gl->glDeleteTextures(1, &fboDepthMask);
    gl->glGenTextures(1, &fboDepthMask);
    gl->glBindTexture(GL_TEXTURE_2D, fboDepthMask);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    // Attach textures to the FBO
    mbo->bind();
    // Here we are attaching all needed textures
    // This should be faster but more expensive in memory
    mbo->addColorAttachment(0, fboMask);
    mbo->addDepthAttachment(fboDepthMask);
    mbo->checkStatus();
    mbo->release();

    obo->bind();
    obo->addColorAttachment(0, fboColor);
    obo->addColorAttachment(1, fboMask);
    obo->addDepthAttachment(fboDepthMask);
    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    gl->glDrawBuffers(2, attachments);
    obo->checkStatus();
    obo->release();

}

void DeferredRenderer::dofPrep(int w, int h)
{
    if(dofMask == 0) gl->glDeleteTextures(1, &dofMask);
    gl->glGenTextures(1, &dofMask);
    gl->glBindTexture(GL_TEXTURE_2D, dofMask);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    dofbo->bind();
    dofbo->addColorAttachment(0, dofMask);
    dofbo->addDepthAttachment(fboDepth);
    dofbo->checkStatus();
    dofbo->release();
}

void DeferredRenderer::resize(int w, int h)
{
    OpenGLErrorGuard guard("DeferredRenderer::resize()");

    fboPrep(w,h);
    gbboPrep(w,h);
    gboPrep(w,h);
    dofPrep(w,h);


    // Debug Preps
    lboPrep(w, h);
    mboPrep(w,h);

    // Gaussian blur uses fboMask, needs to clean and create first
    gblurboPrep(w,h);


}

void DeferredRenderer::cleanBuffers()
{
    // Clean Default
    gl->glClearDepth(1.0);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Clean Grid Background
    gbbo->bind();
    gl->glClearDepth(1.0);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gbbo->release();

    // Clear Geometry
    gbo->bind();
    gl->glClearDepth(1.0);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gbo->release();

    // Clear Lighting

    // Clear Blur fbo
    gblurbo->bind();
    gl->glClearDepth(1.0);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gblurbo->release();

    // Clear Depth of Field bo
    dofbo->bind();
    gl->glClearDepth(1.0);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    dofbo->release();



    // Clear Outline

}

void DeferredRenderer::render(Camera *camera)
{
    OpenGLErrorGuard guard("DeferredRenderer::render()");

    // Clean Buffers () { clean all buffers};
    cleanBuffers();

    // Passes

    passMeshes(camera);

    passLighting();

    if(shownTexture() == "LightSpheres")
        passDebugLights();

    passDepthOfField();

    // Outline will always be just before the blit
    // Else it would be overwritten by other effects


    passGridBackground(camera);
    passOutline(camera);

    if(shownTexture() == "blurDebug")
        passBlurDebug();

    passBlit();
}

void DeferredRenderer::passDepthOfField()
{

    OpenGLErrorGuard guard("DeferredRenderer::passDepthOfField()");
    dofbo->bind();

    gl->glDisable(GL_DEPTH_TEST);
    gl->glDepthMask(GL_FALSE);
    QOpenGLShaderProgram &program = depthOfFieldProgram->program;
    if(program.bind())
    {
        program.setUniformValue("WriteMask", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, dofMask);

        program.setUniformValue("FocusDist", miscSettings->dofFocus);
        program.setUniformValue("FocusDepth", miscSettings->dofDepth);
        program.setUniformValue("FocusFalloff", miscSettings->dofFalloff);
        program.setUniformValue("zfar", camera->zfar);
        program.setUniformValue("viewP", camera->viewportWidth, camera->viewportHeight);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("camPos", camera->position);

        resourceManager->quad->submeshes[0]->draw();
    }

    dofbo->release();

    passBlur(fboColor, fboColor);//, dofMask);

    gl->glEnable(GL_DEPTH_TEST);

    gl->glDepthMask(GL_TRUE);

}

void DeferredRenderer::passBlur(GLuint WriteCol, GLuint ReadCol, GLuint Mask)
{
    if(Mask == UINT_MAX) Mask = resourceManager->texWhite->textureId();

    gl->glDisable(GL_DEPTH_TEST);
    gl->glDepthMask(GL_FALSE);

    gblurbo->bind();
    // This is initialized as such as we just want to add color attachments,
    // It will depend on the texture used to generate the blur and the output
    // With 2nd Texture, we can pass masks that affect the blur intensity

    gblurbo->addColorAttachment(0, WriteCol);
    gblurbo->addColorAttachment(1, ReadCol);
    gblurbo->addColorAttachment(2, Mask);
    //gblurbo->addDepthAttachment(fboDepth);
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    gl->glDrawBuffers(3, attachments);
    gblurbo->checkStatus();


    QOpenGLShaderProgram &program = gaussianblurProgram->program;
    if(program.bind())
    {
        program.setUniformValue("outColor", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, WriteCol);

        program.setUniformValue("colorMap", 1);
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, ReadCol);

        program.setUniformValue("Mask", 2);
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(GL_TEXTURE_2D, Mask);

        program.setUniformValue("ratio", miscSettings->blurVal);
        program.setUniformValue("viewP", camera->viewportWidth, camera->viewportHeight);

        // Vertical Pass
        program.setUniformValue("dir", 0., 1.);
        resourceManager->quad->submeshes[0]->draw();


        // Horizontal Pass
        program.setUniformValue("dir", 1., 0.);
        resourceManager->quad->submeshes[0]->draw();
    }
    gblurbo->release();

    gl->glDepthMask(GL_TRUE);
    gl->glEnable(GL_DEPTH_TEST);
}

void DeferredRenderer::passBlurDebug()
{
    // To make things easier and not make it overcomplicated
    if(!miscSettings->globalBlur && miscSettings->blurVal > 0.) return;

    passBlur(blurDebug, fboColor);
}

void DeferredRenderer::passGridBackground(Camera* camera)
{
    if(!miscSettings->renderGrid) return;

    gbbo->bind();

    gl->glEnable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    gl->glDepthFunc(GL_LEQUAL);

    QOpenGLShaderProgram& program = bggridProgram->program;
    if(program.bind())
    {
        program.setUniformValue("camParams", camera->getLeftRightBottomTop());
        program.setUniformValue("znear", camera->znear);
        program.setUniformValue("worldMatrix", camera->worldMatrix);
        program.setUniformValue("viewMatrix", camera->viewMatrix);
        program.setUniformValue("projectionMatrix", camera->projectionMatrix);

        resourceManager->quad->submeshes[0]->draw();

        program.release();
    }
    gbbo->release();

    gl->glDisable(GL_BLEND);
    gl->glDisable(GL_DEPTH_TEST);
}

void DeferredRenderer::passOutline(Camera *camera)
{
    // We try to not attach a depth mask, then we have to disable write to a depth mask

    if(selection->entities[0] == nullptr ||  !miscSettings->renderOutline) return;

    mbo->bind();
    gl->glClearColor(0.,0.,0.,0.);
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    {
        // Create mask for the desired object
        QOpenGLShaderProgram &program = maskProgram->program;
        if(program.bind())
        {
            program.setUniformValue("outColor", 0);
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, fboMask);

            program.setUniformValue("projectionMatrix", camera->projectionMatrix);

            for(int i = 0; selection->entities[i] != nullptr && i < MAX_SELECTED_ENTITIES; ++i)
            {
                Entity* et = selection->entities[i];
                if(et->meshRenderer != nullptr)
                {
                    // Data to get position properly

                    QMatrix4x4 worldViewMatrix = camera->viewMatrix * et->transform->matrix();
                    program.setUniformValue("worldViewMatrix", worldViewMatrix);

                    auto mr = et->meshRenderer->mesh;
                    for(auto submesh : mr->submeshes)
                    {
                        // at later stages, it should search for material to get bump map probably
                        // Normal is required for it? idk, will add later if required

                        submesh->draw();
                    }
                }

            }
        }
    }
    mbo->release();

    // Prepare glState to render outline
    gl->glDisable(GL_DEPTH_TEST);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    obo->bind();
    // Then generate outline based on the mask
    {
        QOpenGLShaderProgram &program = outlineProgram->program;
        if(program.bind())
        {
            program.setUniformValue("outColor", 0);
            gl->glActiveTexture(GL_TEXTURE0);
            gl->glBindTexture(GL_TEXTURE_2D, fboColor);

            program.setUniformValue("mask", 1);
            gl->glActiveTexture(GL_TEXTURE1);
            gl->glBindTexture(GL_TEXTURE_2D, fboMask);

            program.setUniformValue("viewP", camera->viewportWidth, camera->viewportHeight);

            program.setUniformValue("width", miscSettings->OutlineWidth);
            program.setUniformValue("alpha", miscSettings->OutlineAlpha);

            resourceManager->quad->submeshes[0]->draw();
        }
    }

    obo->release();

    gl->glDisable(GL_BLEND);
    gl->glBlendFunc(GL_ONE,GL_ONE);
    gl->glEnable(GL_DEPTH_TEST);
}

void DeferredRenderer::passMeshes(Camera *camera)
{
    gbo->bind();

    QOpenGLShaderProgram &program = gpassProgram->program;
    gl->glDepthFunc(GL_LEQUAL);
    gl->glDepthMask(GL_TRUE);
    gl->glEnable(GL_BLEND);
    gl->glBlendFunc(GL_ONE, GL_ZERO);

    if(program.bind())
    {

        program.setUniformValue("outColor", 0);
        gl->glActiveTexture(GL_TEXTURE0);
        gl->glBindTexture(GL_TEXTURE_2D, fboColor);

        program.setUniformValue("gboPosition", 1);
        gl->glActiveTexture(GL_TEXTURE1);
        gl->glBindTexture(GL_TEXTURE_2D, gboPosition);

        program.setUniformValue("gboNormal", 2);
        gl->glActiveTexture(GL_TEXTURE2);
        gl->glBindTexture(GL_TEXTURE_2D, gboNormal);

        program.setUniformValue("gboAlbedoSpec", 3);
        gl->glActiveTexture(GL_TEXTURE3);
        gl->glBindTexture(GL_TEXTURE_2D, gboAlbedoSpec);

        program.setUniformValue("fboAmbient", 4);
        gl->glActiveTexture(GL_TEXTURE4);
        gl->glBindTexture(GL_TEXTURE_2D, fboAmbient);


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

    gbo->release();
}

void DeferredRenderer::passLighting()
{
    if(shownTexture() != "Final Render" && shownTexture() != "LightSpheres")
        return;

    fbo->bind();

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
                program.setUniformValue("lightPosition", QVector3D(entity->transform->position));
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
                    float rScale = light->range;
                    scaleMatrix.scale(rScale,rScale,rScale);
                    QMatrix4x4 worldViewMatrix = camera->viewMatrix * worldMatrix * scaleMatrix;

                    program.setUniformValue("worldMatrix", worldMatrix);
                    program.setUniformValue("viewMatrix", camera->viewMatrix);
                    program.setUniformValue("worldViewMatrix", worldViewMatrix);
                    program.setUniformValue("projectionMatrix", camera->projectionMatrix);

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

    fbo->release();
}

void DeferredRenderer::passDebugLights()
{
    /*
    lbo->bind();
    gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gl->glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    lbo->release();
    */
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
        else if(shownTexture() == "Ambient"){
            gl->glBindTexture(GL_TEXTURE_2D, fboAmbient);
        }
        else if(shownTexture() == "LightSpheres"){
            gl->glBindTexture(GL_TEXTURE_2D, lightSpheres);
        }
        else if(shownTexture() == "MaskSelected")
        {
            gl->glBindTexture(GL_TEXTURE_2D, fboMask);
        }
        else if(shownTexture() == "Mouse Picking")
        {
            gl->glBindTexture(GL_TEXTURE_2D, interaction->renderTexture);
        }
        else if(shownTexture() == "blurDebug")
        {
           gl->glBindTexture(GL_TEXTURE_2D, blurDebug);
        }
        else if(shownTexture() == "DOFMask")
        {
            gl->glBindTexture(GL_TEXTURE_2D, dofMask);
        }
        else {
            gl->glBindTexture(GL_TEXTURE_2D, resourceManager->texWhite->textureId());
        }

        resourceManager->quad->submeshes[0]->draw();
    }

    gl->glEnable(GL_DEPTH_TEST);
}
