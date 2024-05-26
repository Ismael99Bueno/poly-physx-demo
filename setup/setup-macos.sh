#!/bin/bash

if [[ "$OSTYPE" != "darwin"* ]]
then
    echo "Cannot run setup because your machine does not have the required OS! Required: MacOS, current: $OSTYPE"
    exit 1
fi

echo "Checking python intallation..."
if ! which -s python3
then
    echo "Python 3 installation not found. Do you wish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "Python 3 installation failed... Setup terminated"
        exit
    fi
    if ! install_brew
    then
        echo "Homebrew installation failed... Setup terminated"
        exit
    fi

    brew install python@3
    if ! which -s python3
    then
        echo "Python 3 installation failed... Setup terminated"
        exit 1
    fi
fi
echo "Python3 installed
"

echo "Checking XCode CLT intallation..."
if ! xcode-select -p &> /dev/null
then
    echo "XCode Command Line Tools installation not found. Do you wish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "XCode CLT installation failed... Setup terminated"
        exit
    fi

    echo "Installing XCode CLT..."
    xcode-select --install
    if ! xcode-select -p &> /dev/null
    then
        echo "XCode CLT installation failed... Setup terminated"
        exit 1
    fi

fi
echo "XCode Command Line Tools installed
"

function install_brew
{
    if which -s brew
    then
        return 0
    fi

    echo "Homebrew is required for the previous installation. Do you whish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        return 1
    fi

    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    return
}

echo "Checking premake installation..."

if ! which -s premake5
then
    echo "premake installation not found. Do you wish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "premake installation failed... Setup terminated"
        exit
    fi
    if ! install_brew
    then
        echo "Homebrew installation failed... Setup terminated"
        exit
    fi

    brew install premake
    if ! which -s premake5
    then
        echo "premake installation failed... Setup terminated"
        exit 1
    fi
fi
echo "premake installed
"

echo "Checking make installation..."

if ! which -s make
then
    echo "make installation not found. Do you wish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "make installation failed... Setup terminated"
        exit
    fi
    if ! install_brew
    then
        echo "Homebrew installation failed... Setup terminated"
        exit
    fi

    brew install make
    if ! which -s make
    then
        echo "make installation failed... Setup terminated"
        exit 1
    fi
fi
echo "make installed
"

SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
ROOT="$DIR/.."


if [ ! -d "/usr/local/include/vulkan" ] || [ ! -f "/usr/local/lib/libvulkan.dylib" ]
then
    echo "VulkanSDK installation not found. Do you wish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "VulkanSDK installation failed... Setup terminated"
        exit
    fi

    if [ ! -f "$ROOT/vendor/vulkan-sdk/bin/vulkan-installer.dmg" ]
    then
        mkdir -p "$ROOT/vendor/vulkan-sdk/bin"
        curl -o "$ROOT/vendor/vulkan-sdk/bin/vulkan-installer.dmg" "https://sdk.lunarg.com/sdk/download/1.3.250.1/mac/vulkansdk-macos-1.3.250.1.dmg"
    fi

    hdiutil attach "$ROOT/vendor/vulkan-sdk/bin/vulkan-installer.dmg"
    /Volumes/VulkanSDK/InstallVulkan.app/Contents/MacOS/InstallVulkan --root ~/VulkanSDK/1.3.250.1 --accept-licenses --default-answer --confirm-command install com.lunarg.vulkan.core com.lunarg.vulkan.usr
    hdiutil detach /Volumes/VulkanSDK

    if [ ! -d "/usr/local/include/vulkan" ] || [ ! -f "/usr/local/lib/libvulkan.dylib" ]
    then
        echo "VulkanSDK installation failed... Setup terminated"
        exit 1
    fi
fi
echo "VulkanSDK installed
"

echo "Compiling shaders...
"

"$ROOT"/lynx/scripts/unix-compile-shaders.sh
if ! mycmd
then
    echo "Shader compilation failed... Setup terminated"
    exit 1
fi

python3 "$DIR/src/generate_build_files.py"
if ! mycmd
then
    echo "Build files generation failed... Setup terminated"
    exit 1
fi

echo "Run generate_build_files.py with python to have further details on how the project can be built. Use the -h flag to display the options available
"
echo "Setup completed successfully!"