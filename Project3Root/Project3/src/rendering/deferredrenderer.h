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

    void passMeshes(Camera *camera);
    void passLighting();
    void passBlit();

    void passDebugLights();

    void passOutline(Camera *camera);

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    ShaderProgram *maskProgram = nullptr;
    ShaderProgram *outlineProgram = nullptr;

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

    // ONLY FOR DEBUG PURPOSES
    // ONLY WRITE TO WHEN THE TEXTURE IS SELECTED
    ShaderProgram *debugSpheres = nullptr;
    GLuint lightSpheres = 0;

    // Framebuffers, defined for use
    FramebufferObject *fbo = nullptr; // Main FBO
    FramebufferObject *gbo = nullptr; // Geometry Pass
    FramebufferObject *lbo = nullptr; // Debug for Light Spheres

    FramebufferObject *mbo = nullptr; // Mask selected Object
    FramebufferObject *obo = nullptr; // Outline buffer object



};


#endif // DEFERREDRENDERER_H
