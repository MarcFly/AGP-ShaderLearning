#include "lightsourcewidget.h"
#include "ecs/scene.h"
#include <QLabel>
#include <QSlider>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QColorDialog>
#include <QSignalBlocker>


LightSourceWidget::LightSourceWidget(QWidget *parent) : QWidget(parent)
{
    setWindowTitle(QString::fromLatin1("Light Source"));

    auto vlayout = new QFormLayout;
    vlayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);

    auto labelType = new QLabel("Type");
    labelType->setMinimumSize(QSize(70, 10));
    comboType = new QComboBox;
    comboType->addItem("Point", QVariant::fromValue<int>((int)LightSource::Type::Point));
    comboType->addItem("Directional", QVariant::fromValue<int>((int)LightSource::Type::Directional));
    vlayout->addRow(labelType, comboType);

    auto labelColor = new QLabel("Color");
    labelColor->setMinimumSize(QSize(70, 10));
    buttonColor = new QPushButton("");
    vlayout->addRow(labelColor, buttonColor);

    auto labelIntensity = new QLabel("Intensity");
    labelIntensity->setMinimumSize(QSize(70, 10));
    spinIntensity = new QDoubleSpinBox();
    spinIntensity->setMinimum(0.0);
    spinIntensity->setMaximum(10000.0);
    spinIntensity->setSingleStep(.2);
    spinIntensity->setValue(1.0);
    vlayout->addRow(labelIntensity, spinIntensity);

    auto labelRange = new QLabel("Range");
    labelRange->setMinimumSize(QSize(70, 10));
    spinRange = new QDoubleSpinBox();
    spinRange->setMinimum(0.0);
    spinRange->setMaximum(10000.0);
    spinRange->setSingleStep(.1);
    spinRange->setValue(1.0);
    vlayout->addRow(labelRange, spinRange);

    auto labelAttenuation = new QLabel("ATTENUATION CONSTANTS");
    labelRange->setMinimumSize(QSize(70, 10));
    vlayout->addRow(labelAttenuation);

    auto labelFixedK = new QLabel("Fixed Falloff");
    labelFixedK->setMinimumSize(QSize(70, 10));
    spinFixedK = new QDoubleSpinBox();
    spinFixedK->setMinimum(0.0);
    spinFixedK->setMaximum(10000.0);
    spinFixedK->setDecimals(5);
    spinFixedK->setSingleStep(.2);
    spinFixedK->setValue(1.0);
    spinFixedK->setSingleStep(.05);
    vlayout->addRow(labelFixedK, spinFixedK);

    auto labelLinearK = new QLabel("Linear Falloff");
    labelLinearK->setMinimumSize(QSize(70, 10));
    spinLinearK = new QDoubleSpinBox();
    spinLinearK->setMinimum(0.0);
    spinLinearK->setMaximum(10.0);
    spinLinearK->setDecimals(5);
    spinLinearK->setSingleStep(.01);
    spinLinearK->setValue(1.);
    vlayout->addRow(labelLinearK, spinLinearK);

    auto labelQuadraticK = new QLabel("Quadratic Falloff");
    labelQuadraticK->setMinimumSize(QSize(70, 10));
    spinQuadraticK = new QDoubleSpinBox();
    spinQuadraticK->setMinimum(0.0);
    spinQuadraticK->setMaximum(100.0);
    spinQuadraticK->setDecimals(5);
    spinQuadraticK->setSingleStep(.01);
    spinQuadraticK->setValue(1.);
    vlayout->addRow(labelQuadraticK, spinQuadraticK);

    setLayout(vlayout);

    connect(comboType, SIGNAL(currentIndexChanged(int)), this, SLOT(onTypeChanged(int)));
    connect(spinIntensity, SIGNAL(valueChanged(double)), this, SLOT(onIntensityChanged(double)));
    connect(spinRange, SIGNAL(valueChanged(double)), this, SLOT(onRangeChanged(double)));
    connect(spinFixedK, SIGNAL(valueChanged(double)), this, SLOT(onFixedKChanged(double)));
    connect(spinLinearK, SIGNAL(valueChanged(double)), this, SLOT(onLinearKChanged(double)));
    connect(spinQuadraticK, SIGNAL(valueChanged(double)), this, SLOT(onQuadraticKChanged(double)));

    connect(buttonColor, SIGNAL(clicked()), this, SLOT(onColorButtonClicked()));
}

void LightSourceWidget::setLightSource(LightSource *light)
{
    lightSource = light;
    if (lightSource == nullptr) return;

    QSignalBlocker b1(comboType);
    QSignalBlocker b2(spinIntensity);
    QSignalBlocker b3(spinRange);
    QSignalBlocker b4(buttonColor);

    comboType->setCurrentIndex((int)lightSource->type);

    spinIntensity->setValue(lightSource->intensity);
    spinRange->setValue(lightSource->range);
    spinFixedK->setValue(lightSource->kc);
    spinLinearK->setValue(lightSource->kl);
    spinQuadraticK->setValue(lightSource->kq);

    QString colorName = lightSource->color.name();
    buttonColor->setStyleSheet(QString::fromLatin1("background-color: %0").arg(colorName));
}

void LightSourceWidget::onTypeChanged(int index)
{
    lightSource->type = (LightSource::Type)comboType->itemData(index).value<int>();
    emit componentChanged(lightSource);
}

void LightSourceWidget::onIntensityChanged(double val)
{
    lightSource->intensity = val;
    emit componentChanged(lightSource);
}

float AsymSigmoid (float x, float a, float b, float c, float d, float m)
{
    return (d + (a - d) / std::pow(1 + std::pow(x/c,b), m));
}

void LightSourceWidget::onRangeChanged(double val)
{
    float test_int_change = val / lightSource->range;
    lightSource->range = val;

    spinLinearK->blockSignals(true);
    spinQuadraticK->blockSignals(true);

    lightSource->kl = AsymSigmoid(val, 4.268f, 10.870f, 1.451f, 0.021f, 0.107f);
    lightSource->kq = AsymSigmoid(val, 9.191f, 17.307f, 3.398f, 0.013f, 0.131f);

    spinLinearK->setValue(lightSource->kl);
    spinQuadraticK->setValue(lightSource->kq);

    spinLinearK->blockSignals(false);
    spinQuadraticK->blockSignals(false);

    emit componentChanged(lightSource);
}

void LightSourceWidget::onFixedKChanged(double val)
{
    lightSource->kc = val;

    spinRange->blockSignals(true);

    lightSource->range = (std::sqrtf(std::pow(lightSource->kl,2.) - 4.*lightSource->kq*((-256.f / 5.f)* val) + lightSource->kc)-lightSource->kl) / (2.*lightSource->kq);
    spinRange->setValue(lightSource->range);

    spinRange->blockSignals(false);

    emit componentChanged(lightSource);
}

void LightSourceWidget::onLinearKChanged(double val)
{
    lightSource->kl = val;

    spinRange->blockSignals(true);

    lightSource->range = (std::sqrtf(std::pow(lightSource->kl,2.) - 4.*lightSource->kq*((-256.f / 8.f)* val) + lightSource->kc)-lightSource->kl) / (2.*lightSource->kq);
    spinRange->setValue(lightSource->range);

    spinRange->blockSignals(false);

    emit componentChanged(lightSource);
}
void LightSourceWidget::onQuadraticKChanged(double val)
{
    lightSource->kq = val;

    spinRange->blockSignals(true);

    lightSource->range = (std::sqrtf(std::pow(lightSource->kl,2.) - 4.*lightSource->kq*((-256.f / 8.f)* val) + lightSource->kc)-lightSource->kl) / (2.*lightSource->kq);
    spinRange->setValue(lightSource->range);

    spinRange->blockSignals(false);

    emit componentChanged(lightSource);
}

void LightSourceWidget::onColorButtonClicked()
{
    QColor color = QColorDialog::getColor(lightSource->color, this, "Light color");
    if (color.isValid())
    {
        QString colorName = color.name();
        buttonColor->setStyleSheet(QString::fromLatin1("background-color: %0").arg(colorName));
        lightSource->color = color;
        emit componentChanged(lightSource);
    }
}
