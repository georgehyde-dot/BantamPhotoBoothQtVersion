#ifndef MOCKCAMERA_H
#define MOCKCAMERA_H

#include "icamera.h"
#include <QLabel>
#include <QTimer>
#include <QPixmap>

class MockCamera : public ICamera {
    Q_OBJECT

public:
    explicit MockCamera(QObject *parent = nullptr);
    ~MockCamera() override;

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
    // void updatePreview();
    // void simulateCapture();

private:
    void setupPhotosDirectory();
    void simulatePhotoCapture();
    QPixmap createTestPhoto(); // Add this line
    
    QLabel *m_previewWidget;
    QTimer *m_captureTimer;
    QString m_photosDirectory;
    bool m_initialized;
    
    QPixmap generateMockFrame();
    QPixmap generateMockPhoto();
};

#endif // MOCKCAMERA_H