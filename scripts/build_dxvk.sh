#!/bin/bash
# Script to build DXVK-native and install it to the specified prefix

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SOURCE_ROOT="$(dirname "$SCRIPT_DIR")"
DXVK_DIR="$SOURCE_ROOT/external/dxvk"
BUILD_DIR="$DXVK_DIR/build.native"
INSTALL_PREFIX="${1:-$HOME/.local}"

echo "Building DXVK-native..."
echo "  Source: $DXVK_DIR"
echo "  Build:  $BUILD_DIR"
echo "  Install: $INSTALL_PREFIX"

# Check if DXVK submodule exists and is initialized
# .git can be either a directory or a file (gitlink) in a submodule
if [ ! -e "$DXVK_DIR/.git" ] || [ ! -f "$DXVK_DIR/meson.build" ]; then
    echo "Error: DXVK submodule not initialized. Run: git submodule update --init external/dxvk"
    exit 1
fi

# Initialize DXVK's own submodules (spirv_headers, etc.)
echo "Initializing DXVK submodules..."
cd "$DXVK_DIR"
git submodule update --init --recursive

# Detect architecture
ARCH=$(uname -m)
case "$ARCH" in
    x86_64)
        CROSS_FILE=""
        ;;
    aarch64)
        # Create cross-file for ARM64 if it doesn't exist
        CROSS_FILE="$DXVK_DIR/build-arm64.txt"
        if [ ! -f "$CROSS_FILE" ]; then
            cat > "$CROSS_FILE" << 'EOF'
[binaries]
c = 'gcc'
cpp = 'g++'
ar = 'ar'
strip = 'strip'
pkgconfig = 'pkg-config'

[host_machine]
system = 'linux'
cpu_family = 'aarch64'
cpu = 'aarch64'
endian = 'little'
EOF
        fi
        CROSS_FILE="--cross-file $CROSS_FILE"
        ;;
    *)
        echo "Unsupported architecture: $ARCH"
        exit 1
        ;;
esac

# Configure DXVK with meson
if [ ! -f "$BUILD_DIR/build.ninja" ]; then
    # Build directory doesn't exist or is invalid, reconfigure
    echo "Configuring DXVK..."
    cd "$DXVK_DIR"
    rm -rf "$BUILD_DIR"
    meson setup "$BUILD_DIR" \
        --prefix="$INSTALL_PREFIX" \
        --libdir=lib/$ARCH-linux-gnu \
        --buildtype=release \
        -Denable_dxgi=true \
        -Denable_d3d9=true \
        -Denable_d3d10=false \
        -Denable_d3d11=false \
        -Dnative_sdl2=enabled \
        $CROSS_FILE
fi

# Build DXVK
echo "Building DXVK..."
cd "$DXVK_DIR"
meson compile -C "$BUILD_DIR" -j$(nproc)

# Install DXVK
echo "Installing DXVK to $INSTALL_PREFIX..."
meson install -C "$BUILD_DIR" --no-rebuild

echo "DXVK build complete!"
echo "Libraries installed to: $INSTALL_PREFIX/lib/$ARCH-linux-gnu/"

