target extended-remote :3333
monitor reset halt
break main
layout src     # now this works
continue

