from building import *

cwd     = GetCurrentDir()
CPPPATH = [
    cwd,
    cwd+'ssl_if/wolfssl'
]
src     = Glob('*.c')

if GetDepend('NET_SERVER_TCP_EXAMPLE'):
    src = src + ['examples/simple_tcp_server.c']

if GetDepend('NET_SERVER_TLS_EXAMPLE'):
    src = src + ['examples/simple_ssl_server.c']

if GetDepend('NET_SERVER_WOLFSSL'):
     src = src + ['ssl_if/wolfssl/ns_ssl_if.c']

group = DefineGroup('net-server', src, depend = ['PKG_USING_NET_SERVER'], CPPPATH = CPPPATH)

Return('group')
