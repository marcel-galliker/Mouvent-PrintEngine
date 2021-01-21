from distutils.core import setup
import py2exe
import os, sys; sys.path.append(os.path.dirname(os.path.realpath(__file__)))

setup(console=['simulate.py'])