#!/usr/bin/python

import os, sys, re, json, shutil
import subprocess
from subprocess import Popen, PIPE, STDOUT

# subprocess.run('source /Users/kord/Documents/source.nosync/emsdk/emsdk_env.sh', shell=True)
os.environ['EM_CONFIG'] = '/Users/kord/Documents/source.nosync/emsdk/.emscripten'

s = open(os.path.expanduser('/Users/kord/Documents/source.nosync/emsdk/.emscripten'), 'r').read()
exec(s)

sys.path.append(EMSCRIPTEN_ROOT)

emcc_args = [
  #'-m32',
  '-O3',
  '-D_ERROR_PRINT',
  '-DBROADWAY_DEBUG',
  # '-DDEBUG',
  # '-DTRACE',
  #'-Dxxx2yyy'
  '--memory-init-file', '1',
  '--llvm-opts', '3',
  '--llvm-lto', '3',
  '-s', 'NO_EXIT_RUNTIME=1',
  # '-s', 'NO_FILESYSTEM=1',
  # '-s', 'NO_BROWSER=1',
  #'-s', 'CORRECT_SIGNS=1',
  #'-s', 'CORRECT_OVERFLOWS=1',
  '-s', 'TOTAL_MEMORY=' + str(50*1024*1024),
  #'-s', 'FAST_MEMORY=' + str(50*1024*1024),
  #'-s', 'ALLOW_MEMORY_GROWTH=0',
  # '-s', 'INVOKE_RUN=0', #!disable this for main() to work
  #'-s', 'RELOOP=1',
  #'-s', 'INLINING_LIMIT=50',
  #'-s', 'OUTLINING_LIMIT=100',
  '-s', 'DOUBLE_MODE=0',
  # '-s', 'PRECISE_I64_MATH=0',
  #'-s', 'SIMD=1',
  '-s', 'AGGRESSIVE_VARIABLE_ELIMINATION=1',
  '-s', 'ALIASING_FUNCTION_POINTERS=1',
  '-s', 'DISABLE_EXCEPTION_CATCHING=1',
  #'-s', 'USE_CLOSURE_COMPILER=1',
  #'-s', 'FORCE_ALIGNED_MEMORY=1', #why doesnt this work?
  # '-s', '''EXPORTED_FUNCTIONS=["_broadwayGetMajorVersion", "_broadwayGetMinorVersion", "_broadwayInit", "_broadwayExit", "_broadwayCreateStream", "_broadwayPlayStream", "_broadwayOnHeadersDecoded", "_broadwayOnPictureDecoded"]''',
  #'--closure', '1',
  '--js-library', 'library.js',
  # '--preload-file', '/Users/kord/Documents/source.nosync/Broadway-H.264-decoder/Decoder/mozilla_story.mp4@mozilla_story.mp4',
  # '--preload-file', '/Users/kord/Documents/source.nosync/Broadway-H.264-decoder/Decoder/fox_baseline.h264@fox_baseline.h264',
  # '--preload-file', '/Users/kord/Documents/source.nosync/Broadway-H.264-decoder/Decoder/h264.h264@h264.h264',
  # '--preload-file', '/Users/kord/Documents/source.nosync/Broadway-H.264-decoder/Decoder/output.h264@output.h264',
  '--preload-file', '/Users/kord/Documents/source.nosync/Broadway-H.264-decoder/Decoder/test2.h264@test2.h264',
  '--pre-js', 'download.js',
  # '-s', 'USE_SDL=2',
  # '-s', 'USE_WEBGL2=1',
  # '-s', 'FULL_ES3=1',
  # '-s', 'MODULARIZE=1'
  '-s', 'USE_SDL=2',
  '-s', 'USE_WEBGL2=1'
]

JS_DIR = "js"
if not os.path.exists(JS_DIR):
  os.makedirs(JS_DIR)
  
OBJ_DIR = "obj"
if not os.path.exists(OBJ_DIR):
  os.makedirs(OBJ_DIR)

print('build')

source_files = [
  'h264bsd_transform.c',
  'h264bsd_util.c',
  'h264bsd_byte_stream.c',
  'h264bsd_seq_param_set.c',
  'h264bsd_pic_param_set.c',
  'h264bsd_slice_header.c',
  'h264bsd_slice_data.c',
  'h264bsd_macroblock_layer.c',
  'h264bsd_stream.c',
  'h264bsd_vlc.c',
  'h264bsd_cavlc.c',
  'h264bsd_nal_unit.c',
  'h264bsd_neighbour.c',
  'h264bsd_storage.c',
  'h264bsd_slice_group_map.c',
  'h264bsd_intra_prediction.c',
  'h264bsd_inter_prediction.c',
  'h264bsd_reconstruct.c',
  'h264bsd_dpb.c',
  'h264bsd_image.c',
  'h264bsd_deblocking.c',
  'h264bsd_conceal.c',
  'h264bsd_vui.c',
  'h264bsd_pic_order_cnt.c',
  'h264bsd_decoder.c',
  'H264SwDecApi.c',
  'extraFlags.c',
  'Decoder.c',
  'main.c',
  # 'Broadway.c'
  ]


for file in source_files:
  target = file.replace('.c', '.o')
  print('emcc %s -> %s' % (file, target))
  subprocess.run(['emcc'] + emcc_args + ['-Isrc', '-Iinc'] + ['-c', os.path.join('src', file), '-o', os.path.join('obj', target)])
  
object_files = [os.path.join('obj', x.replace('.c', '.o')) for x in source_files]

print('link -> %s' % 'avc.bc')
subprocess.run(['emcc', '-r'] + object_files + ['-o', 'avc.bc'])

# print('emcc %s -> %s' % ('avc.bc', os.path.join(JS_DIR, 'avc.js')))
# subprocess.run(['emcc', 'avc.bc', '-o', os.path.join(JS_DIR, 'avc.js')] + emcc_args)

# print('copying %s -> %s' % (os.path.join(JS_DIR, 'avc.js'), os.path.join('..','Player','avc-codec.js')))

print('emcc %s -> %s' % ('avc.bc', 'avc.js'))
subprocess.run(['emcc', 'avc.bc', '-o', 'avc.js'] + emcc_args)
# subprocess.run(['emcc', 'avc.bc', '-o', 'index.html'] + emcc_args)

print('copying %s -> %s' % ('avc.js', os.path.join('..','Player','avc-codec.js')))

f = open(os.path.join('..','Player','Decoder.js'), "w")
f1 = open(os.path.join("..", "templates", 'DecoderPre.js'))
f.write(f1.read())
f2 = open('avc.js')
# f2 = open(os.path.join(JS_DIR, 'avc.js'))
jscont = f2.read()
jscont = jscont.replace('require(', '(null)(')
jscont = jscont.replace('typeof require', 'typeof null')
f.write(jscont)
f3 = open(os.path.join("..", "templates", 'DecoderPost.js'))
f.write(f3.read())
asmfile = open(os.path.join('..','Player','avc.wasm'), "wb")
amsfilein = open('avc.wasm', 'rb')
# amsfilein = open(os.path.join(JS_DIR, 'avc.wasm'), 'rb')
asmfile.write(amsfilein.read())