#!/bin/bash

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
fi
echo "Python3 installed
"

echo "Checking CMake intallation..."
if ! which -s cmake
then
    echo "CMake installation not found. Do you wish to install? [Y]/N"
    read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "CMake installation failed... Setup terminated"
        exit
    fi
    if ! install_brew
    then
        echo "Homebrew installation failed... Setup terminated"
        exit
    fi

    brew install cmake
fi
echo "CMake installed
"

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
fi
echo "make installed
"

echo "Do you want to go ahead and generate the project build files? [Y]/N"
read -r ANSWER
    if [[ "$ANSWER" != "Y" && "$ANSWER" != "y" && -n "$ANSWER" ]]
    then
        echo "Run generate_build_files.py to generate the project's build file. Use the -h flag to display the options available"
        exit
    fi

SOURCE=${BASH_SOURCE[0]}
while [ -L "$SOURCE" ]; do # resolve $SOURCE until the file is no longer a symlink
  DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )
  SOURCE=$(readlink "$SOURCE")
  [[ $SOURCE != /* ]] && SOURCE=$DIR/$SOURCE # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
done
DIR=$( cd -P "$( dirname "$SOURCE" )" >/dev/null 2>&1 && pwd )

python3 "$DIR/generate_build_files.py" "$DIR/.."