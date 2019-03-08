import typing
from yapic.core.test import _typing


def test_get_typing():
    assert _typing.get_typing() is typing
