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

    (cls, attrs, init) = _typing.type_hints(GenericForward["FwTest"])
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
    ("a", ((("a", None), ), None)),
    ("a: A", ((("a", A), ), None)),
    ("a: A, b: FwTest", ((("a", A), ("b", FwTest)), None)),
    ("a: A = 'AD'", ((("a", A, "AD"), ), None)),
    ("a: A = 'AD', b: FwTest = 'BD'", ((("a", A, "AD"), ("b", FwTest, "BD")), None)),
    ("a: A, b: FwTest = 'BD'", ((("a", A), ("b", FwTest, "BD")), None)),
    ("*, kw1", (None, (("kw1", None), ))),
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


def test_callable_hints_class():
    class Basic:
        @classmethod
        def cls_method(cls, a: A):
            pass

        @staticmethod
        def static_method(a: A):
            pass

        def method(self, a: A):
            pass

    hints = _typing.callable_hints(Basic.cls_method)
    assert hints == ((("a", A), ), None)

    hints = _typing.callable_hints(Basic.static_method)
    assert hints == ((("a", A), ), None)

    hints = _typing.callable_hints(Basic.method)
    assert hints == ((("self", None), ("a", A)), None)

    hints = _typing.callable_hints_with_type(Basic.method, Basic)
    assert hints == ((("a", A), ), None)

    class Callable:
        def __call__(self, a: A):
            pass

    class Callable2(typing.Generic[T]):
        def __call__(self, a: T):
            pass

    hints = _typing.callable_hints(Callable())
    assert hints == ((("a", A), ), None)

    hints = _typing.callable_hints(Callable2[FwTest]())
    assert hints == ((("a", FwTest), ), None)

    with pytest.raises(TypeError) as exc:
        hints = _typing.callable_hints(_typing.callable_hints)
    assert "Cannot get type hints from built / c-extension method" in str(exc.value)


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

    (cls, attrs, init) = _typing.type_hints(A[FwTest])
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

    (cls, attrs, init) = _typing.type_hints(A["FwTest"])
    assert cls is A
    fwd_resolved = attrs["a"]()
    assert fwd_resolved is FwTest
    fwd_resolved = attrs["a_forward"]()
    assert fwd_resolved.__args__[0] is FwTest

    (cls, attrs, init) = _typing.type_hints(B)
    assert cls is B
    assert attrs is None
    assert init is None

    (cls, attrs, init) = _typing.type_hints(C[B])
    assert cls is C
    assert attrs["b"] is B
    assert attrs["c"] == A[B]

    (cls, attrs, init) = _typing.type_hints(D["FwTest"])
    assert cls is D
    assert attrs["d"]() == SomethingGeneric[FwTest]
    assert attrs["d2"]() == SomethingGeneric[FwTest]

    (cls, attrs, init) = _typing.type_hints(attrs["d2"]())
    assert cls is SomethingGeneric
    assert attrs["sga"] is FwTest
    assert attrs["forward"]() is SomethingNormal
    assert attrs["forward_generic"]() == FWR[SomethingNormal]
    assert attrs["forward_generic2"]() == FWR[FwTest]


def test_class_hints_deep_generic():
    T = typing.TypeVar("T")

    class A(typing.Generic[T]):
        a: T

    class B(typing.Generic[T]):
        b: A[T]

    class C(typing.Generic[T]):
        c: B[T]
        c2: A[B[T]]

    class D(typing.Generic[T]):
        d: C[B[A[T]]]

    (cls, attrs, init) = _typing.type_hints(A[FwTest])
    assert cls is A
    assert attrs["a"] is FwTest

    (cls, attrs, init) = _typing.type_hints(B[FwTest])
    assert cls is B
    assert attrs["b"] == A[FwTest]

    (cls, attrs, init) = _typing.type_hints(C[FwTest])
    assert cls is C
    assert attrs["c"] == B[FwTest]
    assert attrs["c2"] == A[B[FwTest]]

    (cls, attrs, init) = _typing.type_hints(D[FwTest])
    assert cls is D
    assert attrs["d"] == C[B[A[FwTest]]]


def test_class_hints_deep_generic2():
    TA = typing.TypeVar("TA")
    TB = typing.TypeVar("TB")
    TC = typing.TypeVar("TC")
    TD = typing.TypeVar("TD")

    class A(typing.Generic[TA]):
        a: TA

    class B(typing.Generic[TB]):
        b: A[TB]

    class C(typing.Generic[TC]):
        c: B[TC]
        c2: A[B[TC]]

    class D(typing.Generic[TD]):
        d: C[B[A[TD]]]

    (cls, attrs, init) = _typing.type_hints(A[FwTest])
    assert cls is A
    assert attrs["a"] is FwTest

    (cls, attrs, init) = _typing.type_hints(B[FwTest])
    assert cls is B
    assert attrs["b"] == A[FwTest]

    (cls, attrs, init) = _typing.type_hints(C[FwTest])
    assert cls is C
    assert attrs["c"] == B[FwTest]
    assert attrs["c2"] == A[B[FwTest]]

    (cls, attrs, init) = _typing.type_hints(D[FwTest])
    assert cls is D
    assert attrs["d"] == C[B[A[FwTest]]]


def test_class_hints_deep_generic3():
    Impl = typing.TypeVar("Impl")
    T = typing.TypeVar("T")
    JoinedT = typing.TypeVar("JoinedT")

    class Relation(typing.Generic[Impl, T]):
        def __init__(self, joined: Impl):
            pass

    class OneToMany(typing.Generic[JoinedT]):
        def __init__(self, joined: JoinedT):
            pass

    class One(typing.Generic[JoinedT], Relation[OneToMany[JoinedT], JoinedT]):
        x: OneToMany[JoinedT]

    (cls, attrs, init) = _typing.type_hints(One[FwTest])

    assert cls is One
    assert attrs["x"] == OneToMany[FwTest]
    assert init[0][0][1] == OneToMany[FwTest]


def test_forward_decl():
    T = typing.TypeVar("T")
    T2 = typing.TypeVar("T2")

    class A(typing.Generic[T]):
        a: T

    (cls, attrs, init) = _typing.type_hints(A["FwTest"])
    assert cls is A
    assert _typing.is_forward_decl(attrs["a"])
    assert repr(attrs["a"]) == "<ForwardDecl 'FwTest'>"

    class B(typing.Generic[T, T2]):
        b: T
        b2: T2

    (cls, attrs, init) = _typing.type_hints(B["FwTest", A["FwTest"]])
    assert cls is B
    assert _typing.is_forward_decl(attrs["b"])
    assert _typing.is_forward_decl(attrs["b2"])
    assert attrs["b"]() == FwTest
    assert attrs["b2"]() == A[FwTest]

    class C(typing.Generic[T]):
        def __init__(self, a: B[T, A[T]]):
            pass

    (cls, attrs, init) = _typing.type_hints(C["FwTest"])
    arg = init[0][0][1]
    assert cls is C
    assert _typing.is_forward_decl(arg)
    assert arg() == B[FwTest, A[FwTest]]

    unpacked = _typing.unpack_forward_decl(arg)
    assert unpacked[0] is B
    assert _typing.is_forward_decl(unpacked[1][0])
    assert _typing.is_forward_decl(unpacked[1][1])
    assert unpacked[1][0]() == FwTest
    assert unpacked[1][1]() == A[FwTest]


def test_type_alias_inheritance():
    Impl = typing.TypeVar("Impl")
    PyType = typing.TypeVar("PyType")
    RawType = typing.TypeVar("RawType")

    class Field(typing.Generic[Impl, PyType, RawType]):
        __impl__: Impl

        def __init__(self, impl: Impl):
            pass

    class IntImpl:
        pass

    Int = Field[IntImpl, int, int]

    class Serial(Int):
        pass

    (int_cls, int_attr, int_init) = _typing.type_hints(Int)
    (serial_cls, serial_attr, serial_init) = _typing.type_hints(Serial)

    assert int_cls is Field
    assert serial_cls is Serial

    assert int_attr == serial_attr
    assert int_init == serial_init


def test_parametrized_type_inheritance():
    Impl = typing.TypeVar("Impl")
    PyType = typing.TypeVar("PyType")
    RawType = typing.TypeVar("RawType")

    class Field(typing.Generic[Impl, PyType, RawType]):
        __impl__: Impl

        def __init__(self, impl: Impl):
            pass

    class IntImpl:
        pass

    class Int(Field[IntImpl, int, int]):
        pass

    class Serial(Int):
        pass

    (int_cls, int_attr, int_init) = _typing.type_hints(Int)
    (serial_cls, serial_attr, serial_init) = _typing.type_hints(Serial)

    assert int_cls is Int
    assert serial_cls is Serial

    assert int_attr == serial_attr
    assert int_init == serial_init


def test_builtin_class():
    (cls, attr, init) = _typing.type_hints(_typing.BuiltinClass)

    assert cls is _typing.BuiltinClass
    assert attr is None
    assert init is None
