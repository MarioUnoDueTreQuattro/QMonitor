cmake_minimum_required(VERSION 3.19)
project(QMonitor LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(QMonitor
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    netgraphwidget.h netgraphwidget.cpp
)

target_link_libraries(QMonitor
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_include_directories(QMonitor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
install(TARGETS QMonitor
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QMonitor
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
