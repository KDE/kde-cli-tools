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
 filetypedetails.h
 filegroupdetails.h
 kservicelistwidget.h
 typeslistitem.h
 mimetypedata.h
 mimetypewriter.h
 newtypedlg.h
 kserviceselectdlg.h
)

########### next target ###############

set(kcm_filetypes_SRCS filetypesview.cpp ${libfiletypes_SRCS})

kcoreaddons_add_plugin(kcm_filetypes SOURCES ${kcm_filetypes_SRCS} INSTALL_NAMESPACE "plasma/kcms/systemsettings_qwidgets")

kcmutils_generate_desktop_file(kcm_filetypes)
target_link_libraries(kcm_filetypes
    KF5::ConfigWidgets
    KF5::IconThemes
    KF5::I18n
    KF5::KIOWidgets # KOpenWithDialog, KBuildSycocaProgressDialog
    KF5::Parts
    Qt::DBus
)

########### next target ###############

set(keditfiletype_SRCS keditfiletype.cpp keditfiletype.h ${libfiletypes_SRCS})

add_executable(keditfiletype ${keditfiletype_SRCS})
target_compile_definitions(keditfiletype PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(keditfiletype
    KF5::ConfigCore
    KF5::IconThemes
    KF5::KIOWidgets # KOpenWithDialog, KBuildSycocaProgressDialog
    KF5::WindowSystem
    KF5::I18n
    KF5::Service
    KF5::Parts
    Qt::DBus
)

install_compat_symlink(keditfiletype)
install(TARGETS keditfiletype DESTINATION ${KDE_INSTALL_FULL_BINDIR})

########### install files ###############

install( PROGRAMS org.kde.keditfiletype.desktop DESTINATION ${KDE_INSTALL_APPDIR} )
