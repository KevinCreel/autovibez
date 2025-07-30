#!/bin/bash

# AutoVibez Release Asset Creator
# Creates separate asset packages for GitHub releases

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
RELEASE_DIR="$PROJECT_ROOT/release-assets"

echo "🎵 AutoVibez Release Asset Creator"
echo "==================================="

# Create release assets directory
mkdir -p "$RELEASE_DIR"

# Function to download and package presets
create_preset_package() {
    local preset_name="$1"
    local preset_url="$2"
    local package_name="autovibez-presets-${preset_name}.zip"
    
    echo "📥 Creating preset package: $package_name"
    
    # Create temporary directory
    local temp_dir="$RELEASE_DIR/temp-${preset_name}"
    mkdir -p "$temp_dir"
    cd "$temp_dir"
    
    # Download preset pack
    echo "  - Downloading from: $preset_url"
    curl -L -o "${preset_name}.zip" "$preset_url"
    unzip -q "${preset_name}.zip"
    
    # Find and copy .milk files
    find . -name "*.milk" -type f | while read -r file; do
        # Create directory structure
        local target_dir="$RELEASE_DIR/presets"
        mkdir -p "$target_dir"
        
        # Copy file with relative path
        cp "$file" "$target_dir/"
        echo "  + $(basename "$file")"
    done
    
    # Create zip package
    cd "$RELEASE_DIR"
    zip -r "$package_name" presets/
    
    # Cleanup
    rm -rf "$temp_dir" presets/
    
    echo "  ✅ Created: $package_name"
}

# Function to download and package textures
create_texture_package() {
    local texture_name="$1"
    local texture_url="$2"
    local package_name="autovibez-textures-${texture_name}.zip"
    
    echo "📥 Creating texture package: $package_name"
    
    # Create temporary directory
    local temp_dir="$RELEASE_DIR/temp-${texture_name}"
    mkdir -p "$temp_dir"
    cd "$temp_dir"
    
    # Download texture pack
    echo "  - Downloading from: $texture_url"
    curl -L -o "${texture_name}.zip" "$texture_url"
    unzip -q "${texture_name}.zip"
    
    # Find and copy texture files
    find . -type f \( -name "*.png" -o -name "*.jpg" -o -name "*.jpeg" -o -name "*.bmp" -o -name "*.tga" \) | while read -r file; do
        # Create directory structure
        local target_dir="$RELEASE_DIR/textures"
        mkdir -p "$target_dir"
        
        # Copy file with relative path
        cp "$file" "$target_dir/"
        echo "  + $(basename "$file")"
    done
    
    # Create zip package
    cd "$RELEASE_DIR"
    zip -r "$package_name" textures/
    
    # Cleanup
    rm -rf "$temp_dir" textures/
    
    echo "  ✅ Created: $package_name"
}

# Create preset packages
echo ""
echo "🎨 Creating Preset Packages"
echo "---------------------------"

create_preset_package "cream-of-the-crop" \
    "https://github.com/projectM-visualizer/presets-cream-of-the-crop/archive/refs/heads/main.zip"

create_preset_package "milkdrop-original" \
    "https://github.com/projectM-visualizer/presets-milkdrop-original/archive/refs/heads/main.zip"

# Create texture packages
echo ""
echo "🖼️ Creating Texture Packages"
echo "----------------------------"

create_texture_package "milkdrop-textures" \
    "https://github.com/projectM-visualizer/presets-milkdrop-texture-pack/archive/refs/heads/main.zip"

# Create combined packages
echo ""
echo "📦 Creating Combined Packages"
echo "----------------------------"

# Combined presets package
echo "📥 Creating combined presets package..."
cd "$RELEASE_DIR"
if [ -f "autovibez-presets-cream-of-the-crop.zip" ] && [ -f "autovibez-presets-milkdrop-original.zip" ]; then
    # Create combined presets
    mkdir -p combined-presets
    unzip -q autovibez-presets-cream-of-the-crop.zip -d combined-presets/
    unzip -q autovibez-presets-milkdrop-original.zip -d combined-presets/
    zip -r "autovibez-presets-complete.zip" combined-presets/
    rm -rf combined-presets/
    echo "  ✅ Created: autovibez-presets-complete.zip"
fi

# Combined textures package
echo "📥 Creating combined textures package..."
if [ -f "autovibez-textures-milkdrop-textures.zip" ]; then
    # Rename to complete textures
    cp "autovibez-textures-milkdrop-textures.zip" "autovibez-textures-complete.zip"
    echo "  ✅ Created: autovibez-textures-complete.zip"
fi

# Create installation instructions
echo ""
echo "📋 Creating Installation Instructions"
echo "-----------------------------------"

cat > "$RELEASE_DIR/INSTALL_ASSETS.md" << 'EOF'
# AutoVibez Asset Installation

## Quick Installation

1. **Download Assets**: Download the preset and texture packages from the releases page
2. **Extract Files**: Extract the ZIP files to your AutoVibez assets directory
3. **Restart AutoVibez**: The app will automatically detect the new assets

## Asset Locations

### Windows
```
%APPDATA%\autovibez\presets\
%APPDATA%\autovibez\textures\
```

### macOS
```
~/Library/Application Support/autovibez/presets/
~/Library/Application Support/autovibez/textures/
```

### Linux
```
~/.local/share/autovibez/presets/
~/.local/share/autovibez/textures/
```

## Package Contents

### Preset Packages
- **autovibez-presets-cream-of-the-crop.zip**: ~10,000 curated presets
- **autovibez-presets-milkdrop-original.zip**: Classic Milkdrop presets
- **autovibez-presets-complete.zip**: All presets combined

### Texture Packages
- **autovibez-textures-milkdrop-textures.zip**: Essential textures
- **autovibez-textures-complete.zip**: All textures combined

## Installation Steps

1. **Download**: Get the asset packages from the releases page
2. **Extract**: Extract ZIP files to a temporary location
3. **Copy**: Copy presets to your presets directory
4. **Copy**: Copy textures to your textures directory
5. **Restart**: Launch AutoVibez to see new effects

## Verification

After installation, AutoVibez will show:
- Available presets in the help overlay (press H)
- New visual effects during playback
- Enhanced texture quality in presets

## Troubleshooting

- **No New Effects**: Restart AutoVibez after installing assets
- **Missing Textures**: Ensure textures are in the correct directory
- **Corrupted Files**: Re-download the asset packages
EOF

echo "  ✅ Created: INSTALL_ASSETS.md"

# Show final results
echo ""
echo "✅ Release Assets Created Successfully!"
echo "======================================"
echo ""
echo "📁 Release Assets Directory: $RELEASE_DIR"
echo ""
echo "📦 Available Packages:"
ls -la "$RELEASE_DIR"/*.zip
echo ""
echo "📋 Installation Guide: $RELEASE_DIR/INSTALL_ASSETS.md"
echo ""
echo "🚀 Ready for GitHub Release!"
echo ""
echo "Upload these files as release assets:"
echo "- autovibez-presets-cream-of-the-crop.zip"
echo "- autovibez-presets-milkdrop-original.zip"
echo "- autovibez-presets-complete.zip"
echo "- autovibez-textures-milkdrop-textures.zip"
echo "- autovibez-textures-complete.zip"
echo "- INSTALL_ASSETS.md" 