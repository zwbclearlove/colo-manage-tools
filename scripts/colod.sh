#!/bin/bash
# 请将"process_name"替换为您要杀死的进程名
process_name="colod"
# 查找进程并杀死它们
pkill -f "$process_name"
sudo ../bin/colod
