#!/bin/bash

rm -rf venv
python3 setup.py setup_venv
source ./venv/bin/activate
pip3 install -r requirements.txt
clear
python3 main.py