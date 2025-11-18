mkdir -p build

# Detect architecture.
ARCH=$(uname -m)

# Set LinuxDeploy filename based on architecture.
LINUXDEPLOY_FILENAME=linuxdeploy-${ARCH}.AppImage

# Download LinuxDeploy if not already downloaded.
if [ ! -f "build/$LINUXDEPLOY_FILENAME" ]; then
    wget -O "build/$LINUXDEPLOY_FILENAME" "https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/$LINUXDEPLOY_FILENAME"
    if [ $? -ne 0 ]; then
        curl -L -o "build/$LINUXDEPLOY_FILENAME" "https://github.com/linuxdeploy/linuxdeploy/releases/download/1-alpha-20251107-1/$LINUXDEPLOY_FILENAME"
    fi
fi

# Make LinuxDeploy executable.
chmod +x "build/$LINUXDEPLOY_FILENAME"

# Set LinuxDeploy's Qt plugin filename based on architecture.
LINUXDEPLOY_QT_FILENAME=linuxdeploy-plugin-qt-${ARCH}.AppImage

# Download LinuxDeply's Qt plugin if not already downloaded.
if [ ! -f "build/$LINUXDEPLOY_QT_FILENAME" ]; then
    wget -O "build/$LINUXDEPLOY_QT_FILENAME" "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/$LINUXDEPLOY_QT_FILENAME"
    if [ $? -ne 0 ]; then
        curl -L -o "build/$LINUXDEPLOY_QT_FILENAME" "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/1-alpha-20250213-1/$LINUXDEPLOY_QT_FILENAME"
    fi
fi

# Make LinuxDeploy's Qt plugin executable.
chmod +x "build/$LINUXDEPLOY_QT_FILENAME"

# We want...
# - An optimised Release build.
# - For the program to not be installed to '/usr/local', so that LinuxDeploy detects it.
# - Link-time optimisation, for improved optimisation.
# - To set the CMake policy that normally prevents link-time optimisation.
cmake -B build ../../ \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON \
    -DCMAKE_POLICY_DEFAULT_CMP0069=NEW

# Once again specify the Release build, for generators that required it be done this way.
# Build in parallel to speed-up compilation greatly.
cmake --build build --config Release --parallel $(nproc)

# Make a temporary directory to install the built files into.
# Make sure that it is fresh and empty, in case this script was ran before; we do not want old, leftover files.
rm -rf build/AppDir
mkdir -p build/AppDir

# Install into the temporary directory.
DESTDIR=AppDir cmake --build build --target install

# Produce the AppImage, and bundle it with update metadata.
LINUXDEPLOY_OUTPUT_VERSION=v1.3.0.2 \
LDAI_UPDATE_INFORMATION="gh-releases-zsync|Clownacy|clownmaped|latest|ClownMapEd-*${ARCH}.AppImage.zsync" \
"build/$LINUXDEPLOY_FILENAME" --appdir build/AppDir --output appimage --plugin qt
