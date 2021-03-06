# Right now just a convenience script, may automate more later, use argparser,
# etc. Goes from images and log file to calib_cam.m. Load into MATLAB and then
# run the hand-eye calibration from there.

import sys
import os

IMG_EXT = 'tif|jpg|jpeg'
# TODO: Do command-line argument parsing
IMG_DIR = '/home/ziang/Desktop/20110615_handeye/captures/left'
LOG_FILE = 'raven_tools/in_me_07152011_173012.log'
POS_ORI_FILE = 'raven_tools/pos_ori.log'
CAM_CALIB_LOG = 'camera_calib/cam_calib.log'
CAM_CALIB_MAT = 'cam_calib.m'
RAVEN_NUM = 2

# Get image count and image indices that weren't thrown out
img_count = 0
img_inds = []
for f in os.listdir(IMG_DIR):
    f = f.lower()
    if f.split('.')[-1] in IMG_EXT:
        img_count += 1
        # Makes assumption images named #.ext
        img_inds.append(int(f.split('.')[0]))

# Generate position/orientation log
os.system('python raven_tools/raven_log_parser.py %s %s %d' % (LOG_FILE, \
        POS_ORI_FILE, RAVEN_NUM))

# Remove position/orientation data for images that were thrown out
line_num = 0
output = ''
for line in open(POS_ORI_FILE, 'r'):
    if line_num not in img_inds:
        line_num += 1
        continue
    output += line
    line_num += 1
open(POS_ORI_FILE, 'w').write(output)

# This assumes calibration has been run on images.
# Probably not a good idea to automate calibration--corner numbering issues
# should be manually checked.
os.system('camera_calib/parser -n %d -p %s < %s > camera_calib/%s' % (img_count, \
        POS_ORI_FILE, CAM_CALIB_LOG, CAM_CALIB_MAT))

# Copy outputted MATLAB file to hand-eye directory
os.system('cp camera_calib/%s handeye_calib/%s' % (CAM_CALIB_MAT, CAM_CALIB_MAT))
