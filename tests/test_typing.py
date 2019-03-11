import typing
import inspect
from yapic.core.test import _typing
from fwr import SomethingNormal, SomethingGeneric, FWR

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


def test_resolve_type_vars():
    class B:
        pass

    AB = A[B]

    resolved = _typing.resolve_type_vars(AB)
    assert resolved[T] is B


class FwTest:
    pass


class GenericForward(typing.Generic[T]):
    fwd: "A[T]"


def test_resolve_type_forward_ref():
    AB = A["FwTest"]

    resolved = _typing.resolve_type_vars(AB)
    assert resolved[T]() is FwTest

    ASN = A["SomethingNormal"]
    resolved = _typing.resolve_type_vars(ASN)
    assert resolved[T]() is SomethingNormal

    ASG = A["SomethingGeneric[T]"]
    resolved = _typing.resolve_type_vars(ASG)
    assert _typing.is_generic_type(resolved[T]())

    GFW = GenericForward["FwTest"]
    resolved = _typing.resolve_type_vars(GFW)
    rt = resolved[T]()
    assert rt is FwTest

    (attrs, init) = _typing.class_hints(GenericForward["FwTest"])
    resolved = attrs["fwd"]()
    assert resolved == A[FwTest]

    resolved = _typing.resolve_type_vars(A["SomethingGeneric[FwTest]"])
    rt = resolved[T]()
    assert rt == SomethingGeneric[FwTest]

    # TODO: ...
    # (attrs, init) = _typing.class_hints(SomethingGeneric["FwTest"])
    # print(attrs["sga"]())


def test_resolve_mro_1():
    TA = typing.TypeVar("TA")
    TB = typing.TypeVar("TB")

    class A(typing.Generic[T]):
        a: T

    class B(A[TA], typing.Generic[T, TA]):
        b: T

    class C(typing.Generic[T, TA, TB], B[TB, TA]):
        c: T

    class IA:
        pass

    class IB:
        pass

    class IC:
        pass

    resolved = _typing.resolve_mro(C[IC, IA, IB])

    # for c, g, v in resolved:
    #     print(c, g)
    #     print("    \t%r" % v)

    assert resolved[0][0] is C
    assert resolved[0][2][T] is IC
    assert resolved[0][2][TA] is IA
    assert resolved[0][2][TB] is IB

    assert resolved[1][0] is B
    assert resolved[1][2][T] is IB
    assert resolved[1][2][TA] is IA

    assert resolved[2][0] is A
    assert resolved[2][2][T] is IA


def test_class_hints():
    class X(typing.Generic[T]):
        x: T

    class A(typing.Generic[T]):
        a: T
        a_forward: X["T"]

    class B:
        pass

    class C(typing.Generic[T]):
        b: B
        c: A[T]

    class D(typing.Generic[T]):
        d: SomethingGeneric[T]
        d2: "SomethingGeneric[T]"

    (attrs, init) = _typing.class_hints(A[FwTest])
    assert attrs["a"] is FwTest
    fwd_resolved = attrs["a_forward"]()
    assert fwd_resolved.__args__[0] is FwTest

    (attrs, init) = _typing.class_hints(A["FwTest"])
    fwd_resolved = attrs["a"]()
    assert fwd_resolved is FwTest
    fwd_resolved = attrs["a_forward"]()
    assert fwd_resolved.__args__[0] is FwTest

    (attrs, init) = _typing.class_hints(C[B])
    assert attrs["b"] is B
    assert attrs["c"] == A[B]

    (attrs, init) = _typing.class_hints(D["FwTest"])
    assert attrs["d"]() == SomethingGeneric[FwTest]
    assert attrs["d2"]() == SomethingGeneric[FwTest]

    (attrs, init) = _typing.class_hints(attrs["d2"]())
    assert attrs["sga"] is FwTest
    assert attrs["forward"]() is SomethingNormal
    assert attrs["forward_generic"]() == FWR[SomethingNormal]
    assert attrs["forward_generic2"]() == FWR[FwTest]
