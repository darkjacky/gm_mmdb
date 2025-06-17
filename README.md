# gm_mmdb

This is made to read the MMDB file from https://db-ip.com/db/download/ip-to-country-lite

This would allow you to geofence your server from users in certain countries (whitelist and blacklist). Or use the IP to display a flag in the scoreboard.
Each query takes about 0.0000007 seconds on my computer so it shouldn't give any issues with performance.
I made this with the help of copilot. (As far as it was helpful which it really wasn't some times.)

To compile this yourself you will need to clone the libmaxminddb github with visual studio (2022) then create a 32 bit or 64 bit release and hit build all.
After that create a new console application called gm_mmdb and link maxminddb.lib and ws2_32.lib. Make sure to git clone the garrys mod c++ headers in the development branch and include that and the include folder of libmaxminddb.
This should be enough to compile it yourself.

Currently there is no Close function as I don't deem it a requirement.
