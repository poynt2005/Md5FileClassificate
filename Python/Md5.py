import Md5Module

Md5Module.LoadLibrary()


def md5_hash(input_string: str) -> str:
    return Md5Module.Md5Hash(input_string)


def md5_file_hash(input_filepath: str) -> str:
    return Md5Module.Md5FileHash(input_filepath)
