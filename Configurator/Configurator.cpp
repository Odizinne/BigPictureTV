#include "Configurator.h"
#include "ui_Configurator.h"
#include "Utils.h"
#include "ShortcutManager.h"
#include "AudioManager.h"
#include <QDir>
#include <QDesktopServices>
#include <QMessageBox>
#include <QProcess>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <functional>

Configurator::Configurator(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Configurator)
    , settings("Odizinne", "BigPictureTV")
    , activeFrame(1)
    , devices(AudioManager::ListAudioOutputDevices())

{
    ui->setupUi(this);
    this->setWindowIcon(Utils::getIconForTheme());
    populateComboboxes();
    populateAudioComboBoxes();
    loadSettings();
    ui->avFrame->setVisible(false);
    ui->actionsFrame->setVisible(false);
    ui->advancedFrame->setVisible(false);
    Utils::setFrameColorBasedOnWindow(this, ui->frame);
    this->setFixedSize(382, 187);
    setupConnections();
}

Configurator::~Configurator()
{
    saveSettings();
    emit closed();
    delete ui;
}

void Configurator::setupConnections()
{
    connect(ui->startupCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onStartupCheckboxStateChanged);
    connect(ui->disableAudioCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onDisableAudioCheckboxStateChanged);
    connect(ui->disableMonitorCheckBox, &QCheckBox::checkStateChanged, this, &Configurator::onDisableMonitorCheckboxStateChanged);
    connect(ui->targetWindowComboBox, &QComboBox::currentIndexChanged, this, &Configurator::onTargetWindowComboBoxIndexChanged);
    connect(ui->resetSettingsButton, &QPushButton::clicked, this, &Configurator::createDefaultSettings);
    connect(ui->generalButton, &QPushButton::clicked, this, &Configurator::setGeneralTab);
    connect(ui->avButton, &QPushButton::clicked, this, &Configurator::setAVTab);
    connect(ui->actionsButton, &QPushButton::clicked, this, &Configurator::setActionsTab);
    connect(ui->advancedButton, &QPushButton::clicked, this, &Configurator::setAdvancedTab);

    ui->startupCheckBox->setChecked(ShortcutManager::isShortcutPresent("BigPictureTV.lnk"));
    initDiscordAction();
}

void Configurator::initDiscordAction()
{
    if (!Utils::isDiscordInstalled()) {
        ui->closeDiscordCheckBox->setChecked(false);
        ui->closeDiscordCheckBox->setEnabled(false);
        ui->CloseDiscordLabel->setEnabled(false);
        ui->closeDiscordCheckBox->setToolTip(tr("Discord does not appear to be installed"));
        ui->CloseDiscordLabel->setToolTip(tr("Discord does not appear to be installed"));
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
    ShortcutManager::manageShortcut(isChecked, "BigPictureTV.lnk");
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



void Configurator::createDefaultSettings()
{
    ui->checkrateSpinBox->setValue(1000);
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

    selectAudioDeviceFromSettings(ui->gamemodeAudioComboBox, settings.value("gamemode_audio_device").toString());
    selectAudioDeviceFromSettings(ui->desktopAudioComboBox, settings.value("desktop_audio_device").toString());
}

void Configurator::saveSettings()
{
    settings.setValue("window_checkrate", ui->checkrateSpinBox->value());
    settings.setValue("gamemode_audio_device", ui->gamemodeAudioComboBox->currentText());
    settings.setValue("desktop_audio_device", ui->desktopAudioComboBox->currentText());
    settings.setValue("gamemode_audio_device_id", getDeviceIDFromComboBox(ui->gamemodeAudioComboBox));
    settings.setValue("desktop_audio_device_id", getDeviceIDFromComboBox(ui->desktopAudioComboBox));
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
    ui->desktopAudioComboBox->setEnabled(state);
    ui->desktopAudioLabel->setEnabled(state);
    ui->gamemodeAudioComboBox->setEnabled(state);
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

void Configurator::fadeIn(QWidget *widget) {
    widget->setVisible(true);
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
    widget->setGraphicsEffect(effect);
    QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
    animation->setDuration(175);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void Configurator::fadeOut(QWidget *widget, std::function<void()> onFinished = nullptr) {
    if (widget->isVisible()) {
        QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
        QPropertyAnimation *animation = new QPropertyAnimation(effect, "opacity");
        animation->setDuration(175);
        animation->setStartValue(1.0);
        animation->setEndValue(0.0);

        connect(animation, &QPropertyAnimation::finished, widget, [widget]() {
            widget->setVisible(false);
        });

        if (onFinished) {
            connect(animation, &QPropertyAnimation::finished, onFinished);
        }

        animation->start(QAbstractAnimation::DeleteWhenStopped);
    } else {
        if (onFinished) {
            onFinished();
        }
    }
}

void Configurator::switchTab(int targetFrame, QPushButton* targetButton, QWidget* targetFrameWidget) {
    if (activeFrame == targetFrame) {
        targetButton->setChecked(true);
        return;
    }

    ui->generalButton->setChecked(false);
    ui->avButton->setChecked(false);
    ui->actionsButton->setChecked(false);
    ui->advancedButton->setChecked(false);
    targetButton->setChecked(true);

    QWidget* currentFrame = nullptr;

    switch (activeFrame) {
    case 1: currentFrame = ui->generalFrame; break;
    case 2: currentFrame = ui->avFrame; break;
    case 3: currentFrame = ui->actionsFrame; break;
    case 4: currentFrame = ui->advancedFrame; break;
    default: break;
    }

    if (currentFrame) {
        fadeOut(currentFrame, [this, targetFrameWidget]() {
            fadeIn(targetFrameWidget);
        });
    }

    activeFrame = targetFrame;
}

void Configurator::setGeneralTab() {
    switchTab(1, ui->generalButton, ui->generalFrame);
}

void Configurator::setAVTab() {
    switchTab(2, ui->avButton, ui->avFrame);
}

void Configurator::setActionsTab() {
    switchTab(3, ui->actionsButton, ui->actionsFrame);
}

void Configurator::setAdvancedTab() {
    switchTab(4, ui->advancedButton, ui->advancedFrame);
}

void Configurator::populateAudioComboBoxes()
{
    ui->gamemodeAudioComboBox->clear();
    ui->desktopAudioComboBox->clear();

    for (const Device &device : devices) {
        ui->gamemodeAudioComboBox->addItem(device.name);
        ui->desktopAudioComboBox->addItem(device.name);
    }
}

QString Configurator::getDeviceIDFromComboBox(QComboBox* comboBox)
{
    QString selectedDeviceName = comboBox->currentText();

    for (const Device &device : devices) {
        if (device.name == selectedDeviceName) {
            return device.ID;
        }
    }

    return "";
}

void Configurator::selectAudioDeviceFromSettings(QComboBox *comboBox, const QString &audioDeviceKey)
{
    int index = comboBox->findText(audioDeviceKey);

    if (index != -1) {
        comboBox->setCurrentIndex(index);
    }
}
