#include "configurator.h"
#include "ui_configurator.h"
#include "shortcutmanager.h"
#include <QDir>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProcess>

Configurator::Configurator(QWidget *parent)
    : QMainWindow(parent)
    , utils(new Utils())
    , steamWindowManager(new SteamWindowManager())
    , ui(new Ui::Configurator)
    , settings("Odizinne", "BigPictureTV")

{
    ui->setupUi(this);
    populateComboboxes();
    loadSettings();
    setGeneralTab();
    utils->setFrameColorBasedOnWindow(this, ui->frame);
    this->setFixedSize(356, 187);
    setupConnections();
    getAudioCapabilities();
}

Configurator::~Configurator()
{
    saveSettings();
    emit closed();
    delete utils;
    delete steamWindowManager;
    delete ui;
}

void Configurator::setupConnections()
{
    connect(ui->startupCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onStartupCheckboxStateChanged);
    connect(ui->disableAudioCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onDisableAudioCheckboxStateChanged);
    connect(ui->disableMonitorCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onDisableMonitorCheckboxStateChanged);
    connect(ui->installAudioButton,  &QPushButton::clicked, this, &Configurator::onAudioButtonClicked);
    connect(ui->targetWindowComboBox, &QComboBox::currentIndexChanged, this, &Configurator::onTargetWindowComboBoxIndexChanged);
    connect(ui->resetSettingsButton, &QPushButton::clicked, this, &Configurator::createDefaultSettings);
    connect(ui->generalButton, &QPushButton::clicked, this, &Configurator::setGeneralTab);
    connect(ui->avButton, &QPushButton::clicked, this, &Configurator::setAVTab);
    connect(ui->actionsButton, &QPushButton::clicked, this, &Configurator::setActionsTab);
    connect(ui->advancedButton, &QPushButton::clicked, this, &Configurator::setAdvancedTab);

    ui->startupCheckBox->setChecked(ShortcutManager::isShortcutPresent());
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

void Configurator::onStartupCheckboxStateChanged(Qt::CheckState state)
{
    bool isChecked = (state == Qt::Checked);
    ShortcutManager::manageShortcut(isChecked);
}

void Configurator::onDisableAudioCheckboxStateChanged(Qt::CheckState state)
{
    bool isChecked = (state == Qt::Checked);
    toggleAudioSettings(!isChecked);
}

void Configurator::onDisableMonitorCheckboxStateChanged(Qt::CheckState state)
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
    ui->checkrateSpinBox->setValue(1000);
    ui->desktopAudioLineEdit->setText("Headset");
    ui->gamemodeAudioLineEdit->setText("TV");
    ui->desktopMonitorComboBox->setCurrentIndex(0);
    ui->gamemodeMonitorComboBox->setCurrentIndex(0);
    ui->closeDiscordCheckBox->setChecked(false);
    ui->enablePerformancePowerPlan->setChecked(false);
    ui->disableNightLightCheckBox->setChecked(false);
    ui->pauseMediaAction->setChecked(false);
    ui->disableAudioCheckBox->setChecked(false);
    ui->disableMonitorCheckBox->setChecked(false);
    ui->customWindowLineEdit->setText("");
    ui->targetWindowComboBox->setCurrentIndex(0);

    saveSettings();
}

void Configurator::loadSettings()
{
    ui->checkrateSpinBox->setValue(settings.value("window_checkrate", 1000).toInt());
    ui->desktopAudioLineEdit->setText(settings.value("desktop_audio_device", "Headset").toString());
    ui->gamemodeAudioLineEdit->setText(settings.value("gamemode_audio_device", "TV").toString());
    ui->desktopMonitorComboBox->setCurrentIndex(settings.value("desktop_monitor_mode", 0).toInt());
    ui->gamemodeMonitorComboBox->setCurrentIndex(settings.value("gamemode_monitor_mode", 0).toInt());
    ui->closeDiscordCheckBox->setChecked(settings.value("close_discord_action", false).toBool());
    ui->enablePerformancePowerPlan->setChecked(settings.value("performance_powerplan_action", false).toBool());
    ui->disableNightLightCheckBox->setChecked(settings.value("disable_nightlight_action", false).toBool());
    ui->pauseMediaAction->setChecked(settings.value("pause_media_action", false).toBool());
    ui->disableAudioCheckBox->setChecked(settings.value("disable_audio_switch", false).toBool());
    ui->disableMonitorCheckBox->setChecked(settings.value("disable_monitor_switch", false).toBool());
    ui->customWindowLineEdit->setText(settings.value("custom_window_title", "").toString());
    ui->targetWindowComboBox->setCurrentIndex(settings.value("target_window_mode", 0).toInt());

    toggleAudioSettings(!ui->disableAudioCheckBox->isChecked());
    toggleMonitorSettings(!ui->disableMonitorCheckBox->isChecked());
    toggleCustomWindowTitle(ui->targetWindowComboBox->currentIndex() == 1);
}

void Configurator::saveSettings()
{
    settings.setValue("window_checkrate", ui->checkrateSpinBox->value());
    settings.setValue("gamemode_audio_device", ui->gamemodeAudioLineEdit->text());
    settings.setValue("desktop_audio_device", ui->desktopAudioLineEdit->text());
    settings.setValue("gamemode_monitor_mode", ui->gamemodeMonitorComboBox->currentIndex());
    settings.setValue("desktop_monitor_mode", ui->desktopMonitorComboBox->currentIndex());
    settings.setValue("disable_audio_switch", ui->disableAudioCheckBox->isChecked());
    settings.setValue("disable_monitor_switch", ui->disableMonitorCheckBox->isChecked());
    settings.setValue("close_discord_action", ui->closeDiscordCheckBox->isChecked());
    settings.setValue("performance_powerplan_action", ui->enablePerformancePowerPlan->isChecked());
    settings.setValue("pause_media_action", ui->pauseMediaAction->isChecked());
    settings.setValue("disable_nightlight_action", ui->disableNightLightCheckBox->isChecked());
    settings.setValue("target_window_mode", ui->targetWindowComboBox->currentIndex());
    settings.setValue("custom_window_title", ui->customWindowLineEdit->text());
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


void Configurator::setGeneralTab()
{
    ui->generalFrame->setVisible(true);
    ui->avFrame->setVisible(false);
    ui->actionsFrame->setVisible(false);
    ui->advancedFrame->setVisible(false);
}

void Configurator::setAVTab()
{
    ui->generalFrame->setVisible(false);
    ui->avFrame->setVisible(true);
    ui->actionsFrame->setVisible(false);
    ui->advancedFrame->setVisible(false);
}

void Configurator::setActionsTab()
{
    ui->generalFrame->setVisible(false);
    ui->avFrame->setVisible(false);
    ui->actionsFrame->setVisible(true);
    ui->advancedFrame->setVisible(false);
}

void Configurator::setAdvancedTab()
{
    ui->generalFrame->setVisible(false);
    ui->avFrame->setVisible(false);
    ui->actionsFrame->setVisible(false);
    ui->advancedFrame->setVisible(true);
}
