#include "mockcamera.h"
#include <QLabel>
#include <QTimer>
#include <QPixmap>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>
#include <QPainter>
#include <QFont>

MockCamera::MockCamera(QObject *parent)
    : ICamera(parent)
    , m_previewWidget(nullptr)
    , m_captureTimer(nullptr)
    , m_initialized(false)
{
    setupPhotosDirectory();
}

MockCamera::~MockCamera() {
    cleanup();
}

bool MockCamera::initialize() {
    if (m_initialized) {
        return true;
    }

    qDebug() << "MockCamera: Initializing mock camera";
    
    // Create preview widget
    m_previewWidget = new QLabel();
    m_previewWidget->setAlignment(Qt::AlignCenter);
    m_previewWidget->setStyleSheet("background-color: #2c3e50; color: white; font-size: 18px;");
    m_previewWidget->setText("📷 Mock Camera Preview\n\nClick 'Take Photo' to capture a test image");
    m_previewWidget->setMinimumSize(640, 480);
    
    // Create capture timer for simulating photo delay
    m_captureTimer = new QTimer(this);
    m_captureTimer->setSingleShot(true);
    connect(m_captureTimer, &QTimer::timeout, this, &MockCamera::simulatePhotoCapture);
    
    m_initialized = true;
    qDebug() << "MockCamera: Initialization complete";
    return true;
}

void MockCamera::cleanup() {
    qDebug() << "MockCamera: Cleaning up";
    
    if (m_captureTimer) {
        m_captureTimer->stop();
    }
    
    // m_previewWidget will be deleted by Qt's parent-child system
    m_initialized = false;
}

bool MockCamera::isAvailable() const {
    return m_initialized;
}

QWidget* MockCamera::getPreviewWidget() {
    return m_previewWidget;
}

void MockCamera::startPreview() {
    if (!m_initialized) {
        return;
    }
    
    qDebug() << "MockCamera: Starting preview";
    
    // Show live preview simulation
    m_previewWidget->setText("📷 Mock Camera - Live Preview\n\nReady to take photo!");
    m_previewWidget->setStyleSheet("background-color: #34495e; color: white; font-size: 18px; border: 2px solid #3498db;");
}

void MockCamera::stopPreview() {
    qDebug() << "MockCamera: Stopping preview";
    
    if (m_previewWidget) {
        m_previewWidget->setText("📷 Mock Camera Preview\n\nPreview stopped");
        m_previewWidget->setStyleSheet("background-color: #2c3e50; color: white; font-size: 18px;");
    }
}

void MockCamera::capturePhoto() {
    if (!m_initialized) {
        emit captureError("Mock camera not initialized");
        return;
    }
    
    qDebug() << "MockCamera: Starting photo capture simulation";
    
    // Show capturing state
    m_previewWidget->setText("📸 Capturing...");
    m_previewWidget->setStyleSheet("background-color: #e74c3c; color: white; font-size: 24px; font-weight: bold;");
    
    // Simulate capture delay
    m_captureTimer->start(1000); // 1 second delay
}

void MockCamera::simulatePhotoCapture() {
    qDebug() << "MockCamera: Simulating photo capture";
    
    // Create a test image with some content
    QPixmap testPhoto = createTestPhoto();
    
    // Generate filename
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString filename = QString("mock_photo_%1.png").arg(timestamp);
    QString fullPath = QDir(m_photosDirectory).absoluteFilePath(filename);
    
    // Save the test photo
    if (testPhoto.save(fullPath)) {
        qDebug() << "MockCamera: Photo saved to" << fullPath;
        emit photoReady(testPhoto, fullPath);
    } else {
        qWarning() << "MockCamera: Failed to save photo to" << fullPath;
        emit captureError("Failed to save mock photo");
    }
}

QPixmap MockCamera::createTestPhoto() {
    // Create a 800x600 test image
    QPixmap pixmap(800, 600);
    pixmap.fill(QColor(52, 73, 94)); // Dark blue-gray background
    
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // Draw a gradient background
    QLinearGradient gradient(0, 0, 800, 600);
    gradient.setColorAt(0, QColor(52, 152, 219)); // Light blue
    gradient.setColorAt(1, QColor(44, 62, 80));   // Dark blue
    painter.fillRect(pixmap.rect(), gradient);
    
    // Draw some decorative elements
    painter.setPen(QPen(QColor(255, 255, 255, 100), 2));
    painter.setBrush(QBrush(QColor(255, 255, 255, 50)));
    
    // Draw circles
    painter.drawEllipse(100, 100, 150, 150);
    painter.drawEllipse(550, 350, 200, 200);
    painter.drawEllipse(200, 400, 100, 100);
    
    // Draw text
    painter.setPen(QColor(255, 255, 255));
    painter.setFont(QFont("Arial", 36, QFont::Bold));
    painter.drawText(pixmap.rect(), Qt::AlignCenter, "📷 MOCK PHOTO\n\nPhoto Booth Test");
    
    // Add timestamp
    painter.setFont(QFont("Arial", 16));
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    painter.drawText(20, pixmap.height() - 20, timestamp);
    
    // Add a border
    painter.setPen(QPen(QColor(255, 255, 255), 4));
    painter.drawRect(pixmap.rect().adjusted(2, 2, -2, -2));
    
    return pixmap;
}

void MockCamera::cancelCapture() {
    
    qDebug() << "MockCamera: Cancelling capture";
    MockCamera::cleanup();
    
}

void MockCamera::setupPhotosDirectory() {
    m_photosDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/PhotoBooth";
    QDir dir;
    if (!dir.exists(m_photosDirectory)) {
        if (dir.mkpath(m_photosDirectory)) {
            qDebug() << "MockCamera: Created photos directory:" << m_photosDirectory;
        } else {
            qWarning() << "MockCamera: Failed to create photos directory:" << m_photosDirectory;
            m_photosDirectory = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
        }
    } else {
        qDebug() << "MockCamera: Photos directory:" << m_photosDirectory;
    }
}