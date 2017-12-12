import pytest
from yapic.core.test import _types


def test_types_basic():
    assert isinstance(_types.A, type)
    assert isinstance(_types.B, type)

    assert isinstance(_types.A(), _types.A)
    assert isinstance(_types.B(), _types.A)
    assert isinstance(_types.B(), _types.B)

    assert _types.A.__name__ == "A"
    assert _types.B.__name__ == "B"
    assert _types.S.__name__ == "S"


def test_types_mapping():
    b = _types.B()

    assert len(b) == 42
    assert b["hello"] == "world"
    b["hello"] = "OK"
    assert b["hello"] == "OK"


def test_types_sequence():
    s = _types.S()

    assert len(s) == 33
