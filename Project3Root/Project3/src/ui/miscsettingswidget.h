#ifndef MISCSETTINGSWIDGET_H
#define MISCSETTINGSWIDGET_H

#include <QWidget>

namespace Ui {
class MiscSettingsWidget;
}

class MiscSettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MiscSettingsWidget(QWidget *parent = nullptr);
    ~MiscSettingsWidget();

signals:

    void settingsChanged();

public slots:

    void updateSettings();
    void onCameraSpeedChanged(double speed);
    void onCameraFovYChanged(double speed);
    void onMaxSubmeshesChanged(int n);
    void onBackgroundColorClicked();
    void onGridColorClicked();
    void onVisualHintChanged();
    void onAmbientChanged(double);

    void onOutlineWidthChanged(double);
    void onOutlineAlphaChanged(double);

    void onBlurIntensityChanged(double);

    void onDOFDistChanged(double);
    void onDOFDepthChanged(double);
    void onDOFFalloffChanged(double);

    void onSSAORadChanged(double);

private:
    Ui::MiscSettingsWidget *ui;
};

#endif // MISCSETTINGSWIDGET_H
