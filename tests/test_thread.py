import time
import random
import threading

from yapic.core.test import _thread


def test_thread_basic():
    _thread.set("A", 1)
    assert _thread.get("A") == 1


def test_thread_t():
    def x(v):
        time.sleep(random.random() * 1.0)
        _thread.set("A", v)
        assert _thread.get("A") == v

    def y(v):
        time.sleep(0.5)
        _thread.set("A", v)
        assert _thread.get("A") == v

    threads = []
    for i in range(10):
        t = threading.Thread(target=x, args=(i,))
        threads.append(t)
        t.start()

        t = threading.Thread(target=y, args=(i + 200,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()


def test_thread_rt():
    def x(v):
        time.sleep(random.random() * 1.0)
        _thread.rset("A", v)
        assert _thread.rget("A") == v

    def y(v):
        time.sleep(0.5)
        _thread.rset("A", v)
        assert _thread.rget("A") == v

    threads = []
    for i in range(10):
        t = threading.Thread(target=x, args=(i,))
        threads.append(t)
        t.start()

        t = threading.Thread(target=y, args=(i + 200,))
        threads.append(t)
        t.start()

    for t in threads:
        t.join()
