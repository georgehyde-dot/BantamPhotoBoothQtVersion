#ifndef PICAMERA_H
#define PICAMERA_H

#include "icamera.h"
#include <QLabel>
#include <QProcess>

class PiCamera : public ICamera {
    Q_OBJECT

public:
    explicit PiCamera(QObject *parent = nullptr);
    ~PiCamera() override;

    // ICamera interface
    bool initialize() override;
    void cleanup() override;
    bool isAvailable() const override;

    QWidget* getPreviewWidget() override;
    void startPreview() override;
    void stopPreview() override;

    void capturePhoto() override;
    void cancelCapture() override;

private slots:
    void onCaptureProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onCaptureProcessError(QProcess::ProcessError error);

private:
    QLabel* m_previewWidget;
    QProcess* m_captureProcess;
    bool m_initialized;
    bool m_previewActive;
    QString m_photosDirectory;
    QString m_currentCaptureFile;
    
    void setupPhotosDirectory();
    bool checkCameraAvailable();
};

#endif // PICAMERA_H