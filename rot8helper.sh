#!/usr/bin/env bash

monitor0='eDP-1'

while read rot;
do
  case $rot in
    0)
      hyprctl keyword monitor $monitor0,transform,0 >/dev/null
      ;;
    90)
      hyprctl keyword monitor $monitor0,transform,1 >/dev/null
      ;;
    -90)
      hyprctl keyword monitor $monitor0,transform,3 >/dev/null
      ;;
    180)
      hyprctl keyword monitor $monitor0,transform,2 >/dev/null
      ;;
    *)
      echo ERROR
      ;;
esac
done;
