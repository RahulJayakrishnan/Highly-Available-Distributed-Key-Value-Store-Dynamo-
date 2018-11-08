#!/bin/sh
ps -ef | grep Server | grep -v grep | awk '{print $2}' | xargs kill
