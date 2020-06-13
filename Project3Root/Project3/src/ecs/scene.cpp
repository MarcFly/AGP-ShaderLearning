#include "scene.h"
#include "globals.h"
#include "resources/mesh.h"
#include "resources/texture.h"
#include "resources/material.h"
#include "globals.h"
#include <QJsonArray>


// Scene //////////////////////////////////////////////////////////////////

Scene::Scene()
{
}

Scene::~Scene()
{
    for (auto entity : entities)
    {
        delete entity;
    }
}

int Scene::numEntities() const
{
    return entities.size();
}

Entity *Scene::addEntity()
{
    Entity *entity = new Entity;
    entities.push_back(entity);
    return entity;
}

Entity *Scene::entityAt(int index)
{
    return entities[index];
}

void Scene::removeEntityAt(int index)
{
    delete entities[index];
    entities.removeAt(index);
    //Update entities position
    for (int i = 0; i < entities.length();i++)
    {
        entities[i]->position = i;
    }
}

Component *Scene::findComponent(ComponentType ctype)
{
   for (auto entity : entities) {
       auto comp = entity->findComponent(ctype);
       if (comp != nullptr) return comp;
   }
   return nullptr;
}

void Scene::clear()
{
    for (auto entity : entities)
    {
        delete entity;
    }
    entities.clear();
}

void Scene::handleResourcesAboutToDie()
{
    for (auto entity : entities)
    {
        if (entity->meshRenderer)
        {
            entity->meshRenderer->handleResourcesAboutToDie();
        }
    }
}

void Scene::read(const QJsonObject &json)
{
    //Entity Iteration
    for (QJsonObject::const_iterator i = json.begin(); i != json.end(); i++)
    {
        QString key = i.key();
        if (key != "MiscSettings")
        {
            Entity* ret = addEntity();

            QJsonObject obj = i->toObject();
            ret->read(obj);
        }
        else
            break;
    }
    //Misc Settings Iteration
    for (QJsonObject::const_iterator i = json.begin(); i != json.end(); i++)
    {
        QJsonObject obj = i->toObject();
        miscSettings->read(obj);
    }



}

void Scene::write(QJsonObject &json)
{  
    QJsonObject gos;
    for (int i = 0; i< entities.length();i++)
    {
        QJsonObject go;
        entities[i]->write(go);
        QString goIndex = QString::number(i);
        json.insert(goIndex,go);
    }    

    QJsonObject miscSettingsJsonObject;
    miscSettings->write(miscSettingsJsonObject);
    json.insert("MiscSettings", miscSettingsJsonObject);
}

