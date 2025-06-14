#ifndef QTCAMERA_H
#define QTCAMERA_H

#include "icamera.h"
#include <QCamera>
#include <QVideoWidget>
#include <QImageCapture>
#include <QMediaCaptureSession>

class QtCamera : public ICamera {
    Q_OBJECT

public:
    explicit QtCamera(QObject *parent = nullptr);
    ~QtCamera() override;

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
    void onImageCaptured(int id, const QImage& image);
    void onImageSaved(int id, const QString& fileName);
    void onCaptureError(int id, QImageCapture::Error error, const QString& errorString);
    void onCameraError(QCamera::Error error);

private:
    QCamera* m_camera;
    QVideoWidget* m_videoWidget;
    QImageCapture* m_imageCapture;
    QMediaCaptureSession* m_captureSession;
    bool m_initialized;
    QString m_photosDirectory;
    
    bool initializeCamera();
    void setupPhotosDirectory();
};

#endif // QTCAMERA_H