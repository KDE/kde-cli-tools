add_subdirectory(tests)

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm5_filetypes\")

set(libfiletypes_SRCS
 filetypedetails.cpp
 filegroupdetails.cpp
 kservicelistwidget.cpp
 typeslistitem.cpp
 mimetypedata.cpp
 mimetypewriter.cpp
 newtypedlg.cpp
 kserviceselectdlg.cpp
)

########### next target ###############

set(kcm_filetypes_SRCS filetypesview.cpp ${libfiletypes_SRCS})

add_library(kcm_filetypes MODULE ${kcm_filetypes_SRCS})

target_link_libraries(kcm_filetypes
    KF5::ConfigWidgets
    KF5::IconThemes
    KF5::I18n
    KF5::KIOWidgets # KOpenWithDialog, KBuildSycocaProgressDialog
    Qt::DBus
)

install(TARGETS kcm_filetypes  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

########### next target ###############

set(keditfiletype_SRCS keditfiletype.cpp ${libfiletypes_SRCS})

add_executable(keditfiletype5 ${keditfiletype_SRCS})
target_compile_definitions(keditfiletype5 PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(keditfiletype5
    KF5::ConfigCore
    KF5::IconThemes
    KF5::KIOWidgets # KOpenWithDialog, KBuildSycocaProgressDialog
    KF5::WindowSystem
    KF5::I18n
    KF5::Service
    Qt::DBus
)
if (Q_WS_X11)
  target_link_libraries(keditfiletype5 ${X11_X11_LIB})
endif ()

install(TARGETS keditfiletype5 ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES filetypes.desktop  DESTINATION  ${KDE_INSTALL_KSERVICES5DIR} )
install( PROGRAMS org.kde.keditfiletype.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
