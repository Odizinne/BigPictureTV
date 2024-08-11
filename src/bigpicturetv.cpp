#include "bigpicturetv.h"
#include "colorutils.h"
#include "shortcutmanager.h"
#include "steamwindowmanager.h"
#include "ui_bigpicturetv.h"
#include "audiomanager.h"
#include "utils.h"

#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QIcon>
#include <QProcess>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QCloseEvent>
#include <QMessageBox>
#include <QDebug>
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
    setFrames();
    populateComboboxes();
    loadSettings();
    setupConnections();
    getAudioCapabilities();


    windowCheckTimer->setInterval(ui->checkrate_slider->value());
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
    connect(ui->startup_checkbox, &QCheckBox::stateChanged, this, &BigPictureTV::onStartupCheckboxStateChanged);
    connect(ui->desktop_audio_lineedit, &QLineEdit::textChanged, this, &BigPictureTV::saveSettings);
    connect(ui->gamemode_audio_lineedit, &QLineEdit::textChanged, this, &BigPictureTV::saveSettings);
    connect(ui->disable_audio_checkbox, &QCheckBox::stateChanged, this, &BigPictureTV::onDisableAudioCheckboxStateChanged);
    connect(ui->disable_monitor_checkbox, &QCheckBox::stateChanged, this, &BigPictureTV::onDisableMonitorCheckboxStateChanged);
    connect(ui->checkrate_slider, &QSlider::sliderReleased, this, &BigPictureTV::onCheckrateSliderReleased);
    ui->close_discord_checkbox->setEnabled(isDiscordInstalled());
    ui->close_discord_label->setEnabled(isDiscordInstalled());
    connect(ui->close_discord_checkbox, &QCheckBox::stateChanged, this, &BigPictureTV::saveSettings);
    connect(ui->performance_powerplan_checkbox, &QCheckBox::stateChanged, this, &BigPictureTV::saveSettings);
    connect(ui->desktop_monitor_combobox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->gamemode_monitor_combobox, &QComboBox::currentIndexChanged, this, &BigPictureTV::saveSettings);
    connect(ui->install_audio_button, &QPushButton::clicked, this, &BigPictureTV::onAudioButtonClicked);
}

void BigPictureTV::getAudioCapabilities()
{
    if (!isAudioDeviceCmdletsInstalled())
    {
        qDebug() << "here:";
        ui->disable_audio_checkbox->setChecked(true);
        ui->disable_audio_checkbox->setEnabled(false);
        toggleAudioSettings(false);
    }
    else
    {
        qDebug() << "there:";
        ui->disable_audio_checkbox->setEnabled(true);
        ui->install_audio_button->setVisible(false);
        if (!ui->disable_audio_checkbox->isChecked())
        {
            toggleAudioSettings(true);
        }
        this->adjustSize();
    }
}

void BigPictureTV::setFrames()
{
    setFrameColorBasedOnWindow(this, ui->actions_frame);
    setFrameColorBasedOnWindow(this, ui->audio_frame);
    setFrameColorBasedOnWindow(this, ui->monitor_frame);
    setFrameColorBasedOnWindow(this, ui->settings_frame);
}

void BigPictureTV::populateComboboxes()
{
    ui->desktop_monitor_combobox->addItem("Internal");
    ui->desktop_monitor_combobox->addItem("Extend");
    ui->gamemode_monitor_combobox->addItem("External");
    ui->gamemode_monitor_combobox->addItem("Clone");
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

void BigPictureTV::onCheckrateSliderReleased()
{
    int newInterval = ui->checkrate_slider->value();
    windowCheckTimer->setInterval(newInterval);
    saveSettings();
    qDebug() << "Checkrate updated to:" << newInterval << "ms";
}


void BigPictureTV::onStartupCheckboxStateChanged()
{
    manageShortcut(shortcutName, ui->startup_checkbox->isChecked());
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
    ui->install_audio_button->setEnabled(false);

    QProcess process;
    process.start("powershell", QStringList() << "-NoProfile" << "-ExecutionPolicy" << "Bypass" << "-Command"
                                              << "Install-Module AudioDeviceCmdlets -Force -Scope CurrentUser");

    if (!process.waitForFinished()) {
        status = "Error";
        message = "Failed to execute the PowerShell command.\n"
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
            message = "Failed to install AudioDeviceCmdlets module.\n"
                      "Please install it manually by running this command in PowerShell: "
                      "Install-Module AudioDeviceCmdlets -Force -Scope CurrentUser\n"
                      "You should then restart the application.\n"
                      "Error details:\n" + errorOutput;
        }
    }

    if (status == "Success") {
        QMessageBox::information(this, status, message);
    } else {
        QMessageBox::critical(this, status, message);
        ui->install_audio_button->setEnabled(true);
    }
    getAudioCapabilities();
}


void BigPictureTV::checkWindowTitle()
{
    //qDebug() << "checking";
    bool isRunning = isBigPictureRunning();
    bool disableVideo = ui->disable_monitor_checkbox->isChecked();
    bool disableAudio = ui->disable_audio_checkbox->isChecked();

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

    int index = isDesktopMode ? ui->desktop_monitor_combobox->currentIndex() : ui->gamemode_monitor_combobox->currentIndex();

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

    std::string audioDevice = isDesktopMode ? ui->desktop_audio_lineedit->text().toStdString() : ui->gamemode_audio_lineedit->text().toStdString();

    try {
        setAudioDevice(audioDevice);
        std::cout << "Audio device set successfully." << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

void BigPictureTV::handleActions(bool isDesktopMode)
{
    bool performDiscordAction = ui->close_discord_checkbox->isChecked();
    bool setPowerPlan = ui->performance_powerplan_checkbox->isChecked();
    if (isDesktopMode)
    {
        if (performDiscordAction)
        {
            startDiscord();
        }
        if (setPowerPlan)
        {
            switchPowerPlan(L"381b4222-f694-41f0-9685-ff5bb260df2e");
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
            switchPowerPlan(L"8c5e7fda-e8bf-4a96-9a85-a6e23a8c635c");
        }
    }

}

void BigPictureTV::createDefaultSettings()
{
    firstRun = true;
    ui->desktop_audio_lineedit->setText("Headset");
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
    qDebug() << "applying settings.";
    ui->gamemode_audio_lineedit->setText(settings.value("gamemode_audio").toString());
    ui->desktop_audio_lineedit->setText(settings.value("desktop_audio").toString());
    ui->disable_audio_checkbox->setChecked(settings.value("disable_audio_switch").toBool());
    ui->checkrate_slider->setValue(settings.value("checkrate").toInt(1000));
    ui->close_discord_checkbox->setChecked(settings.value("discord_action").toBool());
    ui->performance_powerplan_checkbox->setChecked(settings.value("powerplan_action").toBool());
    ui->gamemode_monitor_combobox->setCurrentIndex(settings.value("gamemode_monitor").toInt(0));
    ui->desktop_monitor_combobox->setCurrentIndex(settings.value("desktop_monitor").toInt(0));
    ui->disable_monitor_checkbox->setChecked(settings.value("disable_monitor_switch").toBool());

    // Call toggle functions based on settings
    toggleAudioSettings(!ui->disable_audio_checkbox->isChecked());
    toggleMonitorSettings(!ui->disable_monitor_checkbox->isChecked());
}

void BigPictureTV::saveSettings()
{
    qDebug() << "saving settings.";

    settings["gamemode_audio"] = ui->gamemode_audio_lineedit->text();
    settings["desktop_audio"] = ui->desktop_audio_lineedit->text();
    settings["disable_audio_switch"] = ui->disable_audio_checkbox->isChecked();
    settings["checkrate"] = ui->checkrate_slider->value();
    settings["discord_action"] = ui->close_discord_checkbox->isChecked();
    settings["powerplan_action"] = ui->performance_powerplan_checkbox->isChecked();
    settings["gamemode_monitor"] = ui->gamemode_monitor_combobox->currentIndex();
    settings["desktop_monitor"] = ui->desktop_monitor_combobox->currentIndex();
    settings["disable_monitor_switch"] = ui->disable_monitor_checkbox->isChecked();

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
    ui->audio_output_label->setEnabled(state);
    ui->gamemode_audio_label->setEnabled(state);
    ui->gamemode_audio_lineedit->setEnabled(state);
    ui->desktop_audio_label->setEnabled(state);
    ui->desktop_audio_lineedit->setEnabled(state);
}

void BigPictureTV::toggleMonitorSettings(bool state)
{
    ui->monitor_configuration_label->setEnabled(state);
    ui->gamemode_monitor_label->setEnabled(state);
    ui->gamemode_monitor_combobox->setEnabled(state);
    ui->desktop_monitor_label->setEnabled(state);
    ui->desktop_monitor_combobox->setEnabled(state);
}
