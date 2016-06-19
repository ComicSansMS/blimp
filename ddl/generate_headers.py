#!/usr/bin/env python

from subprocess import check_call

ddl_files = [
    'blimp_properties',
    'file_contents',
    'file_element',
    'indexed_locations',
    'snapshot',
    'snapshot_contents',
    'sqlite_master',
    'user_selection',
];

for f in ddl_files:
    print("Generating headers for " + f + "...")
    check_call('python ../external/sqlpp11/scripts/ddl2cpp -fail-on-parse ' + f + '.ddl ' + f + ' blimpdb')

print("All done.")