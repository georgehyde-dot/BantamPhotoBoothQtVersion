#include "./mainwindow.h"
#include "photosessiondata.h" // Include our session data struct

#include <QApplication> // For quit
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <qwidget.h>
#include <utility>
#include <vector>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QDateTime> // For PhotoSessionData
#include <QScreen>
#include <QString>
#include <QWidget>
#include "camerafactory.h"
#include "icamera.h"
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_countdownTimer(new QTimer(this)),
    m_countdownValue(0) {
        loadPersistentChoiceImages();
        setupCamera();
        setupUi();
        setWindowTitle("Qt Photo Booth");

    }


MainWindow::~MainWindow() {
    // m_currentSessionData unique_ptr will automatically delete the object if it holds one.
    // Qt's parent-child system will delete UI widgets.
    if (m_camera) {
        m_camera->stopPreview();
        m_camera->cleanup();
    }
}

void MainWindow::setupCamera() {
    m_camera = CameraFactory::createCamera(CameraFactory::AUTO_DETECT, this);
    
    if (!m_camera->initialize()) {
        qWarning() << "Failed to initialize camera, falling back to mock camera";
        m_camera = CameraFactory::createCamera(CameraFactory::MOCK_CAMERA, this);
        if (!m_camera->initialize()) {
            qCritical() << "Failed to initialize even mock camera!";
        }
    }
    
    // Connect camera signals
    connect(m_camera.get(), &ICamera::photoReady, this, &MainWindow::onCameraPhotoReady);
    connect(m_camera.get(), &ICamera::captureError, this, &MainWindow::onCameraError);
    
    // Setup countdown timer
    connect(m_countdownTimer, &QTimer::timeout, this, &MainWindow::onCountdownTick);
}

void MainWindow::loadPersistentChoiceImages() {
    const std::vector<std::pair<QString, int>> imageCategories = {
        {"weapon", 4},
        {"land", 4},
        {"companion", 4},
    };
    const int iconWidth = 150;
    const int iconHeight = 150;

    for (const auto& categoryPair : imageCategories) {
        const QString& categoryName = categoryPair.first;
        int count = categoryPair.second;
        for (int i = 1; i <= count; ++i) {
            QString imageKey = QString("%1%2").arg(categoryName).arg(i); 
            QString resourcePath = QString(":/%1.jpg").arg(imageKey);

            QPixmap pixmap;

             if (pixmap.load(resourcePath)) {
                m_selectableImages[imageKey] = pixmap.scaled(iconWidth, iconHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);
                qDebug() << "Loaded persistent image:" << resourcePath << "as key:" << imageKey;
            } else {
                qWarning() << "Failed to load persistent image:" << resourcePath << "for key:" << imageKey;
            }
        }
    }
}

void MainWindow::setupUi() {
    m_stackedWidget = new QStackedWidget(this);

    m_startScreenWidget = createStartScreen();
    m_weaponChoiceScreenWidget = createChoiceScreen("Choose Your Weapon", "weapon", 4, SLOT(onWeaponSelected(QString)));
    m_landChoiceScreenWidget = createChoiceScreen("Choose Your Land", "land", 4, SLOT(onLandSelected(QString)));
    m_companionChoiceScreenWidget = createChoiceScreen("Choose Your Companion", "companion", 4, SLOT(onCompanionSelected(QString)));
    m_nameEntryScreenWidget = createNameEntryScreen();
    m_cameraScreenWidget = createCameraScreen(); 

    m_stackedWidget->addWidget(m_startScreenWidget);
    m_stackedWidget->addWidget(m_weaponChoiceScreenWidget);
    m_stackedWidget->addWidget(m_landChoiceScreenWidget);
    m_stackedWidget->addWidget(m_companionChoiceScreenWidget);
    m_stackedWidget->addWidget(m_nameEntryScreenWidget);
    m_stackedWidget->addWidget(m_cameraScreenWidget);

    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentWidget(m_startScreenWidget); 
}

QWidget* MainWindow::createStartScreen() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(50, 50, 50, 50); // Add some padding

    m_startButton = new QPushButton("START PHOTO BOOTH", widget);
    m_startButton->setMinimumSize(300, 100); // Make button larger

    QFont startFont = m_startButton->font();
    startFont.setPointSize(24);
    m_startButton->setFont(startFont);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);

    m_exitButton = new QPushButton("EXIT", widget);
    m_exitButton->setMinimumSize(100, 100);
    QFont exitFont = m_exitButton->font();
    exitFont.setPointSize(16);
    connect(m_exitButton, &QPushButton::clicked, this, &MainWindow::onExitButtonClicked);

    layout->addStretch(); 
    layout->addWidget(m_startButton, 0, Qt::AlignCenter);
    layout->addWidget(m_exitButton, 0, Qt::AlignTop | Qt::AlignRight);
    layout->addStretch();

    widget->setLayout(layout);
    return widget;
}

QWidget* MainWindow::createChoiceScreen(
                                        const QString& title,
                                        const QString& categoryPrefix,
                                        int itemCount, 
                                        const char* slot) {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);

    QLabel *promptLabel = new QLabel(title, widget);
    promptLabel->setAlignment(Qt::AlignCenter);
    QFont promptFont = promptLabel->font();
    promptFont.setPointSize(28);
    promptLabel->setFont(promptFont);
    mainLayout->addWidget(promptLabel, 0, Qt::AlignCenter);

    QHBoxLayout *imageButtonsLayout = new QHBoxLayout(widget);
    imageButtonsLayout->setSpacing(20);

    for (int i = 1; i <= itemCount; ++i) {
        QString imageKey = QString("%1%2").arg(categoryPrefix).arg(i);
        if (m_selectableImages.count(imageKey)) {
            QPushButton *button = new QPushButton(widget);
            const QPixmap& pixmap = m_selectableImages.at(imageKey);
            button->setIcon(QIcon(pixmap));
            button->setIconSize(pixmap.size()); 
            // Make button slightly larger than icon for touchability
            button->setFixedSize(pixmap.width() + 30, pixmap.height() + 30); 
            button->setStyleSheet("QPushButton { border: 2px solid #555; border-radius: 10px; } QPushButton:pressed { background-color: #ddd; }");


            // Connect using a lambda to pass the imageKey to the slot
            // The slot signature needs to be `void onCategorySelected(const QString& id);`
            // We'll map this to specific slots like onWeaponSelected for now.
            // A more advanced way would use QSignalMapper or a single slot with sender() identification.
            // For this structure, we'll connect directly to the specific slots.
            if (categoryPrefix == "weapon") {
                connect(button, &QPushButton::clicked, [this, imageKey]() { this->onWeaponSelected(imageKey); });
            } else if (categoryPrefix == "land") {
                connect(button, &QPushButton::clicked, [this, imageKey]() { this->onLandSelected(imageKey); });
            } else if (categoryPrefix == "companion") {
                connect(button, &QPushButton::clicked, [this, imageKey]() { this->onCompanionSelected(imageKey); });
            }
            imageButtonsLayout->addWidget(button, 0, Qt::AlignCenter); // Center button in its cell
        } else {
            qWarning() << "Image key not found in m_selectableImages:" << imageKey;
        }
    }

    imageButtonsLayout->addStretch();
    imageButtonsLayout->addStretch(1);
    mainLayout->addLayout(imageButtonsLayout);
    imageButtonsLayout->addStretch(2);

   // widget->setLayout(mainLayout);
    return widget;
}

QWidget* MainWindow::createNameEntryScreen() {
    QWidget *widget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(50, 50, 50, 50);
    layout->setSpacing(20);

    QLabel *namePromptLabel = new QLabel("Enter Your Name:", widget);
    QFont promptFont = namePromptLabel->font();
    promptFont.setPointSize(24);
    namePromptLabel->setFont(promptFont);
    namePromptLabel->setAlignment(Qt::AlignCenter);

    m_nameLineEdit = new QLineEdit(widget);
    m_nameLineEdit->setMinimumHeight(60);
    m_nameLineEdit->setFont(promptFont);
    m_nameLineEdit->setAlignment(Qt::AlignCenter);
    m_nameLineEdit->setPlaceholderText("Your Name");

    m_submitNameButton = new QPushButton("Next", widget);
    m_submitNameButton->setMinimumSize(200, 80);
    m_submitNameButton->setFont(promptFont);
    connect(m_submitNameButton, &QPushButton::clicked, this, &MainWindow::onNameSubmitButtonClicked);

    layout->addStretch(1);
    layout->addWidget(namePromptLabel, 0, Qt::AlignCenter);
    layout->addWidget(m_nameLineEdit);
    layout->addWidget(m_submitNameButton, 0, Qt::AlignCenter);
    layout->addStretch(2); // More stretch at the bottom

    widget->setLayout(layout);
    return widget;
}

QWidget* MainWindow::createCameraScreen() {
    QWidget *widget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(20);

    // Get camera preview widget
    m_cameraPreviewWidget = m_camera->getPreviewWidget();
    m_cameraPreviewWidget->setMinimumSize(640, 480);
    m_cameraPreviewWidget->setStyleSheet("border: 2px solid #333; background-color: black;");

    // Countdown label (overlay on preview)
    m_countdownLabel = new QLabel(widget);
    m_countdownLabel->setAlignment(Qt::AlignCenter);
    m_countdownLabel->setStyleSheet(
        "QLabel { "
        "color: white; "
        "background-color: rgba(0, 0, 0, 128); "
        "border-radius: 50px; "
        "font-size: 72px; "
        "font-weight: bold; "
        "min-width: 100px; "
        "min-height: 100px; "
        "}"
    );
    m_countdownLabel->hide();

    // Captured photo display (hidden initially)
    m_capturedPhotoLabel = new QLabel(widget);
    m_capturedPhotoLabel->setAlignment(Qt::AlignCenter);
    m_capturedPhotoLabel->setMinimumSize(640, 480);
    m_capturedPhotoLabel->setStyleSheet("border: 2px solid #333;");
    m_capturedPhotoLabel->hide();

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_takePhotoButton = new QPushButton("Take Photo", widget);
    m_takePhotoButton->setMinimumSize(200, 80);
    m_takePhotoButton->setStyleSheet(
        "QPushButton { "
        "background-color: #4CAF50; "
        "color: white; "
        "border: none; "
        "border-radius: 10px; "
        "font-size: 18px; "
        "font-weight: bold; "
        "} "
        "QPushButton:pressed { background-color: #45a049; }"
    );
    connect(m_takePhotoButton, &QPushButton::clicked, this, &MainWindow::onTakePhotoButtonClicked);

    m_retakeButton = new QPushButton("Retake", widget);
    m_retakeButton->setMinimumSize(150, 80);
    m_retakeButton->setStyleSheet(
        "QPushButton { "
        "background-color: #f44336; "
        "color: white; "
        "border: none; "
        "border-radius: 10px; "
        "font-size: 18px; "
        "} "
        "QPushButton:pressed { background-color: #da190b; }"
    );
    m_retakeButton->hide(); // Hidden until photo is taken
    connect(m_retakeButton, &QPushButton::clicked, this, &MainWindow::onRetakeButtonClicked);

    QPushButton *continueButton = new QPushButton("Continue", widget);
    continueButton->setMinimumSize(150, 80);
    continueButton->setStyleSheet(
        "QPushButton { "
        "background-color: #2196F3; "
        "color: white; "
        "border: none; "
        "border-radius: 10px; "
        "font-size: 18px; "
        "} "
        "QPushButton:pressed { background-color: #1976D2; }"
    );
    continueButton->hide(); // Hidden until photo is taken
    connect(continueButton, &QPushButton::clicked, this, &MainWindow::returnToStartScreen);

    buttonLayout->addStretch();
    buttonLayout->addWidget(m_takePhotoButton);
    buttonLayout->addWidget(m_retakeButton);
    buttonLayout->addWidget(continueButton);
    buttonLayout->addStretch();

    // Layout assembly
    mainLayout->addWidget(m_cameraPreviewWidget);
    mainLayout->addWidget(m_capturedPhotoLabel);
    mainLayout->addLayout(buttonLayout);

    // Position countdown label as overlay
    m_countdownLabel->setParent(widget);
    m_countdownLabel->raise();

    return widget;
}

void MainWindow::startCameraPreview() {
    if (m_camera) {
        m_camera->startPreview();
        m_cameraPreviewWidget->show();
        m_capturedPhotoLabel->hide();
        m_takePhotoButton->show();
        m_retakeButton->hide();
    }
}

void MainWindow::stopCameraPreview() {
    if (m_camera) {
        m_camera->stopPreview();
    }
    stopCountdown();
}

void MainWindow::startCountdown() {
    m_countdownValue = COUNTDOWN_SECONDS;
    m_countdownLabel->setText(QString::number(m_countdownValue));
    m_countdownLabel->show();
    
    // Position countdown label in center of preview widget
    QRect previewRect = m_cameraPreviewWidget->geometry();
    int labelSize = 100;
    m_countdownLabel->setGeometry(
        previewRect.x() + (previewRect.width() - labelSize) / 2,
        previewRect.y() + (previewRect.height() - labelSize) / 2,
        labelSize, labelSize
    );
    
    m_countdownTimer->start(1000); // 1 second intervals
    m_takePhotoButton->setEnabled(false);
}

void MainWindow::stopCountdown() {
    m_countdownTimer->stop();
    m_countdownLabel->hide();
    m_takePhotoButton->setEnabled(true);
}

void MainWindow::capturePhoto() {
    if (m_camera) {
        m_camera->capturePhoto();
    }
}

// --- SLOTS ---
void MainWindow::onStartButtonClicked() {
    qDebug() << "Start button clicked.";
    startNewSession();
    if (m_stackedWidget->widget(1)) { // Go to weapon choice screen
        m_stackedWidget->setCurrentIndex(1);
    }
}
void MainWindow::onExitButtonClicked() {
    qDebug() << "Exit button clicked.";
    QApplication::closeAllWindows();
    QApplication::quit();
}
void MainWindow::onWeaponSelected(const QString& weaponId) {
    qDebug() << "Weapon Selected: " << weaponId;
     if (m_currentSessionData){ 
            m_currentSessionData->chosenWeaponId = weaponId;
        }
    m_stackedWidget->setCurrentIndex(2);
}

void MainWindow::onLandSelected(const QString& landId) {
    qDebug() << "Land Selected: " << landId;
     if (m_currentSessionData){ 
            m_currentSessionData->chosenLandId = landId;
        }
    m_stackedWidget->setCurrentIndex(3);
}

void MainWindow::onCompanionSelected(const QString& companionId) {
    qDebug() << "Companion Selected: " << companionId;
     if (m_currentSessionData){ 
            m_currentSessionData->chosenCompanionId = companionId;
        }
    m_stackedWidget->setCurrentIndex(4); // Go to Name Entry Screen 
    if (m_nameLineEdit) m_nameLineEdit->setFocus(); // Focus keyboard 
}

void MainWindow::onNameSubmitButtonClicked() {
    qDebug() << "Submit name button clicked.";
    processNameEntry();
    // For now, just log and go back to start. Later, this will go to camera preview.
    if (m_currentSessionData) {
        qDebug() << "Session Data Collected: User -" << m_currentSessionData->userName
                 << ", Weapon -" << m_currentSessionData->chosenWeaponId
                 << ", Land -" << m_currentSessionData->chosenLandId
                 << ", Companion -" << m_currentSessionData->chosenCompanionId
                 << ", Started at -" << m_currentSessionData->startTime.toString();
    }
    m_stackedWidget->setCurrentIndex(5);
    startCameraPreview();
}

void MainWindow::onTakePhotoButtonClicked() {
    qDebug() << "Take photo button clicked";
    startCountdown();
}

void MainWindow::onRetakeButtonClicked() {
    qDebug() << "Retake button clicked";
    startCameraPreview();
}

void MainWindow::onCountdownTick() {
    m_countdownValue--;
    
    if (m_countdownValue > 0) {
        m_countdownLabel->setText(QString::number(m_countdownValue));
    } else {
        // Countdown finished, take photo
        stopCountdown();
        m_countdownLabel->setText("📸");
        m_countdownLabel->show();
        
        // Brief delay to show camera icon, then capture
        QTimer::singleShot(500, this, [this]() {
            m_countdownLabel->hide();
            capturePhoto();
        });
    }
}

void MainWindow::onCameraPhotoReady(const QPixmap& photo, const QString& filePath) {
    qDebug() << "Photo captured successfully:" << filePath;
    
    // Store photo in session data
    if (m_currentSessionData) {
        m_currentSessionData->capturedPhotoPath = filePath;
    }
    
    // Hide preview, show captured photo
    m_cameraPreviewWidget->hide();
    m_capturedPhotoLabel->setPixmap(photo.scaled(
        m_capturedPhotoLabel->size(), 
        Qt::KeepAspectRatio, 
        Qt::SmoothTransformation
    ));
    m_capturedPhotoLabel->show();
    
    // Update button visibility
    m_takePhotoButton->hide();
    m_retakeButton->show();
    
    // Show continue button
    QWidget* cameraScreen = m_cameraScreenWidget;
    QList<QPushButton*> buttons = cameraScreen->findChildren<QPushButton*>();
    for (QPushButton* button : buttons) {
        if (button->text() == "Continue") {
            button->show();
            break;
        }
    }
}

void MainWindow::onCameraError(const QString& errorMessage) {
    qWarning() << "Camera error:" << errorMessage;
    
    // Show error to user (you might want to create a proper error dialog)
    m_countdownLabel->setText("Error!");
    m_countdownLabel->setStyleSheet(
        "QLabel { "
        "color: red; "
        "background-color: rgba(255, 255, 255, 200); "
        "border-radius: 10px; "
        "font-size: 24px; "
        "font-weight: bold; "
        "padding: 10px; "
        "}"
    );
    m_countdownLabel->show();
    
    // Hide error after 3 seconds
    QTimer::singleShot(3000, this, [this]() {
        m_countdownLabel->hide();
        // Reset countdown label style
        m_countdownLabel->setStyleSheet(
            "QLabel { "
            "color: white; "
            "background-color: rgba(0, 0, 0, 128); "
            "border-radius: 50px; "
            "font-size: 72px; "
            "font-weight: bold; "
            "min-width: 100px; "
            "min-height: 100px; "
            "}"
        );
    });
    
    stopCountdown();
}



// --- Session Management ---
void MainWindow::startNewSession() {
    m_currentSessionData = std::make_unique<PhotoSessionData>();
    if(m_nameLineEdit) m_nameLineEdit->clear(); 
    qDebug() << "New photo session started. Session data object created.";
}

void MainWindow::processNameEntry() {
    if (m_currentSessionData && m_nameLineEdit) {
        m_currentSessionData->userName = m_nameLineEdit->text();
        qDebug() << "Name entered:" << m_currentSessionData->userName;
    } else {
        qWarning() << "processNameEntry called without active session data or line edit!";
    }
}

void MainWindow::returnToStartScreen() {
    m_currentSessionData.reset(); // Destroys current session data, calling its destructor
    if (QApplication::inputMethod()->isVisible()) {
        QApplication::inputMethod()->hide();
    }
    QApplication::inputMethod()->reset();
    if (m_nameLineEdit) m_nameLineEdit->clear();
    m_stackedWidget->setCurrentIndex(0); // Go back to start screen
    qDebug() << "Returned to start screen. Session data cleared.";
}
