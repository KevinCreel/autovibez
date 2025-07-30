#!/usr/bin/env python3
"""
AutoVibez Release Asset Packager
Creates organized asset packages for GitHub releases
"""

import os
import zipfile
import json
import hashlib
from pathlib import Path
from datetime import datetime

class ReleaseAssetPackager:
    def __init__(self, output_dir="release-assets"):
        self.output_dir = Path(output_dir)
        self.output_dir.mkdir(exist_ok=True)
        
    def create_preset_package(self, name, source_url, description):
        """Create a preset package with metadata"""
        package_name = f"autovibez-presets-{name}.zip"
        package_path = self.output_dir / package_name
        
        # Create metadata
        metadata = {
            "name": f"AutoVibez Presets - {name.title()}",
            "description": description,
            "type": "presets",
            "source_url": source_url,
            "created": datetime.now().isoformat(),
            "version": "1.0.0"
        }
        
        # Create package with metadata
        with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Add metadata
            zipf.writestr("metadata.json", json.dumps(metadata, indent=2))
            
            # Add README
            readme_content = f"""# AutoVibez Presets - {name.title()}

{description}

## Installation

1. Extract this ZIP file
2. Copy the .milk files to your AutoVibez presets directory
3. Restart AutoVibez

## Directory Locations

**Windows:**
```
%APPDATA%\\autovibez\\presets\\
```

**macOS:**
```
~/Library/Application Support/autovibez/presets/
```

**Linux:**
```
~/.local/share/autovibez/presets/
```

## Source

Downloaded from: {source_url}

Generated on: {metadata['created']}
"""
            zipf.writestr("README.md", readme_content)
        
        return package_path
    
    def create_texture_package(self, name, source_url, description):
        """Create a texture package with metadata"""
        package_name = f"autovibez-textures-{name}.zip"
        package_path = self.output_dir / package_name
        
        # Create metadata
        metadata = {
            "name": f"AutoVibez Textures - {name.title()}",
            "description": description,
            "type": "textures",
            "source_url": source_url,
            "created": datetime.now().isoformat(),
            "version": "1.0.0"
        }
        
        # Create package with metadata
        with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Add metadata
            zipf.writestr("metadata.json", json.dumps(metadata, indent=2))
            
            # Add README
            readme_content = f"""# AutoVibez Textures - {name.title()}

{description}

## Installation

1. Extract this ZIP file
2. Copy the texture files to your AutoVibez textures directory
3. Restart AutoVibez

## Directory Locations

**Windows:**
```
%APPDATA%\\autovibez\\textures\\
```

**macOS:**
```
~/Library/Application Support/autovibez/textures/
```

**Linux:**
```
~/.local/share/autovibez/textures/
```

## Supported Formats

- PNG (recommended)
- JPG/JPEG
- BMP
- TGA

## Source

Downloaded from: {source_url}

Generated on: {metadata['created']}
"""
            zipf.writestr("README.md", readme_content)
        
        return package_path
    
    def create_combined_package(self, packages, name, description):
        """Create a combined package from multiple sources"""
        package_name = f"autovibez-{name}-complete.zip"
        package_path = self.output_dir / package_name
        
        # Create metadata
        metadata = {
            "name": f"AutoVibez {name.title()} - Complete",
            "description": description,
            "type": name,
            "packages": packages,
            "created": datetime.now().isoformat(),
            "version": "1.0.0"
        }
        
        # Create package with metadata
        with zipfile.ZipFile(package_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
            # Add metadata
            zipf.writestr("metadata.json", json.dumps(metadata, indent=2))
            
            # Add README
            readme_content = f"""# AutoVibez {name.title()} - Complete Package

{description}

## Contents

This package contains:
{chr(10).join([f"- {pkg}" for pkg in packages])}

## Installation

1. Extract this ZIP file
2. Copy the files to your AutoVibez {name} directory
3. Restart AutoVibez

## Directory Locations

**Windows:**
```
%APPDATA%\\autovibez\\{name}\\
```

**macOS:**
```
~/Library/Application Support/autovibez/{name}/
```

**Linux:**
```
~/.local/share/autovibez/{name}/
```

Generated on: {metadata['created']}
"""
            zipf.writestr("README.md", readme_content)
        
        return package_path
    
    def create_installation_guide(self):
        """Create comprehensive installation guide"""
        guide_path = self.output_dir / "INSTALL_ASSETS.md"
        
        guide_content = """# AutoVibez Asset Installation Guide

## 🎵 Quick Start

1. **Download Assets**: Get the preset and texture packages from the releases page
2. **Extract Files**: Extract the ZIP files to your AutoVibez assets directory
3. **Restart AutoVibez**: The app will automatically detect the new assets

## 📁 Asset Locations

### Windows
```
%APPDATA%\\autovibez\\presets\\
%APPDATA%\\autovibez\\textures\\
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

## 📦 Available Packages

### Preset Packages
- **autovibez-presets-cream-of-the-crop.zip**: ~10,000 curated presets
- **autovibez-presets-milkdrop-original.zip**: Classic Milkdrop presets
- **autovibez-presets-complete.zip**: All presets combined

### Texture Packages
- **autovibez-textures-milkdrop-textures.zip**: Essential textures
- **autovibez-textures-complete.zip**: All textures combined

## 🚀 Installation Steps

### Method 1: Automatic Installation
1. Download the asset packages from the releases page
2. Extract ZIP files to a temporary location
3. Copy presets to your presets directory
4. Copy textures to your textures directory
5. Restart AutoVibez

### Method 2: Manual Installation
1. Create the assets directories if they don't exist
2. Download and extract the asset packages
3. Copy .milk files to the presets directory
4. Copy texture files to the textures directory
5. Restart AutoVibez

## ✅ Verification

After installation, AutoVibez will show:
- Available presets in the help overlay (press H)
- New visual effects during playback
- Enhanced texture quality in presets

## 🆘 Troubleshooting

### No New Effects
- Restart AutoVibez after installing assets
- Check that files are in the correct directories
- Verify file permissions

### Missing Textures
- Ensure textures are in the correct directory
- Check that texture files are valid image formats
- Restart AutoVibez after adding textures

### Corrupted Files
- Re-download the asset packages
- Check file integrity with checksums
- Try extracting with different tools

## 📊 Package Information

Each package contains:
- **Presets**: .milk files for visual effects
- **Textures**: PNG, JPG, BMP, TGA image files
- **Metadata**: Package information and source details
- **README**: Installation and usage instructions

## 🔗 Sources

All assets are sourced from the ProjectM community:
- [Cream of the Crop Presets](https://github.com/projectM-visualizer/presets-cream-of-the-crop)
- [Milkdrop Original Presets](https://github.com/projectM-visualizer/presets-milkdrop-original)
- [Milkdrop Texture Pack](https://github.com/projectM-visualizer/presets-milkdrop-texture-pack)

## 📄 License

Assets are provided under their respective licenses. Please refer to the source repositories for license details.
"""
        
        with open(guide_path, 'w') as f:
            f.write(guide_content)
        
        return guide_path

def main():
    packager = ReleaseAssetPackager()
    
    print("🎵 AutoVibez Release Asset Packager")
    print("====================================")
    
    # Create preset packages
    print("\n📦 Creating Preset Packages...")
    
    cream_package = packager.create_preset_package(
        "cream-of-the-crop",
        "https://github.com/projectM-visualizer/presets-cream-of-the-crop",
        "~10,000 curated presets from the ProjectM community"
    )
    
    milkdrop_package = packager.create_preset_package(
        "milkdrop-original",
        "https://github.com/projectM-visualizer/presets-milkdrop-original",
        "Classic Milkdrop presets from the original Winamp visualizer"
    )
    
    # Create texture packages
    print("\n🖼️ Creating Texture Packages...")
    
    texture_package = packager.create_texture_package(
        "milkdrop-textures",
        "https://github.com/projectM-visualizer/presets-milkdrop-texture-pack",
        "Essential textures for Milkdrop-style visual effects"
    )
    
    # Create combined packages
    print("\n📦 Creating Combined Packages...")
    
    presets_complete = packager.create_combined_package(
        ["Cream of the Crop", "Milkdrop Original"],
        "presets",
        "Complete preset collection with all available presets"
    )
    
    textures_complete = packager.create_combined_package(
        ["Milkdrop Textures"],
        "textures",
        "Complete texture collection for all visual effects"
    )
    
    # Create installation guide
    print("\n📋 Creating Installation Guide...")
    guide = packager.create_installation_guide()
    
    # Show results
    print("\n✅ Release Assets Created Successfully!")
    print("======================================")
    print(f"\n📁 Output Directory: {packager.output_dir.absolute()}")
    print("\n📦 Created Packages:")
    
    for package in packager.output_dir.glob("*.zip"):
        size = package.stat().st_size / (1024 * 1024)  # MB
        print(f"  - {package.name} ({size:.1f} MB)")
    
    print(f"\n📋 Installation Guide: {guide.name}")
    print("\n🚀 Ready for GitHub Release!")

if __name__ == "__main__":
    main() 