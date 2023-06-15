import glob
import shutil
import os

f = None
if os.path.exists('.gitignore'):
    os.remove('.gitignore')

f = open('.gitignore', 'w', encoding='utf-8')


bin_file_exts = [
    'dll',
    'exp',
    'obj',
    'o',
    'exe',
    'zip',
    'node',
    'pch',
    'lib',
    'pdb',
    'bak'
]

for ext in bin_file_exts:
    if not f is None:
        f.write('*.%s\n' % ext)
    globbed_files = glob.glob('**/*.%s' % ext, recursive=True)
    for file in globbed_files:
        try:
            os.remove(file)
        except:
            print('File: %s remove error' % file)

dirs = [
    'node_modules',
    'GenPics',
    'rst',
    '__pycache__',
    '.vs',
    'obj',
    'bin',
    'x64',
    'Debug'
]

for dir in dirs:
    for sub_dir in glob.glob('**/*/%s' % dir, recursive=True):
        try:
            shutil.rmtree(sub_dir)
        except:
            print('Directory: %s remove error' % sub_dir)

    if not f is None:
        f.write('%s\n' % dir)


if not f is None:
    f.close()
