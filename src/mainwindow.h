#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <map>
#include <QString>
#include <QPixmap>
#include <qwidget.h>
#include <QTimer>

class QStackedWidget;
class QPushButton;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPixmap;
class ICamera;

struct PhotoSessionData;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Slots for button clicks
    void onStartButtonClicked();
    void onExitButtonClicked();
    void onWeaponSelected(const QString& weaponId);
    void onLandSelected(const QString& landId);
    void onCompanionSelected(const QString& companionId);
    void onNameSubmitButtonClicked();
    
    // Camera slots
    void onTakePhotoButtonClicked();
    void onRetakeButtonClicked();
    void onCountdownTick();
    void onCameraPhotoReady(const QPixmap& photo, const QString& filePath);
    void onCameraError(const QString& errorMessage);

private:
    void setupUi();
    void setupCamera();
    
    // Screen creators
    QWidget* createStartScreen();
    QWidget* createNameEntryScreen();
    QWidget* createCameraScreen();

    QWidget* createChoiceScreen(
                                const QString& title, 
                                const QString& categoryPrefix, 
                                int itemCount, 
                                const char* slot);
    
    // Camera functionality
    void startCameraPreview();
    void stopCameraPreview();
    void startCountdown();
    void stopCountdown();
    void capturePhoto();

    // Session Management
    void startNewSession();
    void processNameEntry();
    void returnToStartScreen(); // Clears session and goes to start

    void loadPersistentChoiceImages();
    // UI Elements (pointers will be managed by Qt's parent-child or layouts)
    QStackedWidget *m_stackedWidget;

    QPushButton *m_startButton;
    QPushButton *m_exitButton;

    // Name Entry Screen
    QLineEdit *m_nameLineEdit;
    QPushButton *m_submitNameButton;
    
    // Camera Screen
    QWidget *m_cameraPreviewWidget;
    QLabel *m_countdownLabel;
    QPushButton *m_takePhotoButton;
    QPushButton *m_retakeButton;
    QLabel *m_capturedPhotoLabel;

    // Pointers to screen widgets for QStackedWidget
    QWidget *m_startScreenWidget;
    QWidget *m_weaponChoiceScreenWidget;
    QWidget *m_landChoiceScreenWidget;
    QWidget *m_companionChoiceScreenWidget;
    QWidget *m_nameEntryScreenWidget;
    QWidget *m_cameraScreenWidget;

    // Camera system
    std::unique_ptr<ICamera> m_camera;
    QTimer *m_countdownTimer;
    int m_countdownValue;
    static const int COUNTDOWN_SECONDS = 3;

    // Persistent Data (Loaded once)
    std::map<QString, QPixmap> m_selectableImages;
    // Per-Iteration Data
    std::unique_ptr<PhotoSessionData> m_currentSessionData;
};

#endif // MAINWINDOW_H
