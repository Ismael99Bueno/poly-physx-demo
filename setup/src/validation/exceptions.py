class PathNotFoundError(Exception):
    pass


class UnrecognizedWhichArgumentError(Exception):
    pass


class PackageNotFoundError(Exception):
    pass


class DependencyNotFoundError(Exception):
    pass


class PythonVersionError(Exception):
    pass


class BadOSError(Exception):
    pass


class GeneratorNotSupportedError(Exception):
    pass
