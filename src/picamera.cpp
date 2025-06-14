#include "picamera.h"
#include <QLabel>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QPixmap>
#include <QFile>

PiCamera::PiCamera(QObject *parent)
    : ICamera(parent)
    , m_previewWidget(nullptr)
    , m_captureProcess(nullptr)
    , m_initialized(false)
    , m_previewActive(false)
{
    setupPhotosDirectory();
}

PiCamera::~PiCamera() {
    cleanup();
}

bool PiCamera::initialize() {
    if (m_initialized) {
        return true;
    }

    qDebug() << "PiCamera: Initializing Raspberry Pi camera";

    if (!checkCameraAvailable()) {
        qWarning() << "PiCamera: Camera not available";
        return false;
    }

    // Create preview widget
    m_previewWidget = new QLabel();
    m_previewWidget->setAlignment(Qt::AlignCenter);
    m_previewWidget->setStyleSheet("background-color: #34495e; color: white; font-size: 16px;");
    m_previewWidget->setText("Raspberry Pi Camera\nPreview");
    m_previewWidget->setMinimumSize(640, 480);

    // Create capture process
    m_captureProcess = new QProcess(this);
    connect(m_captureProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &PiCamera::onCaptureProcessFinished);
    connect(m_captureProcess, &QProcess::errorOccurred,
            this, &PiCamera::onCaptureProcessError);

    m_initialized = true;
    qDebug() << "PiCamera: Initialization complete";
    return true;
}

void PiCamera::cleanup() {
    if (!m_initialized) {
        return;
    }

    qDebug() << "PiCamera: Cleaning up";
    stopPreview();

    if (m_captureProcess) {
        if (m_captureProcess->state() != QProcess::NotRunning) {
            m_captureProcess->kill();
            m_captureProcess->waitForFinished(3000);
        }
        m_captureProcess = nullptr;
    }

    if (m_previewWidget) {
        m_previewWidget->deleteLater();
        m_previewWidget = nullptr;
    }

    m_initialized = false;
}

bool PiCamera::isAvailable() const {
    return m_initialized && checkCameraAvailable();
}

QWidget* PiCamera::getPreviewWidget() {
    if (!m_initialized) {
        initialize();
    }
    return m_previewWidget;
}

void PiCamera::startPreview() {
    if (!m_initialized || m_previewActive) {
        return;
    }

    qDebug() << "PiCamera: Starting preview";
    m_previewActive = true;
    
    if (m_previewWidget) {
        m_previewWidget->setText("Raspberry Pi Camera\nPreview Active");
        m_previewWidget->setStyleSheet("background-color: #27ae60; color: white; font-size: 16px;");
    }
    
    emitPreviewStarted();
}

void PiCamera::stopPreview() {
    if (!m_previewActive) {
        return;
    }

    qDebug() << "PiCamera: Stopping preview";
    m_previewActive = false;
    
    if (m_previewWidget) {
        m_previewWidget->setText("Raspberry Pi Camera\nPreview Stopped");
        m_previewWidget->setStyleSheet("background-color: #34495e; color: white; font-size: 16px;");
    }
    
    emitPreviewStopped();
}

void PiCamera::capturePhoto() {
    if (!m_initialized) {
        emitCaptureError("Camera not initialized");
        return;
    }

    if (m_captureProcess->state() != QProcess::NotRunning) {
        emitCaptureError("Capture already in progress");
        return;
    }

    // Generate filename with timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    m_currentCaptureFile = QString("%1/pi_photo_%2.jpg").arg(m_photosDirectory, timestamp);

    qDebug() << "PiCamera: Capturing photo to" << m_currentCaptureFile;

    // Use libcamera-still (modern Pi camera command)
    QStringList arguments;
    arguments << "-o" << m_currentCaptureFile;
    arguments << "--width" << "1920";
    arguments << "--height" << "1080";
    arguments << "--quality" << "95";
    arguments << "--timeout" << "1"; // 1ms timeout (immediate capture)

    m_captureProcess->start("libcamera-still", arguments);
    
    if (!m_captureProcess->waitForStarted(3000)) {
        // Fallback to raspistill for older Pi OS versions
        qDebug() << "PiCamera: libcamera-still failed, trying raspistill";
        arguments.clear();
        arguments << "-o" << m_currentCaptureFile;
        arguments << "-w" << "1920";
        arguments << "-h" << "1080";
        arguments << "-q" << "95";
        arguments << "-t" << "1";
        
        m_captureProcess->start("raspistill", arguments);
        
        if (!m_captureProcess->waitForStarted(3000)) {
            emitCaptureError("Failed to start camera capture process");
        }
    }
}

void PiCamera::cancelCapture() {
    if (m_captureProcess && m_captureProcess->state() != QProcess::NotRunning) {
        qDebug() << "PiCamera: Cancelling capture";
        m_captureProcess->kill();
    }
}

void PiCamera::onCaptureProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    qDebug() << "PiCamera: Capture process finished with exit code:" << exitCode;

    if (exitStatus != QProcess::NormalExit || exitCode != 0) {
        emitCaptureError(QString("Capture process failed with exit code: %1").arg(exitCode));
        return;
    }

    // Check if file was created and load it
    if (QFile::exists(m_currentCaptureFile)) {
        QPixmap photo(m_currentCaptureFile);
        if (!photo.isNull()) {
            qDebug() << "PiCamera: Photo captured successfully:" << m_currentCaptureFile;
            emitPhotoReady(photo, m_currentCaptureFile);