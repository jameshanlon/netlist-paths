import subprocess, os

# If we're running on Read the Docs' servers, then run Doxygen.
# See https://breathe.readthedocs.io/en/latest/readthedocs.html
# See https://devblogs.microsoft.com/cppblog/clear-functional-c-documentation-with-sphinx-breathe-doxygen-cmake

def configure_doxyfile(input_dir, output_dir):
    print('Configuring Doxyfile in {}'.format(os.getcwd()))
    with open('Doxyfile.in', 'r') as fp:
        filedata = fp.read()
    filedata = filedata.replace('@DOXYGEN_INPUT_DIR@', input_dir)
    filedata = filedata.replace('@DOXYGEN_OUTPUT_DIR@', output_dir)
    with open('Doxyfile', 'w') as fp:
        fp.write(filedata)

breathe_projects = {}

read_the_docs_build = os.environ.get('READTHEDOCS', None) == 'True'

if read_the_docs_build:
    input_dir = '../include/netlist_paths'
    output_dir = 'doxygen'
    configure_doxyfile(input_dir, output_dir)
    print('Running doxygen in {}'.format(os.getcwd()))
    doxygen_output = subprocess.check_output(['doxygen', 'Doxyfile'], shell=True)
    print(doxygen_output.decode('utf-8'))
    breathe_projects['netlist-paths'] = os.path.join(output_dir, 'xml')
    print('Doxygen XML file location: {}'.format(breathe_projects['netlist-paths']))

# Configuration file for the Sphinx documentation builder.
#
# This file only contains a selection of the most common options. For a full
# list see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.join('@CMAKE_BINARY_DIR@', 'lib', 'netlist_paths'))


# -- Project information -----------------------------------------------------

project = 'Netlist paths'
copyright = '2021, James Hanlon'
author = 'James Hanlon'


# -- General configuration ---------------------------------------------------

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = [
  'sphinx.ext.autodoc',
  'breathe',
  'sphinx_rtd_theme',
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = ['_build', 'Thumbs.db', '.DS_Store']


# -- Options for HTML output -------------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = []

# Breathe configuration
breathe_default_project = "netlist-paths"
