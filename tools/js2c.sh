#!/usr/bin/env bash
proj_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )/../" >/dev/null 2>&1 && pwd )"
js_dir=$proj_dir/src/js
js_modules=''
for file in $(ls $js_dir); do
  filename="${file%.*}"
  if [ "$js_modules" == '' ]; then
    js_modules="$filename=$js_dir/$file"
  else
    js_modules="$js_modules,$filename=$js_dir/$file"
  fi
done
python $proj_dir/tools/js2c.py\
 --buildtype=debug\
 --modules=$js_modules
#  --snapshot-tool=$proj_dir/tools/jerry-snapshot
