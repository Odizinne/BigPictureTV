#include "bigpicturetv.h"
#include <QCloseEvent>
#include <QDesktopServices>
#include <QJsonParseError>
#include <QMenuBar>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>
#include "aboutwindow.h"
#include "audiomanager.h"
#include "shortcutmanager.h"
#include "steamwindowmanager.h"
#include "ui_bigpicturetv.h"
#include "utils.h"
#include <iostream>

const QString BigPictureTV::settingsFile = QStandardPaths::writableLocation(
                                               QStandardPaths::AppDataLocation)
                                           + "/BigPictureTV/settings.json";

BigPictureTV::BigPictureTV(QWidget *parent)
    : QMainWindow(parent)
    , gamemodeActive(false)
    , firstRun(false)
    , ui(new Ui::BigPictureTV)
    , windowCheckTimer(new QTimer(this))
    , menubarVisible(false)
{
    ui->setupUi(this);
    setWindowIcon(getIconForTheme());
    setFont();
    createMenubar();
    populateComboboxes();
    loadSettings();
    setupConnections();
    getAudioCapabilities();
    windowCheckTimer->setInterval(ui->checkrateSpinBox->value());
    connect(windowCheckTimer, &QTimer::timeout, this, &BigPictureTV::checkWindowTitle);
    windowCheckTimer->start();
    createTrayIcon();
    if (firstRun) {
        this->show();
    }
}

BigPictureTV::~BigPictureTV()
{
    saveSettings();
    delete ui;
}

void BigPictureTV::setupConnections()
{
    connect(ui->startupCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::onStartupCheckboxStateChanged);
    connect(ui->desktopAudioLineEdit, &QLineEdit::textChanged, this, &BigPictureTV::saveSettings);
    connect(ui->gamemodeAudioLineEdit, &QLineEdit::textChanged, this, &BigPictureTV::saveSettings);
    connect(ui->disableAudioCheckBox, &QCheckBox::checkStateChanged, this, &BigPictureTV::onDisableAudioCheckboxStateChanged);
    connect(ui->disableMonitorCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::onDisableMonitorCheckboxStateChanged);
    connect(ui->checkrateSpinBox, &QSpinBox::valueChanged, this, &BigPictureTV::onCheckrateSpinBoxValueChanged);
    connect(ui->closeDiscordCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::saveSettings);
    connect(ui->startDiscordCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::saveSettings);
    connect(ui->gamemodePowerPlanComboBox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->desktopPowerPlanComboBox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->desktopMonitorComboBox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->gamemodeMonitorComboBox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->installAudioButton,  &QPushButton::clicked, this, &BigPictureTV::onAudioButtonClicked);
    ui->startupCheckBox->setChecked(isShortcutPresent());
    initDiscordAction();
}

void BigPictureTV::initDiscordAction()
{
    if (!isDiscordInstalled()) {
        ui->closeDiscordCheckBox->setChecked(false);
        ui->closeDiscordCheckBox->setEnabled(false);
        ui->CloseDiscordLabel->setEnabled(false);
        ui->startDiscordCheckBox->setChecked(false);
        ui->startDiscordCheckBox->setEnabled(false);
        ui->startDiscordLabel->setEnabled(false);
        ui->closeDiscordCheckBox->setToolTip(tr("Discord does not appear to be installed"));
        ui->CloseDiscordLabel->setToolTip(tr("Discord does not appear to be installed"));
        ui->startDiscordCheckBox->setToolTip(tr("Discord does not appear to be installed"));
        ui->startDiscordLabel->setToolTip(tr("Discord does not appear to be installed"));
    }
}
void BigPictureTV::getAudioCapabilities()
{
    if (!isAudioDeviceCmdletsInstalled()) {
        ui->disableAudioCheckBox->setChecked(true);
        ui->disableAudioCheckBox->setEnabled(false);
        toggleAudioSettings(false);
    } else {
        ui->disableAudioCheckBox->setEnabled(true);
        ui->installAudioButton->setVisible(false);
        if (!ui->disableAudioCheckBox->isChecked()) {
            toggleAudioSettings(true);
        }
        this->adjustSize();
    }
}

void BigPictureTV::populateComboboxes()
{
    ui->desktopMonitorComboBox->addItem(tr("Internal"));
    ui->desktopMonitorComboBox->addItem(tr("Extend"));
    ui->gamemodeMonitorComboBox->addItem(tr("External"));
    ui->gamemodeMonitorComboBox->addItem(tr("Clone"));
    ui->gamemodePowerPlanComboBox->addItem(tr("Disabled"));
    ui->gamemodePowerPlanComboBox->addItem(tr("Performance"));
    ui->gamemodePowerPlanComboBox->addItem(tr("Balanced"));
    ui->gamemodePowerPlanComboBox->addItem(tr("Energy saver"));
    ui->desktopPowerPlanComboBox->addItem(tr("Disabled"));
    ui->desktopPowerPlanComboBox->addItem(tr("Performance"));
    ui->desktopPowerPlanComboBox->addItem(tr("Balanced"));
    ui->desktopPowerPlanComboBox->addItem(tr("Energy saver"));
}

void BigPictureTV::createMenubar()
{
    menuBar = new QMenuBar(this);
    QMenu *fileMenu = new QMenu(tr("File"), this);

    QAction *resetSettingsAction = new QAction(tr("Reset Default Settings"), this);
    QAction *openSettingsFolderAction = new QAction(tr("Open Settings Folder"), this);
    QAction *exitAction = new QAction(tr("Exit"), this);

    connect(resetSettingsAction, &QAction::triggered, this, &BigPictureTV::createDefaultSettings);
    connect(openSettingsFolderAction, &QAction::triggered, this, []() {
        QString settingsFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDesktopServices::openUrl(QUrl::fromLocalFile(settingsFolder));
    });
    connect(exitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    fileMenu->addAction(resetSettingsAction);
    fileMenu->addAction(openSettingsFolderAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QMenu *helpMenu = new QMenu(tr("Help"), this);

    QAction *aboutAction = new QAction(tr("About"), this);

    connect(aboutAction, &QAction::triggered, this, &BigPictureTV::showAbout);

    helpMenu->addAction(aboutAction);
    menuBar->addMenu(fileMenu);
    menuBar->addMenu(helpMenu);
    menuBar->setVisible(false);
    setMenuBar(menuBar);
}

void BigPictureTV::showAbout()
{
    AboutWindow aboutWindow(this);
    aboutWindow.exec();
}

void BigPictureTV::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(getIconForTheme(), this);
    trayIcon->setToolTip("BigPictureTV");
    trayIcon->setContextMenu(createMenu());
    trayIcon->show();
}

QMenu *BigPictureTV::createMenu()
{
    QMenu *menu = new QMenu(this);

    QAction *settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, &QAction::triggered, this, &BigPictureTV::showSettings);

    QAction *exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    menu->addAction(settingsAction);
    menu->addAction(exitAction);

    return menu;
}

void BigPictureTV::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Alt) {
        toggleMenubarVisibility();
        event->accept();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}

void BigPictureTV::toggleMenubarVisibility()
{
    menubarVisible = !menubarVisible;
    if (menuBar) {
        menuBar->setVisible(menubarVisible);
    }
    this->adjustSize();
}

void BigPictureTV::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

void BigPictureTV::showSettings()
{
    this->show();
}

void BigPictureTV::onCheckrateSpinBoxValueChanged()
{
    int newInterval = ui->checkrateSpinBox->value();
    windowCheckTimer->setInterval(newInterval);
    saveSettings();
}

void BigPictureTV::onStartupCheckboxStateChanged()
{
    manageShortcut(ui->startupCheckBox->isChecked());
}

void BigPictureTV::onDisableAudioCheckboxStateChanged(int state)
{
    bool isChecked = (state == Qt::Checked);
    toggleAudioSettings(!isChecked);
    saveSettings();
}

void BigPictureTV::onDisableMonitorCheckboxStateChanged(int state)
{
    bool isChecked = (state == Qt::Checked);
    toggleMonitorSettings(!isChecked);
    saveSettings();
}

void BigPictureTV::onAudioButtonClicked()
{
    ui->installAudioButton->setEnabled(false);

    QProcess process;
    process.start(
        "powershell",
        QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command"
                      << "Install-PackageProvider -Name NuGet -Force -Scope CurrentUser; "
                         "Install-Module -Name AudioDeviceCmdlets -Force -Scope CurrentUser");

    if (!process.waitForFinished()) {
        QString status = tr("Error");
        QString message = tr("Failed to execute the PowerShell commands.\n"
                             "Please check if PowerShell is installed and properly configured.");
        QMessageBox::critical(this, status, message);
    } else {
        QString output = process.readAllStandardOutput();
        QString errorOutput = process.readAllStandardError();
        int exitCode = process.exitCode();

        QString status;
        QString message;

        if (exitCode == 0) {
            status = tr("Success");
            message = tr("AudioDeviceCmdlets module installed successfully.\nYou can now use audio "
                         "settings.");
            QMessageBox::information(this, status, message);
        } else {
            status = tr("Error");
            message = tr("Failed to install NuGet package provider or AudioDeviceCmdlets module.\n"
                         "Please install them manually by running these commands in PowerShell:\n"
                         "Install-PackageProvider -Name NuGet -Force -Scope CurrentUser;\n"
                         "Install-Module -Name AudioDeviceCmdlets -Force -Scope CurrentUser\n"
                         "You should then restart the application.\n"
                         "Error details:\n")
                      + errorOutput;
            QMessageBox::critical(this, status, message);
            ui->installAudioButton->setEnabled(true);
        }
    }

    getAudioCapabilities();
}

void BigPictureTV::checkWindowTitle()
{
    bool isRunning = isBigPictureRunning();
    bool disableVideo = ui->disableMonitorCheckBox->isChecked();
    bool disableAudio = ui->disableAudioCheckBox->isChecked();

    if (isRunning && !gamemodeActive && !isSunshineStreaming()) {
        gamemodeActive = true;
        handleMonitorChanges(false, disableVideo);
        handleAudioChanges(false, disableAudio);
        handleActions(false);
    } else if (!isRunning && gamemodeActive) {
        gamemodeActive = false;
        handleMonitorChanges(true, disableVideo);
        handleAudioChanges(true, disableAudio);
        handleActions(true);
    }
}

void BigPictureTV::handleMonitorChanges(bool isDesktopMode, bool disableVideo)
{
    if (disableVideo)
        return;

    int index = isDesktopMode ? ui->desktopMonitorComboBox->currentIndex()
                              : ui->gamemodeMonitorComboBox->currentIndex();

    const char *command = nullptr;

    if (index == 0) {
        command = isDesktopMode ? "/internal" : "/external";
    } else if (index == 1) {
        command = isDesktopMode ? "/extend" : "/clone";
    }

    if (command) {
        runDisplayswitch(command);
    }
}

void BigPictureTV::handleAudioChanges(bool isDesktopMode, bool disableAudio)
{
    if (disableAudio)
        return;

    std::string audioDevice = isDesktopMode ? ui->desktopAudioLineEdit->text().toStdString()
                                            : ui->gamemodeAudioLineEdit->text().toStdString();

    try {
        setAudioDevice(audioDevice);
    } catch (const std::runtime_error &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void BigPictureTV::handleActions(bool isDesktopMode)
{
    if (isDesktopMode) {
        if (ui->startDiscordCheckBox->isChecked()) {
            startDiscord();
        }
        if (ui->desktopPowerPlanComboBox->currentIndex() != 0) {
            switchPowerPlan(ui->desktopPowerPlanComboBox->currentIndex());
        }
    }
    if (!isDesktopMode) {
        if (ui->closeDiscordCheckBox->isChecked()) {
            closeDiscord();
        }
        if (ui->gamemodePowerPlanComboBox->currentIndex() != 0) {
            switchPowerPlan(ui->gamemodePowerPlanComboBox->currentIndex());
        }
    }
}

void BigPictureTV::createDefaultSettings()
{
    firstRun = true;
    ui->desktopAudioLineEdit->setText("Headset");
    ui->gamemodeAudioLineEdit->setText("TV");
    ui->desktopMonitorComboBox->setCurrentIndex(0);
    ui->gamemodeMonitorComboBox->setCurrentIndex(0);
    ui->closeDiscordCheckBox->setChecked(false);
    ui->startDiscordCheckBox->setChecked(false);
    ui->gamemodePowerPlanComboBox->setCurrentIndex(0);
    ui->desktopPowerPlanComboBox->setCurrentIndex(0);
    ui->startupCheckBox->setChecked(false);
    ui->disableAudioCheckBox->setChecked(false);
    ui->disableMonitorCheckBox->setChecked(false);
    ui->checkrateSpinBox->setValue(1000);
    saveSettings();
}

void BigPictureTV::loadSettings()
{
    QDir settingsDir(QFileInfo(settingsFile).absolutePath());
    if (!settingsDir.exists()) {
        settingsDir.mkpath(settingsDir.absolutePath());
    }

    QFile file(settingsFile);
    if (!file.exists()) {
        createDefaultSettings();

    } else {
        if (file.open(QIODevice::ReadOnly)) {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                settings = doc.object();
            }
            file.close();
        }
    }
    applySettings();
}

void BigPictureTV::applySettings()
{
    ui->gamemodeAudioLineEdit->setText(settings.value("gamemode_audio").toString());
    ui->desktopAudioLineEdit->setText(settings.value("desktop_audio").toString());
    ui->disableAudioCheckBox->setChecked(settings.value("disable_audio_switch").toBool());
    ui->checkrateSpinBox->setValue(settings.value("checkrate").toInt(1000));
    ui->closeDiscordCheckBox->setChecked(settings.value("close_discord_action").toBool(false));
    ui->startDiscordCheckBox->setChecked(settings.value("start_discord_action").toBool(false));
    ui->gamemodePowerPlanComboBox->setCurrentIndex(settings.value("gamemode_powerplan").toInt(0));
    ui->desktopPowerPlanComboBox->setCurrentIndex(settings.value("desktop_powerplan").toInt(0));
    ui->gamemodeMonitorComboBox->setCurrentIndex(settings.value("gamemode_monitor").toInt(0));
    ui->desktopMonitorComboBox->setCurrentIndex(settings.value("desktop_monitor").toInt(0));
    ui->disableMonitorCheckBox->setChecked(settings.value("disable_monitor_switch").toBool());
    toggleAudioSettings(!ui->disableAudioCheckBox->isChecked());
    toggleMonitorSettings(!ui->disableMonitorCheckBox->isChecked());
}

void BigPictureTV::saveSettings()
{
    settings["gamemode_audio"] = ui->gamemodeAudioLineEdit->text();
    settings["desktop_audio"] = ui->desktopAudioLineEdit->text();
    settings["disable_audio_switch"] = ui->disableAudioCheckBox->isChecked();
    settings["checkrate"] = ui->checkrateSpinBox->value();
    settings["close_discord_action"] = ui->closeDiscordCheckBox->isChecked();
    settings["start_discord_action"] = ui->startDiscordCheckBox->isChecked();
    settings["gamemode_powerplan"] = ui->gamemodePowerPlanComboBox->currentIndex();
    settings["desktop_powerplan"] = ui->desktopPowerPlanComboBox->currentIndex();
    settings["gamemode_monitor"] = ui->gamemodeMonitorComboBox->currentIndex();
    settings["desktop_monitor"] = ui->desktopMonitorComboBox->currentIndex();
    settings["disable_monitor_switch"] = ui->disableMonitorCheckBox->isChecked();

    QFile file(settingsFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(settings);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void BigPictureTV::toggleAudioSettings(bool state)
{
    ui->audioGroupBox->setEnabled(state);
}

void BigPictureTV::toggleMonitorSettings(bool state)
{
    ui->monitorsGroupBox->setEnabled(state);
}

void BigPictureTV::setFont()
{
    QList<QGroupBox*> groupBoxes = {
        ui->audioGroupBox,
        ui->monitorsGroupBox,
        ui->settingsGroupBox,
        ui->desktopActionsGroupBox,
        ui->gamemodeActionsGroupBox
    };

    for (QGroupBox* groupBox : groupBoxes) {
        groupBox->setStyleSheet("font-weight: bold;");

        const QList<QWidget*> children = groupBox->findChildren<QWidget*>();
        for (QWidget* child : children) {
            child->setStyleSheet("font-weight: normal;");
        }
    }
}
