#ifndef PHOTOSESSIONDATA_H
#define PHOTOSESSIONDATA_H

#include <QString>
#include <QDateTime>
#include <QDebug> // For logging

struct PhotoSessionData {
    QDateTime startTime;
    QString chosenWeaponId;
    QString chosenLandId;
    QString chosenCompanionId;
    QString userName;
    QString capturedPhotoPath;

    PhotoSessionData() {
        startTime = QDateTime::currentDateTime();
        qDebug() << "PhotoSessionData: Instance created at" << startTime.toString(Qt::ISODate);
    }

    ~PhotoSessionData() {
        qDebug() << "PhotoSessionData: Instance for user" << (userName.isEmpty() ? "[NoName]" : userName)
                 << "Weapon:" << chosenWeaponId
                 << "Land:" << chosenLandId
                 << "Companion:" << chosenCompanionId
                 << "Photo:" << (capturedPhotoPath.isEmpty() ? "[None]" : capturedPhotoPath)
                 << "destroyed.";
    }

    void clear() {
        // Resets data if you were to reuse the object,
        // but with unique_ptr we'll typically destroy and recreate.
        startTime = QDateTime::currentDateTime();
        chosenWeaponId.clear();
        chosenCompanionId.clear();
        chosenLandId.clear();
        userName.clear();
        capturedPhotoPath.clear();
    }
};

#endif // PHOTOSESSIONDATA_H
