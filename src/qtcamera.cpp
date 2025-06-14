#include "qtcamera.h"
#include <QCamera>
#include <QVideoWidget>
#include <QImageCapture>
#include <QMediaCaptureSession>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QMediaDevices>
#include <QPermissions>
#include <QCoreApplication>

QtCamera::QtCamera(QObject *parent)
    : ICamera(parent)
    , m_camera(nullptr)
    , m_videoWidget(nullptr)
    , m_imageCapture(nullptr)
    , m_captureSession(nullptr)
    , m_initialized(false)
{
    setupPhotosDirectory();
}

QtCamera::~QtCamera() {
    cleanup();
}

bool QtCamera::initialize() {
    if (m_initialized) {
        return true;
    }

    qDebug() << "QtCamera: Initializing Qt camera";

    // On macOS, try to initialize directly - the system will prompt for permissions
    return initializeCamera();
}

bool QtCamera::initializeCamera() {
    // Check if cameras are available
    const QList<QCameraDevice> cameras = QMediaDevices::videoInputs();
    if (cameras.isEmpty()) {
        qWarning() << "QtCamera: No cameras available";
        return false;
    }

    // Use the default camera
    QCameraDevice cameraDevice = QMediaDevices::defaultVideoInput();
    if (cameraDevice.isNull()) {
        qWarning() << "QtCamera: No default camera found";
        return false;
    }

    qDebug() << "QtCamera: Using camera:" << cameraDevice.description();

    try {
        // Create camera components
        m_camera = new QCamera(cameraDevice, this);
        m_videoWidget = new QVideoWidget();
        m_imageCapture = new QImageCapture(this);
        m_captureSession = new QMediaCaptureSession(this);

        // Setup capture session
        m_captureSession->setCamera(m_camera);
        m_captureSession->setVideoOutput(m_videoWidget);
        m_captureSession->setImageCapture(m_imageCapture);

        // Connect signals
        connect(m_camera, &QCamera::errorOccurred, this, &QtCamera::onCameraError);
        connect(m_camera, &QCamera::activeChanged, this, [this](bool active) {
            qDebug() << "QtCamera: Camera active state changed to:" << active;
        });
        connect(m_imageCapture, &QImageCapture::imageCaptured, this, &QtCamera::onImageCaptured);
        connect(m_imageCapture, &QImageCapture::imageSaved, this, &QtCamera::onImageSaved);
        connect(m_imageCapture, &QImageCapture::errorOccurred, this, &QtCamera::onCaptureError);

        // Configure image capture
        m_imageCapture->setFileFormat(QImageCapture::JPEG);
        m_imageCapture->setQuality(QImageCapture::VeryHighQuality);

        // Test camera activation
        m_camera->start();
        
        m_initialized = true;
        qDebug() << "QtCamera: Initialization complete";
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "QtCamera: Exception during initialization:" << e.what();
        return false;
    } catch (...) {
        qWarning() << "QtCamera: Unknown exception during initialization";
        return false;
    }
}

void QtCamera::cleanup() {
    if (!m_initialized) {
        return;
    }

    qDebug() << "QtCamera: Cleaning up";
    stopPreview();

    if (m_camera) {
        m_camera->stop();
    }

    // Qt will handle deletion via parent-child relationships
    m_camera = nullptr;
    m_imageCapture = nullptr;
    m_captureSession = nullptr;
    
    if (m_videoWidget) {
        m_videoWidget->deleteLater();
        m_videoWidget = nullptr;
    }

    m_initialized = false;
}

bool QtCamera::isAvailable() const {
    return m_initialized && m_camera && m_camera->isAvailable();
}

QWidget* QtCamera::getPreviewWidget() {
    if (!m_initialized) {
        initialize();
    }
    return m_videoWidget;
}

void QtCamera::startPreview() {
    if (!m_initialized || !m_camera) {
        qWarning() << "QtCamera: Cannot start preview - camera not initialized";
        return;
    }

    if (m_camera->isActive()) {
        qDebug() << "QtCamera: Preview already active";
        return;
    }

    qDebug() << "QtCamera: Starting preview";
    m_camera->start();
    emitPreviewStarted();
}

void QtCamera::stopPreview() {
    if (!m_initialized || !m_camera) {
        return;
    }

    if (!m_camera->isActive()) {
        return;
    }

    qDebug() << "QtCamera: Stopping preview";
    m_camera->stop();
    emitPreviewStopped();
}

void QtCamera::capturePhoto() {
    if (!m_initialized || !m_imageCapture) {
        emitCaptureError("Camera not initialized");
        return;
    }

    if (!m_camera->isActive()) {
        emitCaptureError("Camera preview not active");
        return;
    }

    if (!m_imageCapture->isReadyForCapture()) {
        emitCaptureError("Camera not ready for capture");
        return;
    }

    // Generate filename with timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = QString("%1/photo_%2.jpg").arg(m_photosDirectory, timestamp);

    qDebug() << "QtCamera: Capturing photo to" << filename;
    m_imageCapture->captureToFile(filename);
}

void QtCamera::cancelCapture() {
    // Qt's QImageCapture doesn't have a direct cancel method
    // The capture is usually very fast, so this is mainly for interface compliance
    qDebug() << "QtCamera: Capture cancel requested (not directly supported by Qt)";
}

void QtCamera::onImageCaptured(int id, const QImage& image) {
    Q_UNUSED(id)
    qDebug() << "QtCamera: Image captured, size:" << image.size();
    // The actual file saving is handled by onImageSaved
}

void QtCamera::onImageSaved(int id, const QString& fileName) {
    Q_UNUSED(id)
    qDebug() << "QtCamera: Image saved to" << fileName;
    
    // Load the saved image and emit the signal
    QPixmap pixmap(fileName);
    if (!pixmap.isNull()) {
        emitPhotoReady(pixmap, fileName);
    } else {
        emitCaptureError("Failed to load captured image");
    }
}

void QtCamera::onCaptureError(int id, QImageCapture::Error error, const QString& errorString) {
    Q_UNUSED(id)
    Q_UNUSED(error)
    qWarning() << "QtCamera: Capture error:" << errorString;
    emitCaptureError(errorString);
}

void QtCamera::onCameraError(QCamera::Error error) {
    QString errorString;
    switch (error) {
        case QCamera::NoError:
            return; // No error, nothing to do
        case QCamera::CameraError:
            errorString = "General camera error";
            break;
        default:
            errorString = "Unknown camera error";
            break;
    }
    
    qWarning() << "QtCamera: Camera error:" << errorString;
    emitCaptureError(errorString);
}

void QtCamera::setupPhotosDirectory() {
    m_photosDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/PhotoBooth";
    QDir().mkpath(m_photosDirectory);
    qDebug() << "QtCamera: Photos directory:" << m_photosDirectory;
}