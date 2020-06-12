#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H


#include "renderer.h"
#include "gl.h"

class ShaderProgram;
class FramebufferObject;

class DeferredRenderer : public Renderer
{
public:

    DeferredRenderer();
    ~DeferredRenderer() override;

    void initialize() override;
    void finalize() override;

    void resize(int width, int height) override;
    void render(Camera *camera) override;

private:

    void fboPrep(int w, int h);
    void gboPrep(int w, int h);
    void cleanBuffers();

    void passMeshes(Camera *camera);
    void passLighting(Camera* camera);
    void passBlit();

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    // MAIN Textures Needed

    GLuint fboFinal = 0;
    GLuint fboEditor = 0;
    GLuint fboDepth = 0;

    // Geometry Pass
    GLuint gboPosition = 0;
    GLuint gboAlbedoSpec = 0;
    GLuint gboNormal = 0;



    // Framebuffers, defined for use

    OpenGLState mainState;
    FramebufferObject *fbo = nullptr; // Main FBO
    // Will use fbo for grid and background also
    OpenGLState gpassState;
    FramebufferObject *gbo = nullptr; // Geometry Pass

    OpenGLState lightingState;

};


#endif // DEFERREDRENDERER_H
