#!/bin/bash

if [[ "$OSTYPE" != "linux-gnu"* ]]
then
    echo "Cannot run setup because your machine does not have the required OS! Required: Linux, current: $OSTYPE"
    exit 1
fi

apt-get update

echo "Checking Python3 installation..."
if ! command -v python3 &> /dev/null; then
    echo "Python 3 installation not found. Installing Python 3..."
    
    apt-get install -y software-properties-common
    add-apt-repository ppa:deadsnakes/ppa -y
    apt-get update
    apt-get install -y python3.11 python3.11-venv python3.11-distutils
    curl -sS https://bootstrap.pypa.io/get-pip.py | python3.11

    if ! command -v python3 &> /dev/null; then
        echo "Python 3 installation failed... Setup terminated"
        exit 1
    fi
else
    echo "Python 3 installed"
fi

echo "Checking build-essential installation..."
if ! dpkg -l | grep -qw build-essential; then
    echo "build-essential not found. Installing build-essential..."
    apt-get update
    apt-get install -y build-essential

    if ! dpkg -l | grep -qw build-essential; then
        echo "build-essential installation failed... Setup terminated"
        exit 1
    fi
else
    echo "build-essential installed"
fi

echo "Checking premake installation..."
if ! command -v premake5 &> /dev/null; then
    echo "premake installation not found. Installing premake..."
    if ! command -v wget &> /dev/null; then
        apt-get install -y wget
    fi
    wget https://github.com/premake/premake-core/releases/download/v5.0.0-alpha15/premake-5.0.0-alpha15-linux.tar.gz
    tar -xzf premake-5.0.0-alpha15-linux.tar.gz
    mv premake5 /usr/local/bin
    rm premake-5.0.0-alpha15-linux.tar.gz

    if ! command -v premake5 &> /dev/null; then
        echo "premake installation failed... Setup terminated"
        exit 1
    fi
else
    echo "premake installed"
fi

echo "Checking Vulkan SDK installation..."
if ! command -v vkcube &> /dev/null; then
    echo "Vulkan SDK installation not found. Installing Vulkan SDK..."
    apt-get install vulkan-tools
    apt-get install libvulkan-dev
    apt-get install vulkan-validationlayers-dev spirv-tools
    wget https://storage.googleapis.com/shaderc/artifacts/prod/graphics_shader_compiler/shaderc/linux/continuous_clang_release/453/20240430-112351/install.tgz
    tar -xzf install.tgz
    mv install/bin/glslc /usr/local/bin
    if ! command -v glslc &> /dev/null; then
        echo "glslc installation failed... Setup terminated"
        exit 1
    fi
    # if ! command -v vkcube &> /dev/null; then
    #     echo "Vulkan SDK installation failed... Setup terminated"
    #     exit 1
    # fi
else
    echo "Vulkan SDK installed"
fi

echo "Checking make installation..."
if ! command -v make &> /dev/null; then
    echo "make installation not found. Installing make..."
    apt-get install -y make
    
    if ! command -v make &> /dev/null; then
        echo "make installation failed... Setup terminated"
        exit 1
    fi

else
    echo "make installed"
fi

SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
ROOT="$DIR/.."

echo "Compiling shaders..."
apt-get install -y libc6
"$ROOT"/lynx/scripts/unix-compile-shaders.sh
if ! mycmd
then
    echo "Shader compilation failed... Setup terminated"
    exit 1
fi

echo "Generating build files..."
python3 "$DIR/src/generate_build_files.py"
if ! mycmd
then
    echo "Build files generation failed... Setup terminated"
    exit 1
fi

echo "Run generate_build_files.py with python to have further details on how the project can be built. Use the -h flag to display the options available."

echo "Setup completed successfully!"
