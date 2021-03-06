#ifndef INTERACTION_H
#define INTERACTION_H

#include "rendering/gl.h"
#include "src/ecs/entity.h"

class FramebufferObject;
class ShaderProgram;
class Camera;

class Interaction
{
public:

    bool update();

    void init();

    void postUpdate();

    void generateBuffers(int width, int height);

    bool mousePicking(Camera* camera);

    GLuint renderTexture;
    bool render = false;

private:

    bool idle();
    bool navigate();
    bool focus();

    void passMeshes(Camera* camera);

    ShaderProgram* mpProgram = nullptr;
    FramebufferObject* frameBuffer = nullptr;
    GLuint depthTexture;

    Entity* selectedEntity = nullptr;


    OpenGLState mpState;

    enum State { Idle, Navigating, Focusing, MousePicking };
    State state = State::Idle;
    State nextState = State::Idle;
};

#endif // INTERACTION
