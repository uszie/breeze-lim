add_definitions(-DTRANSLATION_DOMAIN="breeze_kwin_deco")

find_package(KDecoration2 REQUIRED)
#find_package(KF5 REQUIRED COMPONENTS CoreAddons GuiAddons ConfigWidgets WindowSystem I18n IconThemes)
#find_package(KWaylandServer REQUIRED)
find_package(KF5 REQUIRED COMPONENTS CoreAddons GuiAddons ConfigWidgets WindowSystem I18n IconThemes Wayland)
find_package(Qt5 CONFIG REQUIRED COMPONENTS DBus)
#find_package(dbusmenu-qt5 REQUIRED)
find_package(LibTaskManager)

### XCB
find_package(XCB COMPONENTS XCB)
set_package_properties(XCB PROPERTIES
  DESCRIPTION "X protocol C-language Binding"
  URL "https://xcb.freedesktop.org"
  TYPE OPTIONAL
  PURPOSE "Required to pass style properties to native Windows on X11 Platform"
)

if(UNIX AND NOT APPLE)

  set(BREEZE_HAVE_X11 ${XCB_XCB_FOUND})
  if (XCB_XCB_FOUND)
    find_package(Qt5 REQUIRED CONFIG COMPONENTS X11Extras)
  endif()

else()

  set(BREEZE_HAVE_X11 FALSE)

endif()

################# configuration #################
configure_file(breeze/config-breeze.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-breeze.h )

add_subdirectory(libbreezecommon)
add_subdirectory(libdbusmenuqt)

################# includes #################
include_directories(${CMAKE_SOURCE_DIR}/src/libbreezecommon)
include_directories(${CMAKE_BINARY_DIR}/src/libbreezecommon)
include_directories(${dbusmenu-qt5_INCLUDE_DIRS})
include_directories(breeze)

################# newt target #################
### plugin classes
set(breezelimdecoration_SRCS
    breeze/breezebutton.cpp
    breezebuttonlim.cpp
    breeze/breezedecoration.cpp
    breezedecorationlim.cpp
    breeze/breezeexceptionlist.cpp
    breeze/breezesettingsprovider.cpp
    breeze/breezesizegrip.cpp
    libbreezecommon/breezeboxshadowrenderer.cpp
    KDecoration2Lim/decoration.cpp
    KDecoration2Lim/decorationbutton.cpp
    KDecoration2Lim/decorationbuttongroup.cpp
    KDecoration2Lim/decorationsettings.cpp
    KDecoration2Lim/decoratedclient.cpp
    KDecoration2Lim/private/decoratedclientprivate.cpp
    KDecoration2Lim/appmenumodel.cpp
    KWinLim/decoratedclient.cpp)

kconfig_add_kcfg_files(breezelimdecoration_SRCS breeze/breezesettings.kcfgc)

### config classes
### they are kept separately because they might move in a separate library in the future
set(breezelimdecoration_config_SRCS
    breeze/config/breezeconfigwidget.cpp
    breeze/config/breezedetectwidget.cpp
    breeze/config/breezeexceptiondialog.cpp
    breeze/config/breezeexceptionlistwidget.cpp
    breeze/config/breezeexceptionmodel.cpp
    breeze/config/breezeitemmodel.cpp
)

set(breezelimdecoration_config_PART_FORMS
   breeze/config/ui/breezeconfigurationui.ui
   breeze/config/ui/breezedetectwidget.ui
   breeze/config/ui/breezeexceptiondialog.ui
   breeze/config/ui/breezeexceptionlistwidget.ui
)

ki18n_wrap_ui(breezelimdecoration_config_PART_FORMS_HEADERS ${breezelimdecoration_config_PART_FORMS})

### build library
add_library(breezelimdecoration MODULE
    ${breezelimdecoration_SRCS}
    ${breezelimdecoration_config_SRCS}
    ${breezelimdecoration_config_PART_FORMS_HEADERS})

target_link_libraries(breezelimdecoration
    PUBLIC
        Qt::Core
        Qt::Gui
        Qt::DBus
    PRIVATE
        dbusmenuqt
#        dbusmenu-qt5
#        breezecommon5
        KWaylandServer
        KF5::WaylandClient
        PW::LibTaskManager
        KDecoration2::KDecoration
        kdecorations2private
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::ConfigWidgets
        KF5::GuiAddons
        KF5::I18n
        KF5::IconThemes
        KF5::WindowSystem)

if(BREEZE_HAVE_X11)
  target_link_libraries(breezelimdecoration
    PUBLIC
      Qt::X11Extras
      XCB::XCB)
endif()


install(TARGETS breezelimdecoration DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2)
install(FILES config/breezelimdecorationconfig.desktop DESTINATION  ${KDE_INSTALL_KSERVICES5DIR})
