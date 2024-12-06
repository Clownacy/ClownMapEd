mkdir build-makeAppImage

# Download LinuxDeploy.
if [ ! -f build-makeAppImage/linuxdeploy-x86_64.AppImage ]; then
	wget -O build-makeAppImage/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	if [ $? -ne 0 ]; then
		curl -L -o build-makeAppImage/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	fi
fi

# Make LinuxDeploy executable.
chmod +x build-makeAppImage/linuxdeploy-x86_64.AppImage

# Download LinuxDeply's Qt plugin.
if [ ! -f build-makeAppImage/linuxdeploy-plugin-qt-x86_64.AppImage ]; then
	wget -O build-makeAppImage/linuxdeploy-plugin-qt-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-plugin-qt-x86_64.AppImage
	if [ $? -ne 0 ]; then
		curl -L -o build-makeAppImage/linuxdeploy-plugin-qt-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-plugin-qt-x86_64.AppImage
	fi
fi

chmod +x build-makeAppImage/linuxdeploy-plugin-qt-x86_64.AppImage

# We want...
# - An optimised Release build.
# - For the program to not be installed to '/usr/local', so that LinuxDeploy detects it.
# - Link-time optimisation, for improved optimisation.
# - To set the CMake policy that normally prevents link-time optimisation.
cmake -B build-makeAppImage -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON -DCMAKE_POLICY_DEFAULT_CMP0069=NEW

# Once again specify the Release build, for generators that required it be done this way.
# Build in parallel to speed-up compilation greatly.
cmake --build build-makeAppImage --config Release --parallel $(nproc)

# Make a temporary directory to install the built files into.
# Make sure that it is fresh and empty, in case this script was ran before. We don't want old, leftover files.
rm -r build-makeAppImage/AppDir
mkdir build-makeAppImage/AppDir

# Install into the temporary directory.
DESTDIR=AppDir cmake --build build-makeAppImage --target install

# Produce the AppImage, and bundle it with update metadata.
LINUXDEPLOY_OUTPUT_VERSION=v1.2 LDAI_UPDATE_INFORMATION="gh-releases-zsync|Clownacy|clownmaped|latest|ClownMapEd-*x86_64.AppImage.zsync" build-makeAppImage/linuxdeploy-x86_64.AppImage --appdir build-makeAppImage/AppDir --output appimage --plugin qt
