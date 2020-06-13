#include "miscsettings.h"
#include "globals.h"

MiscSettings::MiscSettings() :
    backgroundColor(40, 40, 40, 255)
{

}

void MiscSettings::read(const QJsonObject &json)
{    
    for (QJsonObject::const_iterator j = json.begin(); j != json.end(); j++)
    {

        QString key = j.key();
        if (key == "renderOutline")
        {
            this->renderOutline = j.value().toBool();
        }
        else if (key == "Outlinewidth")
        {
            this->OutlineWidth = j.value().toDouble();
        }
        else if (key == "Outlinealpha")
        {
            this->OutlineAlpha = j.value().toDouble();
        }
        else if (key == "blurVal")
        {
            this->blurVal = j.value().toDouble();
        }
        else if (key == "Outlinealpha")
        {
            this->OutlineAlpha = j.value().toDouble();
        }
        else if (key == "blurType")
        {
            this->blurType = j.value().toInt();
        }
        else if (key == "backgroundColorR")
        {
            this->backgroundColor.setRed(j.value().toInt());
        }
        else if (key == "backgroundColorG")
        {
            this->backgroundColor.setGreen(j.value().toInt());
        }
        else if (key == "backgroundColorB")
        {
            this->backgroundColor.setBlue(j.value().toInt());
        }
        else if (key == "ambient")
        {
            this->AMBIENT = j.value().toDouble();
        }
        else if (key == "DOF")
        {
            this->checkDOF = j.value().toBool();
        }
        else if (key == "DOFFocusDepth")
        {
            this->dofFocusDepth = j.value().toDouble();
        }
        else if (key == "DOFFall")
        {
            this->dofFalloff = j.value().toDouble();
        }
        else if (key == "DOFFocusDistance")
        {
            this->dofFocusDistance = j.value().toDouble();
        }
        else if (key == "renderGrid")
        {
            this->renderGrid = j.value().toBool();
        }
        else if (key == "lightSources")
        {
            this->renderLightSources = j.value().toBool();
        }
    }

    emit selection->updateMiscSettings();

}

void MiscSettings::write(QJsonObject &json)
{
    //Outline
    json["renderOutline"] = this->renderOutline;
    json["Outlinewidth"] = this->OutlineWidth;
    json["Outlinealpha"] = this->OutlineAlpha;

    //blur
    json["blurVal"] = this->blurVal;
    json["blurType"] = this->blurType;

    //background
    json["backgroundColorR"] = this->backgroundColor.red();
    json["backgroundColorG"] = this->backgroundColor.green();
    json["backgroundColorB"] = this->backgroundColor.blue();
    json["ambient"] = this->AMBIENT;

    //Depth of Field
    json["DOF"] = this->checkDOF;
    json["DOFFocusDepth"] = this->dofFocusDepth;
    json["DOFFall"] = this->dofFalloff;
    json["DOFFocusDistance"] = this->dofFocusDistance;


    //Editor visual hints
    json["renderGrid"] = this->renderGrid;
    json["lightSources"] = this->renderLightSources;

}
