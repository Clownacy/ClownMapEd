mkdir build-makeAppImage
if [ ! -f build-makeAppImage/linuxdeploy-x86_64.AppImage ]; then
	wget -O build-makeAppImage/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	if [ $? -ne 0 ]; then
		curl -L -o build-makeAppImage/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	fi
fi
chmod +x build-makeAppImage/linuxdeploy-x86_64.AppImage
rm -r build-makeAppImage/AppDir
mkdir build-makeAppImage/AppDir
cmake -B build-makeAppImage -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build build-makeAppImage --config Release --parallel $(nproc)
DESTDIR=AppDir cmake --build build-makeAppImage --target install
LINUXDEPLOY_OUTPUT_VERSION=v1.2 LDAI_UPDATE_INFORMATION="gh-releases-zsync|Clownacy|clownmaped|latest|ClownMapEd-*x86_64.AppImage.zsync" build-makeAppImage/linuxdeploy-x86_64.AppImage --appdir build-makeAppImage/AppDir --output appimage
