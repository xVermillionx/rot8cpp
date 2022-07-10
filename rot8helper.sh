#!/usr/bin/env bash

wm="$(echo $XDG_CURRENT_DESKTOP | cut -f1 -d:)"
monitor0='eDP-1'

# Default
cmd="wlr-randr --output --transform "
rot8val=(normal 90 270 180)

case $wm in
  hyprland)
    cmd="hyprctl keyword monitor $monitor0,transform,"
    rot8val=(0 1 3 2)
    ;;
  river)
    ;;
  sway)
    cmd="swaymsg output * transform "
    # rot8val=(normal 90 270 180)
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
      ${cmd}${rot8val[0]} >/dev/null
      ;;
    90)
      ${cmd}${rot8val[1]} >/dev/null
      ;;
    -90)
      ${cmd}${rot8val[2]} >/dev/null
      ;;
    180)
      ${cmd}${rot8val[3]} >/dev/null
      ;;
    *)
      echo ERROR
      ;;
esac
done;
