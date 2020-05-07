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

    void passMeshes(Camera *camera);
    void passLighting();
    void passBlit();

    // Shaders
    ShaderProgram *gpassProgram = nullptr;
    ShaderProgram *deferredProgram = nullptr;
    ShaderProgram *blitProgram = nullptr;

    GLuint fboColor = 0;
    GLuint fboDepth = 0;
    GLuint gboPosition = 0;
    GLuint gboAlbedoSpec = 0;
    GLuint gboNormal = 0;


    FramebufferObject *fbo = nullptr;
    FramebufferObject *gbo = nullptr;

};


#endif // DEFERREDRENDERER_H
