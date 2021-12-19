from building import *

cwd     = GetCurrentDir()
CPPPATH = [
    cwd,
    cwd+'ssl_if/wolfssl'
]
src     = Glob('*.c')

if GetDepend('NET_SERVER_USING_SAMPLE'):
    src = src + ['examples/simple_tcp_server.c']

if GetDepend('NET_SERVER_USING_SSL_SAMPLE'):
    src = src + ['examples/simple_ssl_server.c']

group = DefineGroup('net-server', src, depend = ['PKG_USING_NET_SERVER'], CPPPATH = CPPPATH)

Return('group')
