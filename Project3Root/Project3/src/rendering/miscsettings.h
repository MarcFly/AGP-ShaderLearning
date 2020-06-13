#ifndef MISCSETTINGS_H
#define MISCSETTINGS_H

#include <QColor>

class MiscSettings
{
public:
    MiscSettings();

    // TODO: Maybe not the best place for this stuff...
    QColor backgroundColor;
    QColor gridColor;
    float AMBIENT = 0.5f;
    bool renderLightSources = true;

    // Editor Hints Controls
    bool renderOutline = true;
    bool renderGrid = true;
    float OutlineWidth = 1.;
    float OutlineAlpha = 1.;

    // Blur Controls
    float blurVal = 1.;
    int blurType = 0; // 0 = Gaussian

    // DOF Controls
    float dofFocus = 10.;
    float dofDepth = 20.;
    float dofFalloff = 4.;
    bool checkDOF = true;


};

#endif // MISCSETTINGS_H
