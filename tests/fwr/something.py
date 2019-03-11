from typing import Generic, TypeVar


class SomethingNormal:
    pass


ST = TypeVar("ST")


class FWR(Generic[ST]):
    fwr: ST


class SomethingGeneric(Generic[ST]):
    sga: ST
    forward: "SomethingNormal"
    forward_generic: FWR["SomethingNormal"]
    forward_generic2: "FWR[ST]"

    def __init__(self, sg_in_init: ST):
        self.sg_init = sg_in_init
