from os import path


def get_include_path() -> str:
    return path.join(path.dirname(path.abspath(__file__)), "include")


def get_library_path() -> str:
    raise NotImplementedError()
