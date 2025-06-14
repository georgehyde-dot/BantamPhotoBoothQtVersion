#ifndef CAMERAFACTORY_H
#define CAMERAFACTORY_H

#include "icamera.h"
#include <memory>
#include <QObject>

class CameraFactory {
public:
    enum CameraType {
        AUTO_DETECT,
        QT_CAMERA,      // For Mac/Windows/Linux with standard cameras
        PI_CAMERA,      // For Raspberry Pi camera
        MOCK_CAMERA     // For testing/development
    };

    static std::unique_ptr<ICamera> createCamera(CameraType type = AUTO_DETECT, QObject* parent = nullptr);
    static CameraType detectBestCamera();
    static QString cameraTypeToString(CameraType type);
};

#endif // CAMERAFACTORY_H