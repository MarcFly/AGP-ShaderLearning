#include "components.h"
#include "resources/mesh.h"
#include "resources/material.h"




Transform::Transform() :
    position(0.0f, 0.0f, 0.0f),
    rotation(),
    scale(1.0f, 1.0f, 1.0f)
{

}

QMatrix4x4 Transform::matrix() const
{
    QMatrix4x4 mat;
    mat.translate(position);
    mat.rotate(rotation);
    mat.scale(scale);    
    return mat;
}

void Transform::read(const QJsonObject &json)
{
    QString TransformData = json["PosRotScale"].toString();
    QStringList strList = TransformData.split(",");
    position.setX(strList[0].toFloat());
    position.setY(strList[1].toFloat());
    position.setZ(strList[2].toFloat());

    rotation.setX(strList[3].toFloat());
    rotation.setY(strList[4].toFloat());
    rotation.setZ(strList[5].toFloat());

    scale.setX(strList[6].toFloat());
    scale.setY(strList[7].toFloat());
    scale.setZ(strList[8].toFloat());
}

void Transform::write(QJsonObject &json)
{
    QJsonObject transformComponent;
    QString pos;
    pos.append(QString::number(this->position.x()) + ",");
    pos.append(QString::number(this->position.y()) + ",");
    pos.append(QString::number(this->position.z()) + ",");
    pos.append(QString::number(this->rotation.x()) + ",");
    pos.append(QString::number(this->rotation.y()) + ",");
    pos.append(QString::number(this->rotation.z()) + ",");
    pos.append(QString::number(this->scale.x()) + ",");
    pos.append(QString::number(this->scale.y()) + ",");
    pos.append(QString::number(this->scale.z()) + ",");
    transformComponent["PosRotScale"] = pos;
    json.insert("TransformComponent", transformComponent);
}




MeshRenderer::MeshRenderer()
{
}

void MeshRenderer::handleResourcesAboutToDie()
{
    if (mesh->needsRemove)
    {
        mesh = nullptr;
    }

    for (int i = 0; i < materials.size(); ++i)
    {
        if (materials[i] && materials[i]->needsRemove)
        {
            materials[i] = nullptr;
        }
    }
}

void MeshRenderer::read(const QJsonObject &json)
{


}

void MeshRenderer::write(QJsonObject &json)
{
    this->mesh->write(json);
}




LightSource::LightSource() :
    color(255, 255, 255, 255)
{

}

void LightSource::read(const QJsonObject &json)
{    
    this->color.setRed(json["r"].toInt());
    this->color.setGreen(json["g"].toInt());
    this->color.setBlue(json["b"].toInt());
    this->color.setAlpha(json["a"].toInt());
    //this->kc = json["kc"].toInt();
    //this->kl = json["kl"].toInt();
    //this->kq = json["kq"].toInt();
    Type type = (Type)json["type"].toInt();
    this->type = type;
    this->range = json["range"].toDouble();
    this->intensity = json["intensity"].toDouble();
}

void LightSource::write(QJsonObject &json)
{    
    QJsonObject lightComponent;
    int ret[] = {this->color.red(),this->color.green(),this->color.blue(),this->color.alpha()};
    lightComponent["r"] = ret[0];
    lightComponent["g"] = ret[1];
    lightComponent["b"] = ret[2];
    lightComponent["a"] = ret[3];
    lightComponent["kc"] = this->kc;
    lightComponent["kl"] = this->kl;
    lightComponent["kq"] = this->kq;
    lightComponent["type"] = (int)this->type;
    lightComponent["range"] = this->range;
    lightComponent["intensity"] = this->intensity;

    json.insert("LightComponent", lightComponent);
}

