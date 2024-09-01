#include "configurator.h"
#include "qdir.h"
#include "ui_configurator.h"
#include <QDesktopServices>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QStandardPaths>

const QString Configurator::settingsFile = QStandardPaths::writableLocation(
                                               QStandardPaths::AppDataLocation)
                                           + "/BigPictureTV/settings.json";

Configurator::Configurator(QWidget *parent)
    : QMainWindow(parent)
    , utils(new Utils())
    , shortcutManager(new ShortcutManager())
    , steamWindowManager(new SteamWindowManager())
    , ui(new Ui::Configurator)
{
    ui->setupUi(this);
    setupInfoTab();
    populateComboboxes();
    loadSettings();
    setupConnections();
    getAudioCapabilities();
}

Configurator::~Configurator()
{
    saveSettings();
    emit closed();
    delete shortcutManager;
    delete utils;
    delete steamWindowManager;
    delete ui;
}

void Configurator::setupConnections()
{
    connect(ui->startupCheckBox, &QCheckBox::stateChanged, this, &Configurator::onStartupCheckboxStateChanged);
    connect(ui->disableAudioCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onDisableAudioCheckboxStateChanged);
    connect(ui->disableMonitorCheckBox, &QCheckBox::stateChanged, this, &Configurator::onDisableMonitorCheckboxStateChanged);
    connect(ui->installAudioButton,  &QPushButton::clicked, this, &Configurator::onAudioButtonClicked);
    connect(ui->targetWindowComboBox, &QComboBox::currentIndexChanged, this, &Configurator::onTargetWindowComboBoxIndexChanged);
    connect(ui->resetSettingsButton, &QPushButton::clicked, this, &Configurator::createDefaultSettings);
    connect(ui->openSettingsButton, &QPushButton::clicked, this, []() {
        QString settingsFolder = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDesktopServices::openUrl(QUrl::fromLocalFile(settingsFolder));
    });

    ui->startupCheckBox->setChecked(shortcutManager->isShortcutPresent());
    initDiscordAction();
}

void Configurator::initDiscordAction()
{
    if (!utils->isDiscordInstalled()) {
        ui->closeDiscordCheckBox->setChecked(false);
        ui->closeDiscordCheckBox->setEnabled(false);
        ui->CloseDiscordLabel->setEnabled(false);
        ui->closeDiscordCheckBox->setToolTip(tr("Discord does not appear to be installed"));
        ui->CloseDiscordLabel->setToolTip(tr("Discord does not appear to be installed"));
    }
}

void Configurator::getAudioCapabilities()
{
    if (!utils->isAudioDeviceCmdletsInstalled()) {
        ui->disableAudioCheckBox->setChecked(true);
        ui->disableAudioCheckBox->setEnabled(false);
        toggleAudioSettings(false);
    } else {
        ui->disableAudioCheckBox->setEnabled(true);
        ui->installAudioButton->setEnabled(false);
        ui->installAudioButton->setText(tr("Audio module installed"));
        if (!ui->disableAudioCheckBox->isChecked()) {
            toggleAudioSettings(true);
        }
    }
}

void Configurator::populateComboboxes()
{
    ui->desktopMonitorComboBox->addItem(tr("Internal"));
    ui->desktopMonitorComboBox->addItem(tr("Extend"));
    ui->gamemodeMonitorComboBox->addItem(tr("External"));
    ui->gamemodeMonitorComboBox->addItem(tr("Clone"));

    ui->targetWindowComboBox->addItem(tr("Big Picture"));
    ui->targetWindowComboBox->addItem(tr("Custom"));
}

void Configurator::onStartupCheckboxStateChanged()
{
    shortcutManager->manageShortcut(ui->startupCheckBox->isChecked());
}

void Configurator::onDisableAudioCheckboxStateChanged(int state)
{
    bool isChecked = (state == Qt::Checked);
    toggleAudioSettings(!isChecked);
}

void Configurator::onDisableMonitorCheckboxStateChanged(int state)
{
    bool isChecked = (state == Qt::Checked);
    toggleMonitorSettings(!isChecked);
}

void Configurator::onTargetWindowComboBoxIndexChanged(int index)
{
    if (index == 1) {
        toggleCustomWindowTitle(true);
    } else {
        toggleCustomWindowTitle(false);
    }
}

void Configurator::onAudioButtonClicked()
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

void Configurator::createDefaultSettings()
{
    ui->desktopAudioLineEdit->setText("Headset");
    ui->gamemodeAudioLineEdit->setText("TV");
    ui->desktopMonitorComboBox->setCurrentIndex(0);
    ui->gamemodeMonitorComboBox->setCurrentIndex(0);
    ui->closeDiscordCheckBox->setChecked(false);
    ui->enablePerformancePowerPlan->setChecked(false);
    ui->startupCheckBox->setChecked(false);
    ui->disableAudioCheckBox->setChecked(false);
    ui->disableMonitorCheckBox->setChecked(false);
    ui->checkrateSpinBox->setValue(1000);
    ui->disableNightLightCheckBox->setChecked(false);
    saveSettings();
}

void Configurator::loadSettings()
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

void Configurator::applySettings()
{
    ui->gamemodeAudioLineEdit->setText(settings.value("gamemode_audio").toString());
    ui->desktopAudioLineEdit->setText(settings.value("desktop_audio").toString());
    ui->disableAudioCheckBox->setChecked(settings.value("disable_audio_switch").toBool());
    ui->checkrateSpinBox->setValue(settings.value("checkrate").toInt(1000));
    ui->closeDiscordCheckBox->setChecked(settings.value("close_discord_action").toBool(false));
    ui->enablePerformancePowerPlan->setChecked(settings.value("gamemode_powerplan").toBool(false));
    ui->pauseMediaAction->setChecked(settings.value("gamemode_pause_media").toBool(false));
    ui->gamemodeMonitorComboBox->setCurrentIndex(settings.value("gamemode_monitor").toInt(0));
    ui->desktopMonitorComboBox->setCurrentIndex(settings.value("desktop_monitor").toInt(0));
    ui->disableMonitorCheckBox->setChecked(settings.value("disable_monitor_switch").toBool());
    ui->disableNightLightCheckBox->setChecked(settings.value("disable_nightlight").toBool());
    ui->targetWindowComboBox->setCurrentIndex(settings.value("target_window").toInt(0));
    ui->customWindowLineEdit->setText(settings.value("custom_window").toString());
    toggleAudioSettings(!ui->disableAudioCheckBox->isChecked());
    toggleMonitorSettings(!ui->disableMonitorCheckBox->isChecked());
    toggleCustomWindowTitle(ui->targetWindowComboBox->currentIndex() == 1);
}

void Configurator::saveSettings()
{
    settings["gamemode_audio"] = ui->gamemodeAudioLineEdit->text();
    settings["desktop_audio"] = ui->desktopAudioLineEdit->text();
    settings["disable_audio_switch"] = ui->disableAudioCheckBox->isChecked();
    settings["checkrate"] = ui->checkrateSpinBox->value();
    settings["close_discord_action"] = ui->closeDiscordCheckBox->isChecked();
    settings["gamemode_powerplan"] = ui->enablePerformancePowerPlan->isChecked();
    settings["gamemode_pause_media"] = ui->pauseMediaAction->isChecked();
    settings["gamemode_monitor"] = ui->gamemodeMonitorComboBox->currentIndex();
    settings["desktop_monitor"] = ui->desktopMonitorComboBox->currentIndex();
    settings["disable_monitor_switch"] = ui->disableMonitorCheckBox->isChecked();
    settings["disable_nightlight"] = ui->disableNightLightCheckBox->isChecked();
    settings["target_window"] = ui->targetWindowComboBox->currentIndex();
    settings["custom_window"] = ui->customWindowLineEdit->text();

    QFile file(settingsFile);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(settings);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}

void Configurator::toggleAudioSettings(bool state)
{
    ui->desktopAudioLineEdit->setEnabled(state);
    ui->desktopAudioLabel->setEnabled(state);
    ui->gamemodeAudioLineEdit->setEnabled(state);
    ui->gamemodeAudioLabel->setEnabled(state);
}

void Configurator::toggleMonitorSettings(bool state)
{
    ui->desktopMonitorComboBox->setEnabled(state);
    ui->desktopMonitorLabel->setEnabled(state);
    ui->gamemodeMonitorComboBox->setEnabled(state);
    ui->gamemodeMonitorLabel->setEnabled(state);
}

void Configurator::toggleCustomWindowTitle(bool state)
{
    ui->customWindowLineEdit->setEnabled(state);
    ui->customWindowLabel->setEnabled(state);
}

void Configurator::setupInfoTab()
{
    ui->detectedSteamLanguage->setText(steamWindowManager->getSteamLanguage());

    ui->targetWindowTitle->setText(steamWindowManager->getBigPictureWindowTitle());
    ui->repository->setText("<a href=\"https://github.com/odizinne/bigpicturetv/\">github.com/Odizinne/BigPictureTV</a>");
    ui->repository->setTextFormat(Qt::RichText);
    ui->repository->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->repository->setOpenExternalLinks(true);
    ui->commitID->setText(GIT_COMMIT_ID);
    ui->commitDate->setText(GIT_COMMIT_DATE);
}
