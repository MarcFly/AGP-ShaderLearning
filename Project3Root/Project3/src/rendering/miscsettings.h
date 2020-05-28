#ifndef MISCSETTINGS_H
#define MISCSETTINGS_H

#include <QColor>

class MiscSettings
{
public:
    MiscSettings();

    // TODO: Maybe not the best place for this stuff...
    QColor backgroundColor;
    float AMBIENT = 0.5f;
    bool renderLightSources = true;
    int depthLayers = 1;
    int debugLayer = 0;
};

#endif // MISCSETTINGS_H
