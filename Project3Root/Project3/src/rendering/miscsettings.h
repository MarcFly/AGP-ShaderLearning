#ifndef MISCSETTINGS_H
#define MISCSETTINGS_H

#include <QColor>
#include <QJsonObject>
#include <QObject>

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
    bool checkBlur = false;
    bool maskBlur = false;

    // DOF Controls
    float dofFocusDistance = 5.;
    float dofFocusDepth = 2.;
    float dofFalloff = 1.;
    bool checkDOF = true;
    bool checkBleed = false;

    // SSAO Controls
    bool checkSSAO = true;
    bool blurSSAO = true;
    float aoRadius = .5;


    void read(QJsonObject &json);
    void write(QJsonObject &json);

};

#endif // MISCSETTINGS_H
