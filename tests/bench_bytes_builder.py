import pytest
from yapic.cpp.test import _string_builder

UTF8_STR = "Árvíztűrő tükörfúrógép"
UTF8_4B = "𐌀𐌁𐌂𐌃𐌄𐌅𐌆𐌇𐌈𐌉𐌊𐌋𐌌𐌍𐌏𐌐𐌑𐌒𐌓𐌔𐌕𐌖𐌗𐌘𐌙𐌚𐌛𐌜𐌝𐌞𐌠𐌡𐌢𐌣"
MIXED = UTF8_STR + "Some ascii string..." + UTF8_4B
ASCII = "QWERTZUIOPasdfghjklYXCVBNM"
PARAMS = [
    UTF8_STR,
    UTF8_STR * 1000,
    UTF8_4B,
    UTF8_4B * 1000,
    MIXED,
    MIXED * 500,
    ASCII * 1000
]
IDS = [
    "UTF8_STR",
    "UTF8_STR * 1000",
    "UTF8_4B",
    "UTF8_4B * 1000",
    "MIXED",
    "MIXED * 500",
    "ASCII * 1000"
]


@pytest.mark.parametrize("data", PARAMS, ids=IDS)
def test_py_utf8_to_bytes(benchmark, data):
    benchmark.pedantic(lambda s: (s + "1").encode("utf-8"), args=[data], iterations=100, rounds=100)


@pytest.mark.parametrize("data", PARAMS, ids=IDS)
def test_yapic_utf8_to_bytes(benchmark, data):
    benchmark.pedantic(lambda s: _string_builder.unicode_to_bytes(s + "1"), args=[data], iterations=100, rounds=100)
