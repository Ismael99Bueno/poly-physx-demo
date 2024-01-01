import os

from validation.exceptions import DependencyNotFoundError
from utility.buddy import Buddy
from utility.utils import download_file, unzip_file


def validate_vulkan() -> None:
    print("\n==== VULKAN VALIDATION ====")

    if not __is_vulkan_installed() and not __install_vulkan():
        raise DependencyNotFoundError("The dependency 'Vulkan' was not found")
    print("Vulkan installed")


def __is_vulkan_installed() -> bool:
    return os.environ.get("VULKAN_SDK") is not None


def __install_vulkan() -> bool:
    bud = Buddy()
    if not bud.all_yes and not bud.prompt_to_install("Vulkan"):
        return False

    installer_path = (
        bud.root_path
        / "vendor"
        / "vulkan-sdk"
        / "bin"
        / "VulkanSDK-1.3.250.1-Installer.exe"
    )
    installer_url = "https://sdk.lunarg.com/sdk/download/1.3.250.1/windows/VulkanSDK-1.3.250.1-Installer.exe"
    download_file(installer_url, installer_path)
    print(
        "\Vulkan installer will now be executed. Once the installation finishes, re-run the script"
    )
    input("Press any key to begin installation...")
    os.startfile(installer_path)
    quit()
