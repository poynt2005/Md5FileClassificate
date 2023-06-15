import os
import subprocess
import shutil

msvc2019_tool_path = os.path.join(os.environ['ProgramFiles(x86)'],
                                  'Microsoft Visual Studio', '2019', 'Professional', 'Common7', 'Tools')

os.environ['PATH'] += ';%s' % msvc2019_tool_path


def create_process(cmd, cwd):
    sb = subprocess.Popen(
        cmd,
        shell=True,
        cwd=cwd,
        stdout=subprocess.PIPE
    )
    sb.wait()
    print(sb.stdout.read().decode('utf-8', 'ignore'))


def run_msbuild(directory):
    sb = subprocess.Popen(
        ['cmd.exe'],
        cwd=directory,
        shell=True,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE
    )

    sb.stdin.write('VsDevCmd\r\n'.encode('utf-8'))
    sb.stdin.flush()
    sb.stdin.write(
        'MSBuild /property:Configuration=Release\r\n'.encode('utf-8'))
    sb.stdin.flush()
    sb.communicate()


def build_with_make(content_dir, label):
    create_process(['mingw32-make', label], content_dir)


def build_with_npm(content_dir):
    if not os.path.exists(os.path.join(content_dir, 'node_modules')):
        create_process(['npm', 'i', '--ignore-scripts'], content_dir)
    create_process(['npm', 'run', 'build-native'], content_dir)


print('Build Md5 lib')
build_with_make(os.getcwd(), 'dll')
build_with_make(os.path.join(os.getcwd(), 'Python'), 'pyd')

print('Build Classificator lib')
build_with_make(os.path.join(os.getcwd(), 'Classificator'), 'dll')
build_with_make(os.path.join(os.getcwd(), 'Classificator', 'Cs'), 'dll')
build_with_make(os.path.join(os.getcwd(), 'Classificator', 'Python'), 'pyd')
build_with_make(os.path.join(
    os.getcwd(), 'Classificator', 'SampleFileGen'), 'rc')
build_with_make(os.path.join(
    os.getcwd(), 'Classificator', 'SampleFileGen'), 'dll')
build_with_npm(os.path.join(
    os.getcwd(), 'Classificator', 'Node'))
run_msbuild(os.path.join(
    os.getcwd(), 'Classificator', 'GUI', 'WindowsFormsApp1'))
shutil.copy(os.path.join(os.getcwd(), 'Classificator', 'GUI', 'WindowsFormsApp1', 'WindowsFormsApp1', 'bin',
            'Release', 'WindowsFormsApp1.dll'), os.path.join(os.getcwd(), 'Classificator', 'GUI', 'Form.dll'))
