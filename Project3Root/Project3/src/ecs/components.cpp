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
    rotation.setX(strList[4].toFloat());
    rotation.setX(strList[5].toFloat());

    scale.setX(strList[6].toFloat());
    scale.setX(strList[7].toFloat());
    scale.setX(strList[8].toFloat());
}

void Transform::write(QJsonObject &json)
{
    QJsonObject transfromComponent;

    QString pos;
    pos.append(QString::number(this->position.x()));
    pos.append(",");
    pos.append(QString::number(this->position.y()));
    pos.append(",");
    pos.append(QString::number(this->position.z()));
    pos.append(",");
    pos.append(QString::number(this->rotation.x()));
    pos.append(",");
    pos.append(QString::number(this->rotation.y()));
    pos.append(",");
    pos.append(QString::number(this->rotation.z()));
    pos.append(",");
    pos.append(QString::number(this->scale.x()));
    pos.append(",");
    pos.append(QString::number(this->scale.y()));
    pos.append(",");
    pos.append(QString::number(this->scale.z()));
    transfromComponent["PosRotScale"] = pos;
    json.insert("TransfromComponent", transfromComponent);
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
}




LightSource::LightSource() :
    color(255, 255, 255, 255)
{

}

void LightSource::read(const QJsonObject &json)
{
}

void LightSource::write(QJsonObject &json)
{
}

