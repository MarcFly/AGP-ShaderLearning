#ifndef ENTITY_H
#define ENTITY_H

#include "components.h"

#define MAX_COMPONENTS 8

class Entity
{
public:

    Entity();
    Entity(Transform trans);
    ~Entity();

    Component *addComponent(ComponentType ctype);
    Component *findComponent(ComponentType ctype);
    void removeComponent(Component *component);
    float generateRandomNumber(int low, int high);

    Entity *clone() const;

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

    QString name;
    QVector3D color;
    int position = -1;

    union
    {
        struct
        {
            Transform *transform;
            MeshRenderer *meshRenderer;
            LightSource *lightSource;
        };
        Component *components[MAX_COMPONENTS];
    };

    bool active = true;
};

#endif // ENTITY_H
