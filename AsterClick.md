# Introduction #

AsterClick is a multi-process event driven middleware
between Asterisk AMI and webSocket (HyBi) clients
such as desktop and mobile browsers.


# Details #

Unlike previous communications methods such as AJAX,REST,Commit etc. AsterClick
is event driven end to end with no server intensive polling.

This is extremely important for those aiming to deploy asterisk applications
in the cloud or as applets on mobile devices.

Even 100's or more AsterClick users together can use less bandwidth
then a single old school polling client!

These resource savings also extend to CPU and memory as when not processing
an actual event , AsterClick consumes next to nothing.

A polling client ( AJAX,REST,Commit etc.) rely on constantly hammering
a web server with http requests thus pounding on the network,CPU and memory resources.

Each polling request (several a minute) when packaged in typical
HTTP requests can waste hundreds or even thousands of bytes in overhead.

AsterClick on the other hand uses webSockets, where the message overhead
is at worst about 15 bytes and can be as little as 3 bytes!


