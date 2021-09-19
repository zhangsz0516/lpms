import os
import rtconfig
from building import *

cwd = GetCurrentDir()

# add general drivers
src = Split('''
lpms.c
lpms_notify.c
lpms_drv.c
lpms_tim.c
''')

path =  [cwd]

group = DefineGroup('lpms', src, depend = ['LPMS_ENABLE'], CPPPATH = path)

Return('group')
