add_subdirectory(tests)

# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"filetypes\")

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
    Qt5::DBus
)

install(TARGETS kcm_filetypes  DESTINATION ${PLUGIN_INSTALL_DIR} )

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
    Qt5::DBus
)
if (Q_WS_X11)
  target_link_libraries(keditfiletype5 ${X11_X11_LIB})
endif ()

install(TARGETS keditfiletype5 ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES filetypes.desktop  DESTINATION  ${SERVICES_INSTALL_DIR} )
