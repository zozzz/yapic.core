import pytest


def test_bad_module():
    with pytest.raises(SystemError) as ex:
        from yapic.cpp.test import _bad_module


def test_bad_module2():
    with pytest.raises(Exception) as ex:
        from yapic.cpp.test import _bad_module2

    assert ex.type.__module__ == "yapic.cpp.test._bad_module2"
    assert ex.type.__name__ == "BadError"

    ex.match("Somthing is wrong")


def test_bad_import():
    with pytest.raises(ModuleNotFoundError) as ex:
        from yapic.cpp.test import _bad_import

    ex.match("No module named 'something'")


def test_bad_import2():
    with pytest.raises(AttributeError) as ex:
        from yapic.cpp.test import _bad_import2

    ex.match("module 'os' has no attribute 'wrong'")
