#include "camerafactory.h"
#include "mockcamera.h"

// Include platform-specific cameras based on compile definitions
#ifdef HAS_QT_MULTIMEDIA
#include "qtcamera.h"
#endif

#ifdef IS_RASPBERRY_PI
#include "picamera.h"
#endif

#include <QDebug>
#include <QSysInfo>
#include <QFile>

std::unique_ptr<ICamera> CameraFactory::createCamera(CameraType type, QObject* parent) {
    QString platform = QSysInfo::prettyProductName();
    qDebug() << "Using Qt Camera for platform:" << platform;

#ifdef IS_MAC
    // Always use mock camera on macOS for testing
    qDebug() << "Creating camera of type: \"Mock Camera (macOS testing)\"";
    return std::make_unique<MockCamera>(parent);
#else
    switch (type) {
        case AUTO_DETECT:
            if (platform.contains("Raspberry", Qt::CaseInsensitive)) {
                qDebug() << "Creating camera of type: \"Pi Camera\"";
                return std::make_unique<PiCamera>(parent);
            } else {
                qDebug() << "Creating camera of type: \"Qt Camera\"";
                return std::make_unique<QtCamera>(parent);
            }
        case QT_CAMERA:
            qDebug() << "Creating camera of type: \"Qt Camera\"";
            return std::make_unique<QtCamera>(parent);
        case PI_CAMERA:
            qDebug() << "Creating camera of type: \"Pi Camera\"";
            return std::make_unique<PiCamera>(parent);
        case MOCK_CAMERA:
            qDebug() << "Creating camera of type: \"Mock Camera\"";
            return std::make_unique<MockCamera>(parent);
        default:
            qWarning() << "Unknown camera type, falling back to mock";
            return std::make_unique<MockCamera>(parent);
    }
#endif
}

CameraFactory::CameraType CameraFactory::detectBestCamera() {
#ifdef IS_RASPBERRY_PI
    // Check if we're on Raspberry Pi by looking for Pi-specific files
    if (QFile::exists("/proc/device-tree/model")) {
        QFile modelFile("/proc/device-tree/model");
        if (modelFile.open(QIODevice::ReadOnly)) {
            QString model = QString::fromUtf8(modelFile.readAll());
            if (model.contains("Raspberry Pi", Qt::CaseInsensitive)) {
                qDebug() << "Detected Raspberry Pi via device tree:" << model.trimmed();
                return PI_CAMERA;
            }
        }
    }
    
    // Check hostname as backup
    if (QSysInfo::machineHostName().contains("raspberry", Qt::CaseInsensitive) ||
        QSysInfo::productType().contains("raspberry", Qt::CaseInsensitive)) {
        qDebug() << "Detected Raspberry Pi via hostname/product type";
        return PI_CAMERA;
    }
#endif

#ifdef HAS_QT_MULTIMEDIA
    // For Mac/Windows/Linux with Qt Multimedia, try Qt Camera
    qDebug() << "Using Qt Camera for platform:" << QSysInfo::prettyProductName();
    return QT_CAMERA;
#endif

    // Fallback to mock camera
    qDebug() << "No platform-specific camera available, using mock camera";
    return MOCK_CAMERA;
}

QString CameraFactory::cameraTypeToString(CameraType type) {
    switch (type) {
        case QT_CAMERA: return "Qt Camera";
        case PI_CAMERA: return "Raspberry Pi Camera";
        case MOCK_CAMERA: return "Mock Camera";
        case AUTO_DETECT: return "Auto Detect";
        default: return "Unknown Camera";
    }
}