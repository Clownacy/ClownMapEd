cmake_minimum_required(VERSION 3.5...3.10)

project(ClownMapEd VERSION 1.3.0.3 LANGUAGES CXX)

######
# Qt #
######

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
	"assets/ClownMapEd.rc"
	"icon.qrc"
	"about.cpp"
	"about.h"
	"about.ui"
	"colour-button.cpp"
	"colour-button.h"
	"data-stream.h"
	"dynamic-pattern-load-cues.cpp"
	"dynamic-pattern-load-cues.h"
	"libsonassmd.h"
	"licences.cpp"
	"licences.h"
	"licences.ui"
	"main.cpp"
	"main-window.cpp"
	"main-window.h"
	"main-window.ui"
	"palette.cpp"
	"palette.h"
	"palette-editor.cpp"
	"palette-editor.h"
	"signal-wrapper.h"
	"sprite-viewer.cpp"
	"sprite-viewer.h"
	"sprite-frame.cpp"
	"sprite-frame.h"
	"sprite-mappings.cpp"
	"sprite-mappings.h"
	"sprite-piece.cpp"
	"sprite-piece.h"
	"sprite-piece-button.cpp"
	"sprite-piece-button.h"
	"sprite-piece-picker.cpp"
	"sprite-piece-picker.h"
	"tile-manager.cpp"
	"tile-manager.h"
	"tile-viewer.cpp"
	"tile-viewer.h"
	"utilities.cpp"
	"utilities.h"
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
	qt_add_executable(ClownMapEd
	MANUAL_FINALIZATION
	${PROJECT_SOURCES}
)
# Define target properties for Android with Qt 6 as:
#	set_property(TARGET clownmaped APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#				${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
	if(ANDROID)
		add_library(ClownMapEd SHARED
			${PROJECT_SOURCES}
		)
	# Define properties for Android with Qt 5 after find_package() calls as:
	#	set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
	else()
		add_executable(ClownMapEd
			${PROJECT_SOURCES}
		)
	endif()
endif()

target_link_libraries(ClownMapEd PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(ClownMapEd PROPERTIES
	MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
	MACOSX_BUNDLE TRUE
	WIN32_EXECUTABLE TRUE
)

install(TARGETS ClownMapEd
	BUNDLE DESTINATION .
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
	qt_finalize_executable(ClownMapEd)
endif()

if(EMSCRIPTEN)
	# Needed so that exceptions work.
	# TODO: Make this project not use exceptions in the first place; they're weird.
	target_compile_options(ClownMapEd PUBLIC -sNO_DISABLE_EXCEPTION_CATCHING)
	target_link_options(ClownMapEd PUBLIC -sNO_DISABLE_EXCEPTION_CATCHING)
	# Needed so that message boxes work.
	target_link_options(ClownMapEd PUBLIC -sASYNCIFY)
endif()

#############
# CIEDE2000 #
#############

add_library(CIEDE2000 STATIC
	"CIEDE2000/CIEDE2000.cpp"
	"CIEDE2000/CIEDE2000.h"
)

target_include_directories(CIEDE2000 PRIVATE "CIEDE2000")

target_link_libraries(ClownMapEd PRIVATE CIEDE2000)

###############
# libsonassmd #
###############

add_subdirectory("libsonassmd" EXCLUDE_FROM_ALL)

target_link_libraries(ClownMapEd PRIVATE sonassmd)

################
# Installation #
################

install(TARGETS ClownMapEd DESTINATION "bin")
install(FILES "assets/com.clownacy.ClownMapEd.appdata.xml" DESTINATION "share/metainfo")
install(FILES "assets/com.clownacy.ClownMapEd.desktop" DESTINATION "share/applications")
install(FILES "assets/icon/icon-128.png" DESTINATION "share/icons/hicolor/128x128/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-64.png" DESTINATION "share/icons/hicolor/64x64/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-48.png" DESTINATION "share/icons/hicolor/48x48/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-40.png" DESTINATION "share/icons/hicolor/40x40/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-32.png" DESTINATION "share/icons/hicolor/32x32/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-24.png" DESTINATION "share/icons/hicolor/24x24/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-20.png" DESTINATION "share/icons/hicolor/20x20/apps" RENAME "ClownMapEd.png")
install(FILES "assets/icon/icon-16.png" DESTINATION "share/icons/hicolor/16x16/apps" RENAME "ClownMapEd.png")
