import pytest
from yapic.core.test import _module


def test_basics():
    assert _module.__name__ == "yapic.core.test._module"
    assert _module.XYZ == 42
    assert _module.do_something() == 111
    assert _module.get_state_var() == "__code__"


def test_exception():
    with pytest.raises(_module.SomeError):
        raise _module.SomeError()

    with pytest.raises(_module.SomeError) as ex:
        _module.raise_error()

    ex.match(r"^raised\.\.\.$")

    with pytest.raises(_module.SomeError) as ex:
        _module.raise_error_formatted()

    ex.match(r"^raised\.\.\. 4$")


def test_import():
    from yapic.core.test import _import
    from os import path

    assert _import.path_join("a", "b", "c") == path.join("a", "b", "c")
