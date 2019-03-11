import typing
import inspect
from yapic.core.test import _typing
from fwr import SomethingNormal, SomethingGeneric

T = typing.TypeVar("T")


class A(typing.Generic[T]):
    pass


def test_get_typing():
    assert _typing.get_typing() is typing


def test_is_generic():
    assert _typing.is_generic(A) is False
    assert _typing.is_generic(A.__mro__[1]) is True
    assert _typing.is_generic(typing.Generic[T]) is True
    assert _typing.is_generic(typing.Union[A, None]) is False
    assert _typing.is_generic(A[T]) is False


def test_is_generic_type():
    assert _typing.is_generic_type(A) is False
    assert _typing.is_generic_type(A.__mro__[1]) is False
    assert _typing.is_generic_type(typing.Generic[T]) is False
    assert _typing.is_generic_type(typing.Union[A, None]) is True
    assert _typing.is_generic_type(A[T]) is True


def test_is_forward_ref():
    AX = A["X"]

    assert _typing.is_forward_ref(AX.__args__[0]) is True


def test_resolve_type():
    class B:
        pass

    AB = A[B]

    resolved = _typing.resolve_type(AB)
    assert resolved[T] is B


class FwTest:
    pass


def test_resolve_type_forward_ref():
    AB = A["FwTest"]

    resolved = _typing.resolve_type(AB)
    assert resolved[T] is FwTest

    ASN = A["SomethingNormal"]
    resolved = _typing.resolve_type(ASN)
    assert resolved[T] is SomethingNormal

    ASG = A["SomethingGeneric[T]"]
    resolved = _typing.resolve_type(ASG)
    assert _typing.is_generic_type(resolved[T])

    # TODO: ...
    # X = typing.TypeVar("X")

    # class D(typing.Generic[X]):
    #     d: A["SomethingGeneric[X]"]
