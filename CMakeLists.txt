cmake_minimum_required (VERSION 3.16)
project(breeze-lim)
set(PROJECT_VERSION "5.22.80")
set(PROJECT_VERSION_MAJOR 5)
set(KF5_MIN_VERSION "5.82")
set(QT_MIN_VERSION "5.12.0")

add_definitions (-Wall )# -Werror)

include(GenerateExportHeader)
include(WriteBasicConfigVersionFile)
include (FeatureSummary)

find_package (ECM 0.0.9 REQUIRED NO_MODULE)

set (CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    ${ECM_MODULE_PATH}
)

include (ECMInstallIcons)
include (KDEInstallDirs)
include (KDECMakeSettings)
include (KDECompilerSettings NO_POLICY_SCOPE)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets
    DBus
)


include(ECMQtDeclareLoggingCategory)
#add_subdirectory (src/libdbusmenuqt)

add_subdirectory (src)

feature_summary(WHAT ALL)
