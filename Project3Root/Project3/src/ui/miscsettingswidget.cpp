#include "miscsettingswidget.h"
#include "ui_miscsettingswidget.h"
#include "globals.h"
#include <QColorDialog>


MiscSettingsWidget::MiscSettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MiscSettingsWidget)
{
    ui->setupUi(this);

    ui->spinCameraSpeed->setValue(DEFAULT_CAMERA_SPEED);
    ui->spinFovY->setValue(DEFAULT_CAMERA_FOVY);
    ui->spinAmbient->setSingleStep(.05);
    ui->spinAmbient->setValue(0.5);

    connect(ui->spinCameraSpeed, SIGNAL(valueChanged(double)), this, SLOT(onCameraSpeedChanged(double)));
    connect(ui->spinFovY, SIGNAL(valueChanged(double)), this, SLOT(onCameraFovYChanged(double)));
    connect(ui->buttonBackgroundColor, SIGNAL(clicked()), this, SLOT(onBackgroundColorClicked()));
    connect(ui->checkBoxGrid, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->checkBoxLightSources, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->checkBoxSelectionOutline, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->spinAmbient, SIGNAL(valueChanged(double)), this, SLOT(onAmbientChanged(double)));
    connect(ui->OutlineAlphaSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onOutlineAlphaChanged(double)));
    connect(ui->OutlineWidthSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onOutlineWidthChanged(double)));

    connect(ui->blurIntensity, SIGNAL(valueChanged(double)), this, SLOT(onBlurIntensityChanged(double)));

    connect(ui->globalDOF, SIGNAL(clicked()), this, SLOT(onVisualHintChanged()));
    connect(ui->distSpin, SIGNAL(valueChanged(double)), this, SLOT(onDOFDistChanged(double)));
    connect(ui->depthSpin, SIGNAL(valueChanged(double)), this, SLOT(onDOFDepthChanged(double)));
    connect(ui->falloffSpin, SIGNAL(valueChanged(double)), this, SLOT(onDOFFalloffChanged(double)));
}

MiscSettingsWidget::~MiscSettingsWidget()
{
    delete ui;
}

void MiscSettingsWidget::onCameraSpeedChanged(double speed)
{
    camera->speed = speed;
}

void MiscSettingsWidget::onCameraFovYChanged(double fovy)
{
    camera->fovy = fovy;
    emit settingsChanged();
}

int g_MaxSubmeshes = 100;

void MiscSettingsWidget::onMaxSubmeshesChanged(int n)
{
    g_MaxSubmeshes = n;
    emit settingsChanged();
}

void MiscSettingsWidget::onAmbientChanged(double val)
{
    miscSettings->AMBIENT = val;
    emit settingsChanged();
}

void MiscSettingsWidget::onBackgroundColorClicked()
{
    QColor color = QColorDialog::getColor(miscSettings->backgroundColor, this, "Background color");
    if (color.isValid())
    {
        QString colorName = color.name();
        ui->buttonBackgroundColor->setStyleSheet(QString::fromLatin1("background-color: %0").arg(colorName));
        miscSettings->backgroundColor = color;



        emit settingsChanged();
    }
}

void MiscSettingsWidget::onBlurIntensityChanged(double val)
{
    miscSettings->blurVal = val;

    emit settingsChanged();
}

void MiscSettingsWidget::onVisualHintChanged()
{
    miscSettings->renderLightSources = ui->checkBoxLightSources->isChecked();
    miscSettings->renderOutline = ui->checkBoxSelectionOutline->isChecked();
    miscSettings->renderGrid = ui->checkBoxGrid->isChecked();

    miscSettings->checkDOF = ui->globalDOF->isChecked();

    emit settingsChanged();
}

void MiscSettingsWidget::onOutlineAlphaChanged(double val)
{
    miscSettings->OutlineAlpha = val;
    emit settingsChanged();
}

void MiscSettingsWidget::onOutlineWidthChanged(double val)
{
    miscSettings->OutlineWidth = val;
    emit settingsChanged();
}

void MiscSettingsWidget::onDOFDistChanged(double val)
{
    miscSettings->dofDepth = val;
    emit settingsChanged();
}

void MiscSettingsWidget::onDOFDepthChanged(double val)
{
    miscSettings->dofFocus = val;
    emit settingsChanged();
}

void MiscSettingsWidget::onDOFFalloffChanged(double val)
{
    miscSettings->dofFalloff = val;
    emit settingsChanged();
}
