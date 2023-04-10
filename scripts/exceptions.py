class PathNotFoundError(Exception):
    def __init__(self, path: str) -> None:
        self.__path = path
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"{self.__path} not found. The project's folder structure may have been changed, or the repository may have not been cloned recursively"


class UnrecognizedWhichArgumentError(Exception):
    def __init__(self, arg: str) -> None:
        self.__arg = arg
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Unrecognized --which argument '{self.__arg}'. Expected 'all', 'ppx' or 'sfml'."


class PackageNotFoundError(Exception):
    def __init__(self, package: str) -> None:
        self.__package = package
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Package {self.__package} not found"


class DependencyNotFoundError(Exception):
    def __init__(self, dependency: str) -> None:
        self.__dependency = dependency
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Dependency {self.__dependency} not found"


class PythonVersionError(Exception):
    def __init__(self, required: str, current: str) -> None:
        self.__required = required
        self.__current = current
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Minimum python version required is {self.__required}, but {self.__current} found. Consider upgrading to {self.__required}"


class BadOSError(Exception):
    def __init__(self, bad: str, good: str) -> None:
        self.__bad = bad
        self.__good = good
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Cannot run setup because your machine does not have the required OS! Required: {self.__good}, current: {self.__bad}"


class GeneratorNotSupportedError(Exception):
    def __init__(self, generator: str) -> None:
        self.__generator = generator
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Generator {self.__generator} is not supported for the current operating system"


class InstallationFailedError(Exception):
    def __init__(self, thing_installed: str, error: str) -> None:
        self.__thing_installed = thing_installed
        self.__error = error
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Installation for {self.__thing_installed} failed with the following error: {self.__error}"
