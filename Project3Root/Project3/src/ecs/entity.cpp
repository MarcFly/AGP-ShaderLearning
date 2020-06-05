#include "entity.h"
#include "globals.h"
#include "util/modelimporter.h"

Entity::Entity() :
    name("Entity")
{
    for (int i = 0; i < MAX_COMPONENTS; ++i)
        components[i] = nullptr;
    transform = new Transform;
    //TODO RANDOM COLOR
    color = QVector3D(1.0,0.0,0.0);
}
Entity::Entity(Transform trans) : name("Entity")
{
    for (int i = 0; i < MAX_COMPONENTS; ++i)
        components[i] = nullptr;
    transform = &trans;
}

Entity::~Entity()
{
    delete transform;
    delete meshRenderer;
    delete lightSource;
}

Component *Entity::addComponent(ComponentType componentType)
{
    Component *component = nullptr;

    switch (componentType)
    {
    case ComponentType::Transform:
        Q_ASSERT(transform == nullptr);
        component = transform = new Transform;
        break;
    case ComponentType::LightSource:
        Q_ASSERT(lightSource == nullptr);
        component = lightSource = new LightSource;
        break;
    case ComponentType::MeshRenderer:
        Q_ASSERT(meshRenderer == nullptr);
        component = meshRenderer = new MeshRenderer;
        break;
    default:
        Q_ASSERT(false && "Invalid code path");
    }

    component->entity = this;
    return component;
}

void Entity::removeComponent(Component *component)
{
    if (transform == component)
    {
        delete transform;
        transform = nullptr;
    }
    else if (component == meshRenderer)
    {
        delete meshRenderer;
        meshRenderer = nullptr;
    }
    else if (component == lightSource)
    {
        delete lightSource;
        lightSource = nullptr;
    }
}

Component *Entity::findComponent(ComponentType ctype)
{
    for (int i = 0; i < MAX_COMPONENTS; ++i)
    {
        if (components[i] != nullptr && components[i]->componentType() == ctype)
        {
            return components[i];
        }
    }
    return nullptr;
}

Entity *Entity::clone() const
{
    Entity *entity = scene->addEntity(); // Global scene
    entity->name = name;
    entity->active = active;
    if (transform != nullptr) {
        //entity->addComponent(ComponentType::Transform); // transforms are created by default
        *entity->transform = *transform;
        entity->transform->entity = entity;
    }
    if (meshRenderer != nullptr) {
        entity->addComponent(ComponentType::MeshRenderer);
        *entity->meshRenderer = *meshRenderer;
        entity->meshRenderer->entity = entity;
    }
    if (lightSource != nullptr) {
        entity->addComponent(ComponentType::LightSource);
        *entity->lightSource = *lightSource;
        entity->lightSource->entity = entity;
    }
    return entity;
}

void Entity::read(const QJsonObject &json)
{
    for (QJsonObject::const_iterator j = json.begin(); j != json.end(); j++)
    {

        QString key = j.key();
        if (key == "TransformComponent")
        {
            transform = new Transform();
            QJsonObject TransformObject = j->toObject();
            transform->read(TransformObject);
        }
        else if (key == "LightComponent")
        {
            //Create new LightSource in the entity
            lightSource = new LightSource();
            lightSource->entity = this;

            //Update lightsource values
            QJsonObject lightObject = j->toObject();
            lightSource->read(lightObject);            
        }
        else if (key == "MeshRenderer")
        {
            QJsonObject meshObject = j->toObject();            

            QString path = meshObject["MeshFilePath"].toString();
            if (path.isEmpty()) return;

            ModelImporter importer;
            Entity* ret = importer.import(path);
            this->meshRenderer = ret->meshRenderer;
        }
        else if (key == "name")
        {
            name = j.value().toString();
        }
        else if (key == "isActive")
        {
            this->active = j.value().toBool();
        }
    }

}

void Entity::write(QJsonObject &json)
{
    json["name"] = this->name;
    json["isActive"] = this->active;

    if (transform != nullptr)
    {        
        transform->write(json);
    }
    if (this->lightSource != nullptr)
    {
        lightSource->write(json);
    }
    if (this->meshRenderer != nullptr)
    {
        meshRenderer->write(json);
    }

}
