import io
import sys

import pydpapi


class TestModule:
    def test_module_docstring_set(self):
        assert pydpapi.__doc__ is not None
        assert pydpapi.__doc__ != ""


class TestEncrypt:
    def test_empty_array(self):
        s = bytearray()
        pydpapi.encrypt(s)

    def test_sets_input_string_to_null(self):
        s = bytearray(b"Hello World")
        pydpapi.encrypt(s)
        assert s != bytearray(b"Hello World")

    def test_cipher_text_is_not_plaintext(self):
        s = bytearray(b"Hello World")
        e = pydpapi.encrypt(s)
        assert e != bytearray(b"Hello World")


class TestIntegration:
    def test_encrypt_then_decrypt(self):
        s = bytearray(b"Hello World")
        e = pydpapi.encrypt(s)
        t = pydpapi.decrypt(e)
        assert t == bytearray(b"Hello World")
