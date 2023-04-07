class RootPathNotFoundError(Exception):
    pass


class SFMLPathNotFoundError(Exception):
    pass


class UnrecognizedWhichArgumentError(Exception):
    pass


class PackageNotFoundError(Exception):
    def __init__(self, package: str) -> None:
        self.__package = package
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Package {self.__package} not found"


class PythonVersionError(Exception):
    def __init__(self, required: str, current: str) -> None:
        self.__required = required
        self.__current = current
        super().__init__(self.__repr__())

    def __repr__(self) -> str:
        return f"Minimum python version required is {self.__required}, but {self.__current} found. Consider upgrading to {self.__required}"
