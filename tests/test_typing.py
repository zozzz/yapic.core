import typing, pytest
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


callable_cases = [
    ("a: A", ((("a", A), ), None)),
    ("a: A, b: FwTest", ((("a", A), ("b", FwTest)), None)),
    ("a: A = 'AD'", ((("a", A, "AD"), ), None)),
    ("a: A = 'AD', b: FwTest = 'BD'", ((("a", A, "AD"), ("b", FwTest, "BD")), None)),
    ("a: A, b: FwTest = 'BD'", ((("a", A), ("b", FwTest, "BD")), None)),
    ("*, kw1: A", (None, (("kw1", A), ))),
    ("*, kw1: A, kw2: FwTest", (None, (("kw1", A), ("kw2", FwTest)))),
    ("*, kw1: A = 'KW1D'", (None, (("kw1", A, "KW1D"), ))),
    ("*, kw1: A = 'KW1D', kw2: FwTest = 'KW2D'", (None, (("kw1", A, "KW1D"), ("kw2", FwTest, "KW2D")))),
    ("*, kw1: A, kw2: FwTest = 'KW2D'", (None, (("kw1", A), ("kw2", FwTest, "KW2D")))),
    ("a: A, *, kw1: A", ((("a", A), ), (("kw1", A), ))),
    ("a: A, b: FwTest, *, kw1: A, kw2: FwTest", ((("a", A), ("b", FwTest)), (("kw1", A), ("kw2", FwTest)))),
]


@pytest.mark.parametrize("fnHead,expected", callable_cases, ids=[x[0] for x in callable_cases])
def test_callable_hints_basic(fnHead, expected):
    code = compile(f"""def fn({fnHead}): pass""", "<string>", "exec")
    locals_ = dict(locals())
    exec(code, globals(), locals_)

    hint = _typing.callable_hints(locals_["fn"])
    assert hint == expected


@pytest.mark.parametrize("fnHead,expected", callable_cases, ids=[x[0] for x in callable_cases])
def test_callable_hints_class_init(fnHead, expected):
    if fnHead:
        fnHead = f", {fnHead}"
    code = compile(f"""
class CLASS:
    def __init__(self{fnHead}):
        pass
""", "<string>", "exec")
    locals_ = dict(locals())
    exec(code, globals(), locals_)

    cls = locals_["CLASS"]
    hint = _typing.callable_hints_with_type(cls.__init__, cls)
    assert hint == expected


def test_class_hints():
    class X(typing.Generic[T]):
        x: T

    class A(typing.Generic[T]):
        a: T
        a_forward: X["T"]

        def __init__(self, a_init: T, a_init_fw: X["T"], *, kw: "FwTest"):
            pass

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
    (init_pos, init_kw) = init
    assert len(init_pos) == 2
    assert init_pos[0][0] == "a_init"
    assert init_pos[0][1] is FwTest
    assert init_pos[1][0] == "a_init_fw"
    assert init_pos[1][1]() == X[FwTest]
    assert init_kw[0][0] == "kw"
    assert init_kw[0][1]() is FwTest

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
