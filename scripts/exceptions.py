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
