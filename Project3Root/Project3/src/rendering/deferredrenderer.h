#ifndef DEFERREDRENDERER_H
#define DEFERREDRENDERER_H


#include "renderer.h"
#include "gl.h"
#include <QOpenGLShaderProgram>

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

    void fboPrep(int w, int h); // Prepare main buffers (depth and final color)
    void gboPrep(int w, int h); // Prepare geometry buffers
    void lboPrep(int w, int h); // Prepare buffers for debug of lights
    void sboPrep(int w, int h); // Prepare buffers for sketch

    void passMeshes(Camera *camera);
    void passLighting();

    void passMeshesForLayers(Camera * camera);

    void PassNormalDepth(Camera* camera, QOpenGLShaderProgram* program);
    void passNextLayer(Camera* camera);
    void passPrevLayer(Camera* camera);
    void passSketch();

    void passBlit();

    void passDebugLights();

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    ShaderProgram *GenLayers = nullptr;
    ShaderProgram *sketchProgram = nullptr;

    // MAIN Textures Needed

    GLuint fboColor = 0;
    GLuint fboDepth = 0;

    // GBuffer Textures
    GLuint gboPosition = 0;
    GLuint gboAlbedoSpec = 0;
    GLuint gboNormal = 0;

    GLuint fboAmbient = 0;

    // Depth Peeling
    std::vector<GLuint> normalLayers;
    std::vector<GLuint> depthLayers;
    std::vector<GLuint> edgeMasks;

    // ONLY FOR DEBUG PURPOSES
    // ONLY WRITE TO WHEN THE TEXTURE IS SELECTED
    ShaderProgram *debugSpheres = nullptr;
    GLuint lightSpheres = 0;

    GLuint sketchOnly = 0;
    uint sketchLayer = 0;

    // Framebuffers, defined for use
    FramebufferObject *fbo = nullptr; // Main FBO
    FramebufferObject *gbo = nullptr; // Geometry Pass

    FramebufferObject *lbo = nullptr; // Debug for Light Spheres

    FramebufferObject* peelBO = nullptr;
    FramebufferObject *prevLayerbo = nullptr;
    FramebufferObject *nextLayerbo = nullptr;

};


#endif // DEFERREDRENDERER_H
