#ifndef SELECTION_H
#define SELECTION_H

#include <QObject>

class Entity;

#define MAX_SELECTED_ENTITIES 255

class Selection : public QObject
{
    Q_OBJECT

public:
    Selection();

    void clear();
    void select(Entity *);

    int count = 0;
    Entity *entities[MAX_SELECTED_ENTITIES] = {};

signals:

    void entitySelected(Entity *);
    void onClick();

public slots:

    void onEntitySelectedFromEditor(Entity *);
    void onEntityRemovedFromEditor(Entity *);
    void onEntitySelectedFromSceneView(Entity *);
};

#endif // SELECTION_H
