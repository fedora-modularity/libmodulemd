#! /usr/bin/python

import sys

import json

sys.path.append("/opt/local/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages/")
import yaml

all = {}
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

        if d['module'] in all:
            if all[d['module']] is None:
                continue
            if d['pri'] == all[d['module']]['pri']:
                print >>sys.stderr, "Warn:", d['module'], "listed twice"
                all[d['module']] = None
                continue
            if d['pri'] <= all[d['module']]['pri']:
                continue

        all[d['module']] = d

for n in sorted(all):
    d = all[n]
    if d is None: continue

    print d['module'], '=>', "%s:%s" % (d['module'], d['stream'])
    for stream in sorted(d['profiles']):
        print " ", "%s:%s" % (d['module'], stream), '=>', d['profiles'][stream]

