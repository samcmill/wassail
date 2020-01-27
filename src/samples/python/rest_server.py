# Copyright (c) 2018-2020 Scott McMillan <scott.andrew.mcmillan@gmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This sample sets up a REST API for the wassail data building blocks.
# Fresh data is collected for each query, i.e., the wassail cache
# capabilities are not used.
#
# To run this sample in a non-production environment:
# $ FLASK_APP=rest_server.py python -m flask run
#
# Then to query the REST API:
# $ curl localhost:5000/data/getcpuid

from flask import Flask
import json

try:
    import wassail
except ModuleNotFoundError:
    # If the wassail module is installed in a non-default location,
    # you may need to set the PYTHONPATH environment variable to that
    # location.  Also use the same python version the module was built
    # for, typically the latest version of python present on the
    # system.
    print('Unable to load the wassail module')
    exit(1)

app = Flask(__name__)

def data(d):
    """Generic wassail data building block wrapper"""
    try:
        d.evaluate()
        j = json.loads(str(d))
        return(j)
    except RuntimeError as e:
        return {'error': str(e)}

@app.route('/data/environment')
def environment():
  d = wassail.data.environment()
  return data(d)

@app.route('/data/getcpuid')
def getcpuid():
  d = wassail.data.getcpuid()
  return data(d)

@app.route('/data/getfsstat')
def getfsstat():
  d = wassail.data.getfsstat()
  return data(d)

@app.route('/data/getloadavg')
def getloadavg():
  d = wassail.data.getloadavg()
  return data(d)

@app.route('/data/getmntent')
def getmntent():
  d = wassail.data.getmntent()
  return data(d)

@app.route('/data/getrlimit')
def getrlimit():
  d = wassail.data.getrlimit()
  return data(d)

@app.route('/data/pciaccess')
def pciaccess():
  d = wassail.data.pciaccess()
  return data(d)

@app.route('/data/pciutils')
def pciutils():
  d = wassail.data.pciutils()
  return data(d)

@app.route('/data/ps')
def ps():
  d = wassail.data.ps()
  return data(d)

@app.route('/data/stream')
def stream():
  d = wassail.data.stream()
  return data(d)

@app.route('/data/sysconf')
def sysconf():
  d = wassail.data.sysconf()
  return data(d)

@app.route('/data/sysctl')
def sysctl():
  d = wassail.data.sysctl()
  return data(d)

@app.route('/data/sysinfo')
def sysinfo():
  d = wassail.data.sysinfo()
  return data(d)

@app.route('/data/umad')
def umad():
  d = wassail.data.umad()
  return data(d)

@app.route('/data/uname')
def uname():
  d = wassail.data.uname()
  return data(d)
