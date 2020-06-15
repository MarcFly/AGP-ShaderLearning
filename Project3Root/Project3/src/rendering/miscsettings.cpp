#include "miscsettings.h"
#include "globals.h"

MiscSettings::MiscSettings() :
    backgroundColor(40, 40, 40, 255)
{

}

void MiscSettings::read(QJsonObject &json)
{    
    camera->read(json);
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
        else if (key == "SSAO")
        {
            this->checkSSAO = j.value().toBool();
        }
        else if(key == "ssaoRad")
        {
            this->aoRadius = j.value().toDouble();
        }
        else if(key == "blurSSAO")
        {
            this->blurSSAO = j.value().toBool();
        }
        else if(key == "bgColR")
        {
            this->backgroundColor.setRedF(j.value().toDouble());
        }
        else if(key == "bgColG")
        {
            this->backgroundColor.setBlueF(j.value().toDouble());
        }
        else if(key == "bgColB")
        {
            this->backgroundColor.setGreenF(j.value().toDouble());
        }
        else if(key == "gColR")
        {
            this->gridColor.setRedF(j.value().toDouble());
        }
        else if(key == "gColG")
        {
            this->gridColor.setGreenF(j.value().toDouble());
        }
        else if(key == "gColB")
        {
            this->gridColor.setBlueF(j.value().toDouble());
        }
        else if(key == "DOFBleed")
        {
            this->checkBleed = j.value().toBool();
        }
        else if(key == "checkBlur")
        {
            this->checkBlur = j.value().toBool();
        }
        else if(key == "maskBlur")
        {
            this->maskBlur = j.value().toBool();
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
    json["checkBlur"] = this->checkBlur;
    json["maskBlur"] = this->maskBlur;

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
    json["DOFBleed"] = this->checkBleed;


    //Editor visual hints
    json["renderGrid"] = this->renderGrid;
    json["lightSources"] = this->renderLightSources;
    json["bgColR"] = this->backgroundColor.redF();
    json["bgColG"] = this->backgroundColor.blueF();
    json["bgColB"] = this->backgroundColor.greenF();
    json["gColR"] = this->gridColor.redF();
    json["gColG"] = this->gridColor.blueF();
    json["gColB"] = this->gridColor.greenF();


    //SSAO
    json["SSAO"] = this->checkSSAO;
    json["ssaoRad"] = this->aoRadius;
    json["blurSSAO"] = this->blurSSAO;

}
