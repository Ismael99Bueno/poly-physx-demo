import platform
import subprocess


def main() -> None:
    plf = platform.system()
    if plf == "Darwin":
        subprocess.run("./setup-macos.sh")
    elif plf == "Windows":
        print("Windows support is still under development")
    elif plf == "Linux":
        print("poly-physx-demo does not support Linux :(")
    else:
        print(f"Unrecognized {plf} platform")


if __name__ == "__main__":
    main()
