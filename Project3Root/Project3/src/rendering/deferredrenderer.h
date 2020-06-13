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
    void outlinePrep(int w, int h);

    void blurDebugPrep(int w, int h);

    void CleanFirstBuffers();

    void passMeshes(Camera *camera);
    void passLighting(Camera* camera);
    void passBlit();

    void PassGridBackground(Camera* camera);
    void PassOutline(Camera* camera);

    void PassBlur(FramebufferObject* fb, GLuint Read, GLuint Mask = UINT_MAX);

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    ShaderProgram *gridbgProgram = nullptr;
    ShaderProgram *outlineProgram = nullptr;
    ShaderProgram *maskProgram = nullptr;

    ShaderProgram *blurProgram = nullptr;

    // MAIN Textures Needed

    GLuint fboFinal = 0;
    GLuint fboEditor = 0;
    GLuint fboDepth = 0;

    // Geometry Pass
    GLuint gboPosition = 0;
    GLuint gboAlbedoSpec = 0;
    GLuint gboNormal = 0;

    // Editor Passes Textures
    GLuint fboEditorDepth = 0;
    GLuint fboOutlineMask = 0;

    // Blur Based Passes
    GLuint stepBlur = 0;

    // Framebuffers, defined for use

    OpenGLState mainState;
    FramebufferObject *fbo = nullptr; // Main FBO

    OpenGLState gpassState;
    FramebufferObject *gbo = nullptr; // Geometry Pass

    OpenGLState lightingState;

    FramebufferObject *dofBlurBO = nullptr;
    FramebufferObject *dofMaskBO = nullptr;
    OpenGLState dofMaskState;

    // Editor Framebuffers
    FramebufferObject *gridbgBO = nullptr;
    OpenGLState gridbgState;

    FramebufferObject *outlineBO = nullptr;
    FramebufferObject *maskBO = nullptr;
    OpenGLState outlineState;
    OpenGLState maskState;

    FramebufferObject *blurDebugBO = nullptr;
    OpenGLState blurState;




};


#endif // DEFERREDRENDERER_H
