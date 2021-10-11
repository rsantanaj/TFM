# -*- coding: utf-8 -*-

from distutils.core import setup 
import py2exe 
 
setup(name="main", 
 version="1.0", 
 
 console=[{"script": "main.py"}]
)