#include "bigpicturetv.h"
#include "colorutils.h"
#include "shortcutmanager.h"
#include "steamwindowmanager.h"
#include "ui_bigpicturetv.h"
#include "audiomanager.h"
#include "utils.h"
#include <QStandardPaths>
#include <QProcess>
#include <QJsonParseError>
#include <QCloseEvent>
#include <QMessageBox>
#include <iostream>

const QString BigPictureTV::settingsFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/BigPictureTV/settings.json";

BigPictureTV::BigPictureTV(QWidget *parent)
    : QMainWindow(parent)
    , gamemodeActive(false)
    , firstRun(false)
    , ui(new Ui::BigPictureTV)
    , windowCheckTimer(new QTimer(this))

{
    ui->setupUi(this);
    setWindowIcon(getIconForTheme());
    if (isWindows10())
    {
        setFrames();

    }
    populateComboboxes();
    loadSettings();
    setupConnections();
    getAudioCapabilities();


    windowCheckTimer->setInterval(ui->checkrateSpinBox->value());
    connect(windowCheckTimer, &QTimer::timeout, this, &BigPictureTV::checkWindowTitle);
    windowCheckTimer->start();
    createTrayIcon();
    if (firstRun)
    {
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
    connect(ui->gamemode_audio_lineedit, &QLineEdit::textChanged, this, &BigPictureTV::saveSettings);
    connect(ui->disableAudioCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::onDisableAudioCheckboxStateChanged);
    connect(ui->disableMonitorCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::onDisableMonitorCheckboxStateChanged);
    connect(ui->checkrateSpinBox, &QSpinBox::valueChanged, this, &BigPictureTV::onCheckrateSpinBoxValueChanged);
    connect(ui->closeDiscordCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::saveSettings);
    connect(ui->performancePowerPlanCheckBox, &QCheckBox::stateChanged, this, &BigPictureTV::saveSettings);
    connect(ui->desktopMonitorComboBox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->gamemodeMonitorComboBox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->installAudioButton, &QPushButton::clicked, this, &BigPictureTV::onAudioButtonClicked);
    ui->startupCheckBox->setChecked(isShortcutPresent());
    initDiscordAction();
}

void BigPictureTV::initDiscordAction()
{
    if (!isDiscordInstalled())
    {
        ui->closeDiscordCheckBox->setChecked(false);
        ui->closeDiscordCheckBox->setEnabled(false);
        ui->CloseDiscordLabel->setEnabled(false);
        ui->closeDiscordCheckBox->setToolTip("Discord does not appear to be installed");
        ui->CloseDiscordLabel->setToolTip("Discord does not appear to be installed");
    }
}
void BigPictureTV::getAudioCapabilities()
{
    if (!isAudioDeviceCmdletsInstalled())
    {
        ui->disableAudioCheckBox->setChecked(true);
        ui->disableAudioCheckBox->setEnabled(false);
        toggleAudioSettings(false);
    }
    else
    {
        ui->disableAudioCheckBox->setEnabled(true);
        ui->installAudioButton->setVisible(false);
        if (!ui->disableAudioCheckBox->isChecked())
        {
            toggleAudioSettings(true);
        }
        this->adjustSize();
    }
}

void BigPictureTV::setFrames()
{
    setFrameColorBasedOnWindow(this, ui->actionsFrame);
    setFrameColorBasedOnWindow(this, ui->audio_frame);
    setFrameColorBasedOnWindow(this, ui->monitorsFrame);
    setFrameColorBasedOnWindow(this, ui->settingsFrame);
    ui->checkrateSpinBox->setFrame(true);
}

void BigPictureTV::populateComboboxes()
{
    ui->desktopMonitorComboBox->addItem("Internal");
    ui->desktopMonitorComboBox->addItem("Extend");
    ui->gamemodeMonitorComboBox->addItem("External");
    ui->gamemodeMonitorComboBox->addItem("Clone");
}

void BigPictureTV::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(getIconForTheme(), this);
    trayIcon->setToolTip("BigPictureTV");
    trayIcon->setContextMenu(createMenu());
    trayIcon->show();
}

QMenu* BigPictureTV::createMenu()
{
    QMenu *menu = new QMenu(this);

    QAction *settingsAction = new QAction("Settings", this);
    connect(settingsAction, &QAction::triggered, this, &BigPictureTV::showSettings);

    QAction *exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, QApplication::instance(), &QApplication::quit);

    menu->addAction(settingsAction);
    menu->addAction(exitAction);

    return menu;
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
    process.start("powershell", QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command"
                                              << "Install-PackageProvider -Name NuGet -Force -Scope CurrentUser; "
                                                 "Install-Module -Name AudioDeviceCmdlets -Force -Scope CurrentUser");

    if (!process.waitForFinished()) {
        status = "Error";
        message = "Failed to execute the PowerShell commands.\n"
                  "Please check if PowerShell is installed and properly configured.";
    } else {
        QString output = process.readAllStandardOutput();
        QString errorOutput = process.readAllStandardError();
        int exitCode = process.exitCode();

        if (exitCode == 0) {
            status = "Success";
            message = "AudioDeviceCmdlets module installed successfully.\nYou can now use audio settings.";
        } else {
            status = "Error";
            message = "Failed to install NuGet package provider or AudioDeviceCmdlets module.\n"
                      "Please install them manually by running these commands in PowerShell:\n"
                      "Install-PackageProvider -Name NuGet -Force -Scope CurrentUser;\n"
                      "Install-Module -Name AudioDeviceCmdlets -Force -Scope CurrentUser\n"
                      "You should then restart the application.\n"
                      "Error details:\n" + errorOutput;
        }
    }

    if (status == "Success") {
        QMessageBox::information(this, status, message);
    } else {
        QMessageBox::critical(this, status, message);
        ui->installAudioButton->setEnabled(true);
    }
    getAudioCapabilities();
}

void BigPictureTV::checkWindowTitle()
{
    bool isRunning = isBigPictureRunning();
    bool disableVideo = ui->disableMonitorCheckBox->isChecked();
    bool disableAudio = ui->disableAudioCheckBox->isChecked();

    if (isRunning && !gamemodeActive)
    {
        gamemodeActive = true;
        handleMonitorChanges(false, disableVideo);
        handleAudioChanges(false, disableAudio);
        handleActions(false);
    }
    else if (!isRunning && gamemodeActive)
    {
        gamemodeActive = false;
        handleMonitorChanges(true, disableVideo);
        handleAudioChanges(true, disableAudio);
        handleActions(true);
    }
}

void BigPictureTV::handleMonitorChanges(bool isDesktopMode, bool disableVideo)
{
    if (disableVideo) return;

    int index = isDesktopMode ? ui->desktopMonitorComboBox->currentIndex() : ui->gamemodeMonitorComboBox->currentIndex();

    if (isDesktopMode)
    {
        if (index == 0)
        {
            runDisplayswitch("/internal");
        }
        else if (index == 1)
        {
            runDisplayswitch("/extend");
        }
    }
    else
    {
        if (index == 0)
        {
            runDisplayswitch("/external");
        }
        else if (index == 1)
        {
            runDisplayswitch("/clone");
        }
    }
}

void BigPictureTV::handleAudioChanges(bool isDesktopMode, bool disableAudio)
{
    if (disableAudio) return;

    std::string audioDevice = isDesktopMode ? ui->desktopAudioLineEdit->text().toStdString() : ui->gamemode_audio_lineedit->text().toStdString();

    try {
        setAudioDevice(audioDevice);
        std::cout << "Audio device set successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void BigPictureTV::handleActions(bool isDesktopMode)
{
    bool performDiscordAction = ui->closeDiscordCheckBox->isChecked();
    bool setPowerPlan = ui->performancePowerPlanCheckBox->isChecked();
    if (isDesktopMode)
    {
        if (performDiscordAction)
        {
            startDiscord();
        }
        if (setPowerPlan)
        {
            switchPowerPlan("381b4222-f694-41f0-9685-ff5bb260df2e");
        }
    }
    else
    {
        if (performDiscordAction)
        {
            closeDiscord();
        }
        if (setPowerPlan)
        {
            switchPowerPlan("8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
        }
    }

}

void BigPictureTV::createDefaultSettings()
{
    firstRun = true;
    ui->desktopAudioLineEdit->setText("Headset");
    ui->gamemode_audio_lineedit->setText("TV");
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
            } else {
                qWarning() << "Error parsing settings file:" << parseError.errorString();
            }
            file.close();
        } else {
            qWarning() << "Failed to open settings file for reading.";
        }
    }
    applySettings();
}

void BigPictureTV::applySettings()
{
    ui->gamemode_audio_lineedit->setText(settings.value("gamemode_audio").toString());
    ui->desktopAudioLineEdit->setText(settings.value("desktop_audio").toString());
    ui->disableAudioCheckBox->setChecked(settings.value("disable_audio_switch").toBool());
    ui->checkrateSpinBox->setValue(settings.value("checkrate").toInt(1000));
    ui->closeDiscordCheckBox->setChecked(settings.value("discord_action").toBool());
    ui->performancePowerPlanCheckBox->setChecked(settings.value("powerplan_action").toBool());
    ui->gamemodeMonitorComboBox->setCurrentIndex(settings.value("gamemode_monitor").toInt(0));
    ui->desktopMonitorComboBox->setCurrentIndex(settings.value("desktop_monitor").toInt(0));
    ui->disableMonitorCheckBox->setChecked(settings.value("disable_monitor_switch").toBool());
    toggleAudioSettings(!ui->disableAudioCheckBox->isChecked());
    toggleMonitorSettings(!ui->disableMonitorCheckBox->isChecked());
}

void BigPictureTV::saveSettings()
{
    settings["gamemode_audio"] = ui->gamemode_audio_lineedit->text();
    settings["desktop_audio"] = ui->desktopAudioLineEdit->text();
    settings["disable_audio_switch"] = ui->disableAudioCheckBox->isChecked();
    settings["checkrate"] = ui->checkrateSpinBox->value();
    settings["discord_action"] = ui->closeDiscordCheckBox->isChecked();
    settings["powerplan_action"] = ui->performancePowerPlanCheckBox->isChecked();
    settings["gamemode_monitor"] = ui->gamemodeMonitorComboBox->currentIndex();
    settings["desktop_monitor"] = ui->desktopMonitorComboBox->currentIndex();
    settings["disable_monitor_switch"] = ui->disableMonitorCheckBox->isChecked();

    QFile file(settingsFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(settings);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        qWarning() << "Failed to open settings file for writing.";
    }
}

void BigPictureTV::toggleAudioSettings(bool state)
{
    ui->audioOutputLabel->setEnabled(state);
    ui->gamemodeAudioLabel->setEnabled(state);
    ui->gamemode_audio_lineedit->setEnabled(state);
    ui->desktopAudioLabel->setEnabled(state);
    ui->desktopAudioLineEdit->setEnabled(state);
}

void BigPictureTV::toggleMonitorSettings(bool state)
{
    ui->monitorConfigurationLabel->setEnabled(state);
    ui->gamemodeMonitorLabel->setEnabled(state);
    ui->gamemodeMonitorComboBox->setEnabled(state);
    ui->desktopMonitorLabel->setEnabled(state);
    ui->desktopMonitorComboBox->setEnabled(state);
}
