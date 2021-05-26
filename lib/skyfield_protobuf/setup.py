# SPDX-FileCopyrightText: 2021 Mario Franke <research@m-franke.net>
#
# SPDX-License-Identifier: GPL-2.0-or-later

# Based on setup.py from protobuf-evi by Dominik S. Buse <buse@ccs-labs.org>

from __future__ import print_function

import glob
import os
import subprocess

from distutils.command.build_py import build_py
from distutils.command.clean import clean
from distutils.command.sdist import sdist
from setuptools import setup
from distutils.spawn import find_executable
from distutils.dir_util import remove_tree


# find protobuf compiler
if 'PROTOC' in os.environ and os.path.exists(os.environ['PROTOC']):
    protoc = os.environ['PROTOC']
else:
    protoc = find_executable("protoc")


protobuf_dir = 'protobuf'
protobuf_package_dir = 'skyfield_protobuf'
init_file_tpl = "from .{subpackage} import {module}\n"


def compile_proto_dir(proto_dir, output_dir, start_init_files_at=0):
    # ensure output_dir exist
    os.makedirs(output_dir, exist_ok=True)
    for dirpath, subdirs, files in os.walk(proto_dir):
        proto_files = [f for f in files if f.endswith('.proto')]
        if proto_files:
            for proto_file in proto_files:
                protoc_call = [protoc,
                               '--proto_path=' + proto_dir,
                               '--python_out=' + output_dir,
                               os.path.join(dirpath, proto_file)]
                print(subprocess.list2cmdline(protoc_call))
                subprocess.check_call(protoc_call)
            if len(os.path.relpath(dirpath, start=proto_dir).split(os.path.sep)) > start_init_files_at:
                init_name = os.path.join(output_dir, os.path.relpath(dirpath, start=proto_dir), '__init__.py')
                print('Creating INIT file:', init_name, 'subdirs:', subdirs)
                with open(init_name, 'w') as f:
                    for subpackage in subdirs:
                        for protofile in glob.glob(os.path.join(dirpath, subpackage, '*.proto')):
                            f.write(init_file_tpl.format(subpackage=subpackage,
                                                         module=os.path.basename(protofile.replace('.proto', '_pb2'))))
            # FIXME: make this generic to work with more arbitrary package structures
            master_file = "skyfield_protobuf/skyfield_protobuf_pb2.py"
            if os.path.exists(master_file):
                replace_calls = [
                    r's/import skyfield_protobuf\.\([a-zA-Z]\+_pb2\)/from skyfield_protobuf import \1 as skyfield_protobuf_\1/',
                    r's/skyfield_protobuf\.\([a-zA-Z]\+_pb2\)/skyfield_protobuf_\1/g'
                ]
                for call in replace_calls:
                    print(subprocess.list2cmdline(['sed', '-i', call, master_file]))
                    subprocess.check_call(['sed', '-i', call, master_file])


# protobuf helper classes
class ProtocBuildPy(build_py):
    def run(self):
        # first generate python files from .proto files
        compile_proto_dir(protobuf_dir, protobuf_package_dir)
        # then continue with the usual build
        build_py.run(self)


class ProtocClean(clean):
    def run(self):
        # first delete generated python files
        remove_tree(protobuf_package_dir)
        # then continue with the usual clean
        clean.run(self)


class ProtoSdist(sdist):
    def run(self):
        print("Hello from sdist")
        # first generate python files from .proto files
        compile_proto_dir(protobuf_dir, protobuf_package_dir)
        # then continue with the usual sdist
        sdist.run(self)


setup(name='skyfield_protobuf_protocol',
      version='0.1.0',
      description='Protocol used by space veins to communicate with skyfield-mobility-server.',
      maintainer='Mario Franke',
      maintainer_email='research@m-franke.net',
      url='https://www.cms-labs.org/people/franke/',
      packages=['skyfield_protobuf', 'skyfield_protobuf.skyfield_protobuf'],
      cmdclass={
          'clean': ProtocClean,
          'build_py': ProtocBuildPy,
          'sdist': ProtoSdist,
      })
