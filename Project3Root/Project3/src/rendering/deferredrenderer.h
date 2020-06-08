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
    void lboPrep(int w, int h);
    void mboPrep(int w, int h);
    void gbboPrep(int w, int h);
    void gblurboPrep(int w, int h);
    void dofPrep(int w, int h);
    void cleanBuffers();

    void passMeshes(Camera *camera);
    void passLighting();
    void passBlit();

    void passDebugLights();

    void passBlur(GLuint ReadCol, GLuint Mask = UINT_MAX);
    void passBlurDebug();

    void passDepthOfField();

    void passOutline(Camera *camera);

    void passGridBackground(Camera* camera);

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    ShaderProgram *maskProgram = nullptr;
    ShaderProgram *outlineProgram = nullptr;

    ShaderProgram* bggridProgram = nullptr;

    ShaderProgram* gaussianblurProgram = nullptr;

    ShaderProgram* depthOfFieldProgram = nullptr;

    // MAIN Textures Needed

    GLuint fboColor = 0;
    GLuint fboDepth = 0;

    // Geometry Pass
    GLuint gboPosition = 0;
    GLuint gboAlbedoSpec = 0;
    GLuint gboNormal = 0;
    GLuint fboAmbient = 0;

    // Outline Pass
    GLuint fboMask = 0;
    GLuint fboDepthMask = 0;

    // Depth of Field Pass
    GLuint stepBlur = 0;
    GLuint dofMask = 0;

    // ONLY FOR DEBUG PURPOSES
    // ONLY WRITE TO WHEN THE TEXTURE IS SELECTED
    ShaderProgram *debugSpheres = nullptr;
    GLuint lightSpheres = 0;

    GLuint blurDebug = 0;


    // Framebuffers, defined for use
    FramebufferObject *fbo = nullptr; // Main FBO
    // Will use fbo for grid and background also
    FramebufferObject *gbo = nullptr; // Geometry Pass
    FramebufferObject *lbo = nullptr; // Debug for Light Spheres

    FramebufferObject *mbo = nullptr; // Mask selected Object
    FramebufferObject *obo = nullptr; // Outline buffer object

    FramebufferObject *gbbo = nullptr; // Gird & Background Buffer Object

    FramebufferObject* gblurbo = nullptr;

    FramebufferObject* dofbo = nullptr;

};


#endif // DEFERREDRENDERER_H
