#!/usr/bin/python
#coding:utf-8
import sys,os
import urllib
query = os.getenv('QUERY_STRING')

if len(query):
    print ("Content-type:text/html\n")
    print ("<!DOCTYPE html>")
    print ('<html>')
    print ('<head>')
    print ('<title>GET</title>')
    print ('</head>')
    print ('<body>')
    print ('<h2> Your GET data: </h2>')
    print ('<ul>')
    for data in query.split('&'):
        print  ('<li>'+data+'</li>')
    print ('</ul>')
    print ('</body>')
    print ('</html>')
else:
    print ("Content-type:text/html\n")
    print ('no found')

sys.stderr.close()
