#! /usr/bin/python

import sys

import json

sys.path.append("/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/")
import yaml

all = {}
def _add(d):
    name = d['module']

    if name in all:
        if all[name] is None:
            return
        if d['pri'] == all[name]['pri']:
            print >>sys.stderr, "Warn:", name, "listed twice"
            all[name] = None
            return
        if d['pri'] <= all[name]['pri']:
            return

    all[name] = d

pri = 0
for arg in sys.argv[1:]:
    pri += 1
    for modef in yaml.load_all(open(arg).read()):

        assert modef['document'] == 'modulemd-defaults'
        assert modef['version'] == 1
        # assert modef['data']['module'] is ''

        d = modef['data']

        assert d['stream'] in d['profiles']

        d['pri'] = pri

        _add(d)
        for intn in d.get('intents', []):
            intd = {'module' : intn + '/' + d['module']}
            intd['stream'] = d['intents'][intn]['stream']
            intd['profiles'] = d['intents'][intn]['profiles']
            intd['pri'] = pri
            _add(intd)

for n in sorted(all):
    d = all[n]
    if d is None: continue

    print d['module'], '=>', "%s:%s" % (d['module'], d['stream'])
    for stream in sorted(d['profiles']):
        print " ", "%s:%s" % (d['module'], stream), '=>', d['profiles'][stream]

