'''
Parses log dump from ravens, forming a dictionary of state properties then
writing relevant position and orientation attrs for hand-eye calibration.
Must also have corresponding images in image directory.
'''

import sys
from numpy import pi

XYZ_SCALE = 1000.0
YPR_SCALE = 1000000.0

try:
    raven_log = open(sys.argv[1], 'r')
    out_file = sys.argv[2]
    raven_num = int(sys.argv[3])
except Exception as e:
    print 'usage: python raven_log_parser.py [raven_log] [out_file] [raven_num]'
    print '-- raven_log:\traven log file with attrs on first line'
    print '-- out_file:\toutput file with positions & orientations'
    print '-- raven_num:\traven # [0-3 for ravens in santa cruz]'
    print e
    sys.exit(1)

attrs = raven_log.readline().split(' ')[1:]
raven_dict = {}
for attr in attrs:
    raven_dict[attr] = []

pos_count = 0
for l in raven_log:
    vals = l.split(' ')[1:]
    for i in range(0, len(attrs)):
        raven_dict[attrs[i]].append(vals[i])
    pos_count += 1

output = ''
for i in range(0, pos_count):
    attr_base = 'rdev.mech[%d].' % raven_num
    pos_base = attr_base + 'pos.'
    ori_base = attr_base + 'ori.'
    x, y, z = int(raven_dict[pos_base+'x'][i]), \
            int(raven_dict[pos_base+'y'][i]), \
            int(raven_dict[pos_base+'z'][i])
    roll, pitch, yaw = int(raven_dict[ori_base+'roll'][i]), \
            int(raven_dict[ori_base+'pitch'][i]), \
            int(raven_dict[ori_base+'yaw'][i])
    output += '%f %f %f %f %f %f' % (-y/XYZ_SCALE, -x/XYZ_SCALE, -z/XYZ_SCALE, \
            -pitch/YPR_SCALE*180/pi, -roll/YPR_SCALE*180/pi, \
            -yaw/YPR_SCALE*180/pi) + '\n'
#    output += '%f %f %f %f %f %f' % (-y/XYZ_SCALE, -x/XYZ_SCALE, -z/XYZ_SCALE, \
#            roll/YPR_SCALE*180/pi, pitch/YPR_SCALE*180/pi, \
#            yaw/YPR_SCALE*180/pi) + '\n'

open(out_file, 'w').write(output)
