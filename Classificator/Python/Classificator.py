import ClassificatorModule
from typing import Callable, List

ClassificatorModule.LoadLibrary()


def Classificate(source_dir: str, duplicate_dest_dir: str = None, teller: Callable = None) -> List:
    def __teller(current: int, total: int, is_dup: int, filename: str, new_filename: str):
        filename_decode = filename.decode('utf-8', 'ignore')
        new_filename_decode = None

        if new_filename:
            new_filename_decode = new_filename.decode('utf-8', 'ignore')

        if callable(teller):
            teller(current, total, [False, True][is_dup],
                   filename_decode, new_filename_decode)

    duplicated_files = ClassificatorModule.Classificator(
        source_dir, duplicate_dest_dir, __teller)

    duplicated_files = [filename.decode(
        'utf-8', 'ignore') for filename in duplicated_files]
    return duplicated_files
