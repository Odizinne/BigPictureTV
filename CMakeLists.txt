cmake_minimum_required(VERSION 3.21)

project(BigPictureTV VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Qml Quick)

qt_standard_project_setup()

set(PROJECT_SOURCES
    src/main.cpp
    src/appbridge.cpp
    src/appconfiguration.cpp
    src/windoweventmonitor.cpp
    src/audiomanager.cpp
    src/nightlightswitcher.cpp
    src/shortcutmanager.cpp
    src/steamwindowmanager.cpp
    src/utils.cpp
    extern/HDRtray/common/HDR.cpp
)

set(PROJECT_HEADERS
    include/appbridge.h
    include/appconfiguration.h
    include/windoweventmonitor.h
    include/audiomanager.h
    include/policyconfig.h
    include/nightlightswitcher.h
    include/shortcutmanager.h
    include/steamwindowmanager.h
    include/utils.h
    extern/HDRtray/common/HDR.h
)

set(QML_FILES
    qml/Main.qml
    qml/CustomComboBox.qml
    qml/Card.qml
)

set(QML_SINGLETONS
    qml/Singletons/Constants.qml
)

set_source_files_properties(${QML_SINGLETONS}
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)

qt_add_executable(BigPictureTV
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
    resources/resources.qrc
)

qt_add_qml_module(BigPictureTV
    URI Odizinne.BigPictureTV
    VERSION 1.0
    QML_FILES ${QML_FILES} ${QML_SINGLETONS}
)

target_include_directories(BigPictureTV PRIVATE
    include
    extern/HDRtray/common
)

target_link_libraries(BigPictureTV PRIVATE
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
)

if(WIN32)
    target_link_libraries(BigPictureTV PRIVATE
        ole32
        user32
        advapi32
        shell32
    )

    target_sources(BigPictureTV PRIVATE Resources/appicon.rc)
endif()

set_target_properties(BigPictureTV PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS BigPictureTV
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET BigPictureTV
    OUTPUT_SCRIPT deploy_script
    NO_TRANSLATIONS
    DEPLOY_TOOL_OPTIONS --no-compiler-runtime --no-opengl-sw --no-system-dxc-compiler --no-system-d3d-compiler --skip-plugin-types designer,iconengines,qmllint,generic,networkinformation,help,qmltooling,sqldrivers,qmlls
)
install(SCRIPT ${deploy_script})
