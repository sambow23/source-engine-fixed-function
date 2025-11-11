#!/bin/bash
# Half-Life 2 launcher with DXVK-native support

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Set up library paths - DXVK libraries are now installed in bin/ by waf
export LD_LIBRARY_PATH="$SCRIPT_DIR/bin:$LD_LIBRARY_PATH"

# DXVK-native configuration
export DXVK_WSI_DRIVER=SDL2  # Required: Tell DXVK to use SDL2 for window system integration

# Enable DXVK debug logging (optional - comment out for production)
# export DXVK_LOG_LEVEL=info
# export DXVK_HUD=fps

# Launch the game
cd "$SCRIPT_DIR"
exec ./hl2_launcher "$@"

