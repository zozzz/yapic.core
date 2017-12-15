import pytest
from operator import index
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


def test_types_number_proto():
    def inplace(op):
        l = {"np": _types.NumberP()}
        c = compile("np %s 1" % op, "<inplace %s>" % op, "single")
        exec(c, {}, l)
        return l["np"]

    np = _types.NumberP()

    assert bool(np) is False
    assert np + 1 == 1
    assert np - 1 == 2
    assert np * 1 == 3
    assert np % 1 == 4
    assert divmod(np, 1) == 5
    assert np ** 1 == 6
    assert -np == 7
    assert +np == 8
    assert abs(np) == 9
    assert ~np == 10
    assert np << 1 == 11
    assert np >> 1 == 12
    assert np & 1 == 13
    assert np ^ 1 == 14
    assert np | 1 == 15
    assert int(np) == 16
    assert float(np) == 17.0
    assert inplace("+=") == 18
    assert inplace("-=") == 19
    assert inplace("*=") == 20
    assert inplace("%=") == 21
    assert inplace("**=") == 22
    assert inplace("<<=") == 23
    assert inplace(">>=") == 24
    assert inplace("&=") == 25
    assert inplace("^=") == 26
    assert inplace("|=") == 27
    assert np // 1 == 28
    assert np / 1 == 29
    assert inplace("//=") == 30
    assert inplace("/=") == 31
    assert inplace("/=") == 31
    assert index(np) == 32
    assert np @ 1 == 33
    assert inplace("@=") == 34


def test_types_no_subclass():
    with pytest.raises(TypeError) as exc:
        class Sub(_types.A):
            pass

    exc.match("type 'A' is not an acceptable base type")


def test_types_yes_subclass():
    class Sub(_types.Base):
        pass

    assert isinstance(Sub(), _types.Base)
    assert issubclass(Sub, _types.Base)


def test_types_freelist():
    x = []
    for i in range(0, 20):
        t = _types.FR()
        assert isinstance(t, _types.FR)
        x.append(t)

    for fr in x:
        assert isinstance(fr, _types.FR)

    del x

    for i in range(0, 20):
        t = _types.FR()
        assert isinstance(t, _types.FR)


