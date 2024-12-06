mkdir makeAppImage-build
if [ ! -f makeAppImage-build/linuxdeploy-x86_64.AppImage ]; then
	wget -O makeAppImage-build/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	if [ $? -ne 0 ]; then
		curl -L -o makeAppImage-build/linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/2.0.0-alpha-1-20241106/linuxdeploy-x86_64.AppImage
	fi
fi
chmod +x makeAppImage-build/linuxdeploy-x86_64.AppImage
rm -r makeAppImage-build/AppDir
mkdir makeAppImage-build/AppDir
cmake -B makeAppImage-build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr
cmake --build makeAppImage-build --config Release --parallel $(nproc)
DESTDIR=AppDir cmake --build makeAppImage-build --target install
LINUXDEPLOY_OUTPUT_VERSION=v1.2 LDAI_UPDATE_INFORMATION="gh-releases-zsync|Clownacy|clownmaped|latest|ClownMapEd-*x86_64.AppImage.zsync" makeAppImage-build/linuxdeploy-x86_64.AppImage --appdir makeAppImage-build/AppDir --output appimage
