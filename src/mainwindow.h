#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <map>
#include <QString>
#include <QPixmap>
#include <qwidget.h>

class QStackedWidget;
class QPushButton;
class QLabel;
class QLineEdit;
class QVBoxLayout;
class QHBoxLayout;
class QPixmap;

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

private:
    void setupUi();
    QWidget* createStartScreen();
    QWidget* createWeaponChoiceScreen();
    QWidget* createLandChoiceScreen();
    QWidget* createCompanionChoiceScreen();
    QWidget* createNameEntryScreen();

    QWidget* createChoiceScreen(
                                const QString& title, 
                                const QString& categoryPrefix, 
                                int itemCount, 
                                const char* slot);
    

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

    // Pointers to screen widgets for QStackedWidget
    QWidget *m_startScreenWidget;
    QWidget *m_weaponChoiceScreenWidget;
    QWidget *m_landChoiceScreenWidget;
    QWidget *m_companionChoiceScreenWidget;
    QWidget *m_nameEntryScreenWidget;

    // Persistent Data (Loaded once)
    std::map<QString, QPixmap> m_selectableImages;
    // Per-Iteration Data
    std::unique_ptr<PhotoSessionData> m_currentSessionData;
};

#endif // MAINWINDOW_H
