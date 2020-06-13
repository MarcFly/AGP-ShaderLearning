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
    void gridbgPrep(int w, int h);

    void CleanFirstBuffers();

    void passMeshes(Camera *camera);
    void passLighting(Camera* camera);
    void passBlit();

    void PassGridBackground(Camera* camera);

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    ShaderProgram *gridbgProgram = nullptr;

    // MAIN Textures Needed

    GLuint fboFinal = 0;
    GLuint fboEditor = 0;
    GLuint fboDepth = 0;

    // Geometry Pass
    GLuint gboPosition = 0;
    GLuint gboAlbedoSpec = 0;
    GLuint gboNormal = 0;

    // Editor Grid and Background Pass
    GLuint fboGridBgDepth = 0;




    // Framebuffers, defined for use

    OpenGLState mainState;
    FramebufferObject *fbo = nullptr; // Main FBO

    OpenGLState gpassState;
    FramebufferObject *gbo = nullptr; // Geometry Pass

    OpenGLState lightingState;

    FramebufferObject *gridbgBO = nullptr;
    OpenGLState gridbgState;

};


#endif // DEFERREDRENDERER_H
