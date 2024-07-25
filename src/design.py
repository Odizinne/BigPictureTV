# Form implementation generated from reading ui file '.\ui\design.ui'
#
# Created by: PyQt6 UI code generator 6.7.0
#
# WARNING: Any manual changes made to this file will be lost when pyuic6 is
# run again.  Do not edit this file unless you know what you are doing.


from PyQt6 import QtCore, QtGui, QtWidgets


class Ui_MainWindow(object):
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(495, 437)
        sizePolicy = QtWidgets.QSizePolicy(QtWidgets.QSizePolicy.Policy.Preferred, QtWidgets.QSizePolicy.Policy.Preferred)
        sizePolicy.setHorizontalStretch(0)
        sizePolicy.setVerticalStretch(0)
        sizePolicy.setHeightForWidth(MainWindow.sizePolicy().hasHeightForWidth())
        MainWindow.setSizePolicy(sizePolicy)
        self.centralwidget = QtWidgets.QWidget(parent=MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        self.gridLayout_2 = QtWidgets.QGridLayout(self.centralwidget)
        self.gridLayout_2.setObjectName("gridLayout_2")
        self.label_7 = QtWidgets.QLabel(parent=self.centralwidget)
        self.label_7.setMinimumSize(QtCore.QSize(0, 25))
        font = QtGui.QFont()
        font.setBold(True)
        self.label_7.setFont(font)
        self.label_7.setObjectName("label_7")
        self.gridLayout_2.addWidget(self.label_7, 8, 0, 1, 1)
        self.label_3 = QtWidgets.QLabel(parent=self.centralwidget)
        self.label_3.setMinimumSize(QtCore.QSize(0, 25))
        font = QtGui.QFont()
        font.setBold(True)
        self.label_3.setFont(font)
        self.label_3.setObjectName("label_3")
        self.gridLayout_2.addWidget(self.label_3, 2, 0, 1, 1)
        self.audioFrame = QtWidgets.QFrame(parent=self.centralwidget)
        self.audioFrame.setFrameShape(QtWidgets.QFrame.Shape.StyledPanel)
        self.audioFrame.setObjectName("audioFrame")
        self.gridLayout = QtWidgets.QGridLayout(self.audioFrame)
        self.gridLayout.setContentsMargins(9, 9, 9, 9)
        self.gridLayout.setSpacing(12)
        self.gridLayout.setObjectName("gridLayout")
        self.gamemodeLabel = QtWidgets.QLabel(parent=self.audioFrame)
        self.gamemodeLabel.setMinimumSize(QtCore.QSize(0, 25))
        self.gamemodeLabel.setObjectName("gamemodeLabel")
        self.gridLayout.addWidget(self.gamemodeLabel, 0, 0, 1, 1)
        self.desktopLabel = QtWidgets.QLabel(parent=self.audioFrame)
        self.desktopLabel.setMinimumSize(QtCore.QSize(0, 25))
        self.desktopLabel.setObjectName("desktopLabel")
        self.gridLayout.addWidget(self.desktopLabel, 1, 0, 1, 1)
        self.desktopEntry = QtWidgets.QLineEdit(parent=self.audioFrame)
        self.desktopEntry.setMinimumSize(QtCore.QSize(0, 25))
        self.desktopEntry.setAutoFillBackground(False)
        self.desktopEntry.setFrame(True)
        self.desktopEntry.setObjectName("desktopEntry")
        self.gridLayout.addWidget(self.desktopEntry, 1, 1, 1, 1)
        self.gamemodeEntry = QtWidgets.QLineEdit(parent=self.audioFrame)
        self.gamemodeEntry.setMinimumSize(QtCore.QSize(0, 25))
        self.gamemodeEntry.setAutoFillBackground(False)
        self.gamemodeEntry.setFrame(True)
        self.gamemodeEntry.setObjectName("gamemodeEntry")
        self.gridLayout.addWidget(self.gamemodeEntry, 0, 1, 1, 1)
        self.gridLayout_2.addWidget(self.audioFrame, 7, 0, 1, 1)
        self.label_2 = QtWidgets.QLabel(parent=self.centralwidget)
        self.label_2.setMinimumSize(QtCore.QSize(0, 25))
        font = QtGui.QFont()
        font.setBold(True)
        self.label_2.setFont(font)
        self.label_2.setObjectName("label_2")
        self.gridLayout_2.addWidget(self.label_2, 0, 0, 1, 1)
        self.videoFrame = QtWidgets.QFrame(parent=self.centralwidget)
        self.videoFrame.setFrameShape(QtWidgets.QFrame.Shape.StyledPanel)
        self.videoFrame.setFrameShadow(QtWidgets.QFrame.Shadow.Sunken)
        self.videoFrame.setObjectName("videoFrame")
        self.gridLayout_6 = QtWidgets.QGridLayout(self.videoFrame)
        self.gridLayout_6.setContentsMargins(9, 9, 9, 9)
        self.gridLayout_6.setHorizontalSpacing(12)
        self.gridLayout_6.setObjectName("gridLayout_6")
        self.desktopVideoLabel = QtWidgets.QLabel(parent=self.videoFrame)
        self.desktopVideoLabel.setMinimumSize(QtCore.QSize(0, 25))
        self.desktopVideoLabel.setObjectName("desktopVideoLabel")
        self.gridLayout_6.addWidget(self.desktopVideoLabel, 1, 0, 1, 1)
        self.desktopVideoBox = QtWidgets.QComboBox(parent=self.videoFrame)
        self.desktopVideoBox.setMinimumSize(QtCore.QSize(0, 25))
        self.desktopVideoBox.setObjectName("desktopVideoBox")
        self.gridLayout_6.addWidget(self.desktopVideoBox, 1, 1, 1, 1)
        self.gamemodeVideoLabel = QtWidgets.QLabel(parent=self.videoFrame)
        self.gamemodeVideoLabel.setMinimumSize(QtCore.QSize(0, 25))
        self.gamemodeVideoLabel.setObjectName("gamemodeVideoLabel")
        self.gridLayout_6.addWidget(self.gamemodeVideoLabel, 0, 0, 1, 1)
        self.gamemodeVideoBox = QtWidgets.QComboBox(parent=self.videoFrame)
        self.gamemodeVideoBox.setMinimumSize(QtCore.QSize(0, 25))
        self.gamemodeVideoBox.setObjectName("gamemodeVideoBox")
        self.gridLayout_6.addWidget(self.gamemodeVideoBox, 0, 1, 1, 1)
        self.gridLayout_2.addWidget(self.videoFrame, 9, 0, 1, 1)
        self.settingsFrame = QtWidgets.QFrame(parent=self.centralwidget)
        self.settingsFrame.setFrameShape(QtWidgets.QFrame.Shape.StyledPanel)
        self.settingsFrame.setFrameShadow(QtWidgets.QFrame.Shadow.Sunken)
        self.settingsFrame.setObjectName("settingsFrame")
        self.gridLayout_5 = QtWidgets.QGridLayout(self.settingsFrame)
        self.gridLayout_5.setContentsMargins(9, 9, 9, 9)
        self.gridLayout_5.setSpacing(12)
        self.gridLayout_5.setObjectName("gridLayout_5")
        self.checkRateSpinBox = QtWidgets.QSpinBox(parent=self.settingsFrame)
        self.checkRateSpinBox.setMinimumSize(QtCore.QSize(90, 25))
        self.checkRateSpinBox.setFrame(True)
        self.checkRateSpinBox.setMinimum(1)
        self.checkRateSpinBox.setMaximum(1000)
        self.checkRateSpinBox.setProperty("value", 1000)
        self.checkRateSpinBox.setObjectName("checkRateSpinBox")
        self.gridLayout_5.addWidget(self.checkRateSpinBox, 3, 2, 1, 1)
        self.displayswitchBox = QtWidgets.QCheckBox(parent=self.settingsFrame)
        self.displayswitchBox.setMinimumSize(QtCore.QSize(0, 25))
        self.displayswitchBox.setLayoutDirection(QtCore.Qt.LayoutDirection.RightToLeft)
        self.displayswitchBox.setText("")
        self.displayswitchBox.setChecked(True)
        self.displayswitchBox.setObjectName("displayswitchBox")
        self.gridLayout_5.addWidget(self.displayswitchBox, 2, 2, 1, 1)
        self.displayswitchLabel = QtWidgets.QLabel(parent=self.settingsFrame)
        self.displayswitchLabel.setMinimumSize(QtCore.QSize(0, 25))
        self.displayswitchLabel.setObjectName("displayswitchLabel")
        self.gridLayout_5.addWidget(self.displayswitchLabel, 2, 0, 1, 1)
        spacerItem = QtWidgets.QSpacerItem(0, 25, QtWidgets.QSizePolicy.Policy.Expanding, QtWidgets.QSizePolicy.Policy.Minimum)
        self.gridLayout_5.addItem(spacerItem, 3, 1, 1, 1)
        self.startupCheckBox = QtWidgets.QCheckBox(parent=self.settingsFrame)
        self.startupCheckBox.setMinimumSize(QtCore.QSize(0, 25))
        self.startupCheckBox.setLayoutDirection(QtCore.Qt.LayoutDirection.RightToLeft)
        self.startupCheckBox.setText("")
        self.startupCheckBox.setObjectName("startupCheckBox")
        self.gridLayout_5.addWidget(self.startupCheckBox, 0, 1, 1, 2)
        self.label_6 = QtWidgets.QLabel(parent=self.settingsFrame)
        self.label_6.setMinimumSize(QtCore.QSize(0, 25))
        self.label_6.setObjectName("label_6")
        self.gridLayout_5.addWidget(self.label_6, 1, 0, 1, 1)
        self.label_5 = QtWidgets.QLabel(parent=self.settingsFrame)
        self.label_5.setMinimumSize(QtCore.QSize(0, 25))
        self.label_5.setObjectName("label_5")
        self.gridLayout_5.addWidget(self.label_5, 0, 0, 1, 1)
        self.disableAudioCheckbox = QtWidgets.QCheckBox(parent=self.settingsFrame)
        self.disableAudioCheckbox.setMinimumSize(QtCore.QSize(0, 25))
        self.disableAudioCheckbox.setLayoutDirection(QtCore.Qt.LayoutDirection.RightToLeft)
        self.disableAudioCheckbox.setText("")
        self.disableAudioCheckbox.setObjectName("disableAudioCheckbox")
        self.gridLayout_5.addWidget(self.disableAudioCheckbox, 1, 1, 1, 2)
        self.label_4 = QtWidgets.QLabel(parent=self.settingsFrame)
        self.label_4.setMinimumSize(QtCore.QSize(0, 25))
        self.label_4.setObjectName("label_4")
        self.gridLayout_5.addWidget(self.label_4, 3, 0, 1, 1)
        self.gridLayout_2.addWidget(self.settingsFrame, 1, 0, 1, 1)
        MainWindow.setCentralWidget(self.centralwidget)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow"))
        self.label_7.setText(_translate("MainWindow", "Video output"))
        self.label_3.setText(_translate("MainWindow", "Audio output"))
        self.gamemodeLabel.setText(_translate("MainWindow", "Gamemode"))
        self.desktopLabel.setText(_translate("MainWindow", "Desktop"))
        self.label_2.setText(_translate("MainWindow", "Settings"))
        self.desktopVideoLabel.setText(_translate("MainWindow", "Desktop"))
        self.gamemodeVideoLabel.setText(_translate("MainWindow", "Gamemode"))
        self.displayswitchLabel.setToolTip(_translate("MainWindow", "Use windows built in displayswitch.exe to switch monitors.\n"
"\n"
"This should offer a smoother transition.\n"
"\n"
"You should use it if you only have 2 monitors.\n"
"\n"
"For 3 monitors or more, it is recommended to specify your outputs."))
        self.displayswitchLabel.setText(_translate("MainWindow", "Use old screen switch ⚠️"))
        self.label_6.setText(_translate("MainWindow", "Disable audio switching"))
        self.label_5.setText(_translate("MainWindow", "Run at startup"))
        self.label_4.setText(_translate("MainWindow", "Window check rate"))
