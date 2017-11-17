import pytest
from yapic.core.test import _string_builder


@pytest.mark.parametrize("value", [
    "Hello World",
    "Árvíz",
    "VeryLong" * 100
])
def test_ascii(value):
    assert _string_builder.ascii_builder(value) == f"@Y@{value}@X@"


def test_ascii_bad():
    with pytest.raises(UnicodeError) as ex:
        _string_builder.ascii_builder("ერთადერთი")

    ex.match("The given string must be ascii encoded.")


@pytest.mark.parametrize("value", [
    "Hello World",
    "Árvíz",
    "VeryLong" * 100,
    "ერთადერთი",
    "ერთადერთი" * 100
])
def test_unicode(value):
    assert _string_builder.unicode_builder(value) == f"@Y@{value}@X@"


@pytest.mark.parametrize("value", [
    "Hello World",
    "Árvíz".encode(),
    "VeryLong".encode() * 100,
    "ერთადერთი".encode(),
    "ერთადერთი".encode() * 100,
    "ერთადერთი",
])
def test_bytes(value):
    assert _string_builder.bytes_builder(value) \
        == "@Y@".encode() + (value.encode() if isinstance(value, str) else value) + "@X@☀".encode()
