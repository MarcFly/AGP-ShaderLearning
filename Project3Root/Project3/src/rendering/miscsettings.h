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
    float dofFocusDistance = 5.;
    float dofFocusDepth = 2.;
    float dofFalloff = .5;
    bool checkDOF = true;

    void read(const QJsonObject &json);
    void write(QJsonObject &json);

};

#endif // MISCSETTINGS_H
