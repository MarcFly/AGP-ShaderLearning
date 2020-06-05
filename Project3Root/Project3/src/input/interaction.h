#ifndef INTERACTION_H
#define INTERACTION_H

#include "rendering/gl.h"
class FramebufferObject;
class ShaderProgram;

class Interaction
{
public:

    bool update();

    void init();

    void postUpdate();

    void generateBuffers(int width, int height);

    bool mousePicking();

    GLuint renderTexture;
    bool render = false;

private:

    bool idle();
    bool navigate();
    bool focus();

    void passMeshes();

    ShaderProgram* mpProgram = nullptr;
    FramebufferObject* frameBuffer = nullptr;
    GLuint depthTexture;



    enum State { Idle, Navigating, Focusing, MousePicking };
    State state = State::Idle;
    State nextState = State::Idle;
};

#endif // INTERACTION
