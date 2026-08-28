cmake_minimum_required(VERSION 3.21)

project(AvenBrowser VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.5 REQUIRED COMPONENTS Quick QuickControls2 WebEngineQuick)
include(CTest)

qt_standard_project_setup(REQUIRES 6.5)
qt_policy(SET QTP0004 NEW)

# Keep every QML type at the Aven module root.
set_source_files_properties(qml/Main.qml PROPERTIES QT_RESOURCE_ALIAS Main.qml)
set_source_files_properties(qml/BrowserView.qml PROPERTIES QT_RESOURCE_ALIAS BrowserView.qml)
set_source_files_properties(qml/HomeView.qml PROPERTIES QT_RESOURCE_ALIAS HomeView.qml)
set_source_files_properties(qml/DetachedWindow.qml PROPERTIES QT_RESOURCE_ALIAS DetachedWindow.qml)

qt_add_executable(aven
    src/main.cpp
    src/AppSettings.cpp
    src/AppSettings.h
    src/PerformanceStatus.cpp
    src/PerformanceStatus.h
    src/MemoryController.cpp
    src/MemoryController.h
    src/MemoryPolicy.cpp
    src/MemoryPolicy.h
    src/WindowsMemoryMetrics.cpp
    src/WindowsMemoryMetrics.h
    src/TabManager.cpp
    src/TabManager.h
    src/UrlResolver.cpp
    src/UrlResolver.h
)

qt_add_qml_module(aven
    URI Aven
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/BrowserView.qml
        qml/HomeView.qml
        qml/DetachedWindow.qml
)

qt_add_resources(aven "aven_resources"
    PREFIX "/"
    FILES
        resources/start.html
        resources/aven-nordic-home.png
        resources/aven-app-icon.png
)

target_link_libraries(aven PRIVATE
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::WebEngineQuick
)

if(WIN32)
    target_sources(aven PRIVATE resources/aven.rc)
    target_link_libraries(aven PRIVATE Psapi)
    set_target_properties(aven PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "Aven"
    )
endif()

install(TARGETS aven
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

if(BUILD_TESTING)
    find_package(Qt6 6.5 REQUIRED COMPONENTS Network Test)
    qt_add_executable(aven_urlresolver_tests
        tests/UrlResolverTests.cpp
        src/UrlResolver.cpp
        src/UrlResolver.h
    )
    target_link_libraries(aven_urlresolver_tests PRIVATE Qt6::Network Qt6::Test)
    target_include_directories(aven_urlresolver_tests PRIVATE src)
    add_test(NAME aven_urlresolver_tests COMMAND aven_urlresolver_tests)
    set_tests_properties(aven_urlresolver_tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
    qt_add_executable(aven_memorypolicy_tests
        tests/MemoryPolicyTests.cpp
        src/MemoryPolicy.cpp
        src/MemoryPolicy.h
    )
    target_link_libraries(aven_memorypolicy_tests PRIVATE Qt6::Test)
    target_include_directories(aven_memorypolicy_tests PRIVATE src)
    add_test(NAME aven_memorypolicy_tests COMMAND aven_memorypolicy_tests)
    set_tests_properties(aven_memorypolicy_tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
    qt_add_executable(aven_tabmanager_tests
        tests/TabManagerTests.cpp
        src/AppSettings.cpp
        src/AppSettings.h
        src/TabManager.cpp
        src/TabManager.h
        src/UrlResolver.cpp
        src/UrlResolver.h
        src/WindowsMemoryMetrics.cpp
        src/WindowsMemoryMetrics.h
    )
    target_link_libraries(aven_tabmanager_tests PRIVATE Qt6::Network Qt6::Test)
    target_include_directories(aven_tabmanager_tests PRIVATE src)
    if(WIN32)
        target_link_libraries(aven_tabmanager_tests PRIVATE Psapi)
    endif()
    add_test(NAME aven_tabmanager_tests COMMAND aven_tabmanager_tests)
    set_tests_properties(aven_tabmanager_tests PROPERTIES
        ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:$<TARGET_FILE_DIR:Qt6::Core>"
    )
endif()
