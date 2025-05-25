#include "mainwindow.h"
#include "photosessiondata.h" // Include our session data struct

#include <QApplication> // For quit
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <utility>
#include <vector>
#include <QPixmap>
#include <QIcon>
#include <QDebug>
#include <QDateTime> // For PhotoSessionData
#include <QScreen>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    loadPersistentChoiceImages();

    setupUi();
    setWindowTitle("Qt Photo Booth");
}

MainWindow::~MainWindow() {
    // m_currentSessionData unique_ptr will automatically delete the object if it holds one.
    // Qt's parent-child system will delete UI widgets.
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

    m_stackedWidget->addWidget(m_startScreenWidget);
    m_stackedWidget->addWidget(m_weaponChoiceScreenWidget);
    m_stackedWidget->addWidget(m_landChoiceScreenWidget);
    m_stackedWidget->addWidget(m_companionChoiceScreenWidget);
    m_stackedWidget->addWidget(m_nameEntryScreenWidget);

    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentWidget(m_startScreenWidget); 
}

QWidget* MainWindow::createStartScreen() {
    QWidget *widget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(50, 50, 50, 50); // Add some padding

    m_startButton = new QPushButton("START PHOTO BOOTH", widget);
    m_startButton->setMinimumSize(300, 100); // Make button larger
    QFont startFont = m_startButton->font();
    startFont.setPointSize(24);
    m_startButton->setFont(startFont);
    connect(m_startButton, &QPushButton::clicked, this, &MainWindow::onStartButtonClicked);

    layout->addStretch(); 
    layout->addWidget(m_startButton, 0, Qt::AlignCenter);
    layout->addStretch();

    widget->setLayout(layout);
    return widget;
}

QWidget* MainWindow::createChoiceScreen(const QString& title,
                                        const QString& categoryPrefix,
                                        int itemCount, 
                                        const char* slot) {
    QWidget *widget = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(widget);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    mainLayout->setSpacing(30);

    QLabel *promptLabel = new QLabel(title, widget);
    promptLabel->setAlignment(Qt::AlignCenter);
    QFont promptFont = promptLabel->font();
    promptFont.setPointSize(28);
    promptLabel->setFont(promptFont);
    mainLayout->addWidget(promptLabel, 0, Qt::AlignCenter);

    QHBoxLayout *imageButtonsLayout = new QHBoxLayout();
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
    QWidget *widget = new QWidget;
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

// --- SLOTS ---
void MainWindow::onStartButtonClicked() {
    qDebug() << "Start button clicked.";
    startNewSession();
    if (m_stackedWidget->widget(1)) { // Go to weapon choice screen
        m_stackedWidget->setCurrentIndex(1);
    }
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
    returnToStartScreen();
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
    if (m_nameLineEdit) m_nameLineEdit->clear();
    m_stackedWidget->setCurrentIndex(0); // Go back to start screen
    qDebug() << "Returned to start screen. Session data cleared.";
}
