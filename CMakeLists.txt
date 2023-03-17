cmake_minimum_required(VERSION 3.5)

project(clownmaped VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
	colour-button.cpp
	colour-button.h
	main.cpp
	main-window.cpp
	main-window.h
	main-window.ui
	palette.cpp
	palette.h
	palette-editor.cpp
	palette-editor.h
	sprite-viewer.cpp
	sprite-viewer.h
	sprite-mappings.cpp
	sprite-mappings.h
	tile-pixmaps.cpp
	tile-pixmaps.h
	tile-viewer.cpp
	tile-viewer.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(clownmaped
	MANUAL_FINALIZATION
	${PROJECT_SOURCES}
)
# Define target properties for Android with Qt 6 as:
#	set_property(TARGET clownmaped APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#				${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
	if(ANDROID)
		add_library(clownmaped SHARED
			${PROJECT_SOURCES}
		)
	# Define properties for Android with Qt 5 after find_package() calls as:
	#	set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
	else()
		add_executable(clownmaped
			${PROJECT_SOURCES}
		)
	endif()
endif()

target_link_libraries(clownmaped PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(clownmaped PROPERTIES
	MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
)

install(TARGETS clownmaped
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(clownmaped)
endif()
