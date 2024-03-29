#!/usr/bin/env bash

wm="$(echo ${XDG_CURRENT_DESKTOP:-$TERM} | cut -f1 -d:)"
monitor0='eDP-1'

# Default
cmd="wlr-randr --output --transform "
cmd2=""
rot8val=(normal 90 270 180)

case $wm in
  hyprland|Hyprland)
    cmd="hyprctl keyword monitor $monitor0,transform,"
    # cmd2="hyprctl keyword input:touchdevice:transform "
    cmd2="hyprctl keyword device:wacom-hid-49c3-finger:transform "
    rot8val=(0 1 3 2)
    ;;
  river)
    ;;
  sway)
    cmd="swaymsg output * transform "
    # rot8val=(normal 90 270 180)
    ;;
  linux)
    sudo -v || exit 1
    cmd='sudo tee /sys/class/graphics/fbcon/rotate_all <<< '
    rot8val=(0 1 3 2)
    ;;
  *)
    cmd="wlr-randr --output --transform "
    # rot8val=(normal 90 270 180)
    ;;
esac

while read rot;
do
  case $rot in
    0)
      ${cmd}${rot8val[0]}
      # ${cmd}${rot8val[0]} >/dev/null
      ${cmd2:+${cmd2}${rot8val[0]}} >/dev/null
      ;;
    90)
      ${cmd}${rot8val[1]}
      # ${cmd}${rot8val[1]}
      ${cmd2:+${cmd2}${rot8val[1]}} >/dev/null
      ;;
    -90)
      ${cmd}${rot8val[2]}
      # ${cmd}${rot8val[2]}
      ${cmd2:+${cmd2}${rot8val[2]}} >/dev/null
      ;;
    180)
      ${cmd}${rot8val[3]}
      # ${cmd}${rot8val[3]} >/dev/null
      ${cmd2:+${cmd2}${rot8val[3]}} >/dev/null
      ;;
    *)
      echo ERROR
      ;;
esac
done;
