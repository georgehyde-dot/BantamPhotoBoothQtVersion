#ifndef ICAMERA_H
#define ICAMERA_H

#include <QObject>
#include <QWidget>
#include <QPixmap>
#include <QString>

class ICamera : public QObject {
    Q_OBJECT

public:
    explicit ICamera(QObject *parent = nullptr) : QObject(parent) {}
    virtual ~ICamera() = default;

    // Camera lifecycle
    virtual bool initialize() = 0;
    virtual void cleanup() = 0;
    virtual bool isAvailable() const = 0;

    // Preview functionality
    virtual QWidget* getPreviewWidget() = 0;
    virtual void startPreview() = 0;
    virtual void stopPreview() = 0;

    // Capture functionality
    virtual void capturePhoto() = 0;
    virtual void cancelCapture() = 0;

signals:
    void photoReady(const QPixmap& photo, const QString& filePath);
    void captureError(const QString& errorMessage);
    void previewStarted();
    void previewStopped();

protected:
    // Helper for implementations to emit signals
    void emitPhotoReady(const QPixmap& photo, const QString& filePath) {
        emit photoReady(photo, filePath);
    }
    void emitCaptureError(const QString& error) {
        emit captureError(error);
    }
    void emitPreviewStarted() {
        emit previewStarted();
    }
    void emitPreviewStopped() {
        emit previewStopped();
    }
};

#endif // ICAMERA_H