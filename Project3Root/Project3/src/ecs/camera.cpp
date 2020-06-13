#include "camera.h"
#include "globals.h"
#include <QtMath>
#include <QMatrix3x3>


Camera::Camera()
{
    // Camera position
    position = QVector3D(0.0, 2.0, 6.0);
    isOrbital = false;

}

QVector4D Camera::getLeftRightBottomTop()
{
    const float aspectRatio = float(viewportWidth) / viewportHeight;
    const float alpha = qDegreesToRadians(fovy * 0.5);
    const float top = znear * qTan(alpha);
    const float bottom = -top;
    const float right = top * aspectRatio;
    const float left = -right;
    QVector4D params(left, right, bottom, top);
    return params;
}

QVector3D Camera::screenPointToWorldRay(int x, int y)
{
    QVector4D lrbt = getLeftRightBottomTop();
    const float l = lrbt.x();
    const float r = lrbt.y();
    const float b = lrbt.z();
    const float t = lrbt.w();
    const float rayX = l + (r - l) * x / viewportWidth;
    const float rayY = b + (t - b) * (viewportHeight - y - 1) / viewportHeight;
    const float rayZ = -znear;
    QVector3D rayViewspace = QVector3D(rayX, rayY, rayZ);

    prepareMatrices();
    QVector3D rayWorldspace = QVector3D(worldMatrix * QVector4D(rayViewspace, 0.0));

    return rayWorldspace;
}

QVector3D Camera::screenDisplacementToWorldVector(int x0, int y0, int x1, int y1, const QVector3D &worldPoint)
{
    const QVector3D worldVectorZNear = screenPointToWorldRay(x1, y1) - screenPointToWorldRay(x0, y0);
    const QVector3D eyeWorldspace = QVector3D(worldMatrix * QVector4D(0.0, 0.0, 0.0, 1.0));
    const float distance = (worldPoint - eyeWorldspace).length();
    const QVector3D worldVector = distance * worldVectorZNear / znear;
    return worldVector;
}

QVector2D Camera::worldToScreenPoint(const QVector3D &pointWorld)
{
    QVector4D lrbt = getLeftRightBottomTop();
    const float l = lrbt.x();
    //const float r = lrbt.y();
    const float b = lrbt.z();
    const float t = lrbt.w();
    const QVector3D pointEye = viewMatrix * pointWorld;
    const QVector2D pointEyeZnear = znear * pointEye.toVector2D() / pointEye.z();
    const QVector2D pointViewport = (pointEyeZnear - QVector2D(l, b)) / QVector2D(t - l, t - b);
    const QVector2D pointScreen = pointViewport * QVector2D(viewportWidth, viewportHeight);
    return pointScreen;
}

void Camera::lookAt(const QVector3D v)
{

    QVector3D forward = position - v;
    forward.normalize();

    QVector3D tmp = QVector3D(0,1,0);
    QVector3D right = QVector3D::crossProduct(tmp,forward);

}

void Camera::updateOrbitalCam()
{
    if (isOrbital)
    {
        //Setting up orbital camera
        static float timer;
        static const float dt = 1.0/60.0f; // Delta time
        float rotSpeed = 0.5f;
        float xSpread = 10.0f;
        float zSpread = 10.0f;
        float yOffset = 3.5f;
        QVector3D centerPoint;

        timer += dt * rotSpeed;
        float x = -cos(timer)* xSpread;
        float z = sin(timer) * zSpread;
        position = QVector3D(x,yOffset,z) + centerPoint;
        lookAt(centerPoint);

        qDebug("%f_%f_%f", position.x(),position.y(),position.z());
    }


}


void Camera::prepareMatrices()
{
    worldMatrix.setToIdentity();
    worldMatrix.translate(position);
    worldMatrix.rotate(yaw, QVector3D(0.0, 1.0, 0.0));
    worldMatrix.rotate(pitch, QVector3D(1.0, 0.0, 0.0));

    viewMatrix = worldMatrix.inverted();

    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(fovy, float(viewportWidth) / viewportHeight, znear, zfar);
}
