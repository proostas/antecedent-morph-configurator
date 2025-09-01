cmake_minimum_required(VERSION 3.19)
project(antecedent-morph-configurator LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(antecedent-morph-configurator
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.hpp

    schema.hpp schema.cpp
    schemamodel.hpp schemamodel.cpp
    schemaview.hpp schemaview.cpp
    schemapropertiesdialog.hpp schemapropertiesdialog.cpp
    lineedit.hpp lineedit.cpp
    codegeneratordialog.hpp codegeneratordialog.cpp
    codegenerator.hpp codegenerator.cpp
    zmkcodegenerator.hpp zmkcodegenerator.cpp
    qmkcodegenerator.hpp qmkcodegenerator.cpp
)

target_link_libraries(antecedent-morph-configurator
    PRIVATE
        Qt::Core
        Qt::Widgets
)

include(GNUInstallDirs)

install(TARGETS antecedent-morph-configurator
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET antecedent-morph-configurator
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
