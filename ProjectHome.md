AsterClick is an Event driven Asterisk/(XML)/JavaScript
interface for the Asterisk VOIP PBX server.

The HTML5 features required by AsterClick are supported
by at least current versions of Chrome,Fire Fox,Opera,Safari
and of course the AsterClick WBEA desktop deployment tool
that makes HTML5 applications into stand alone desktop
executables for at least Windows and Linux.

Unlike old school server interactions like (AJAX,Comit,etc.)
AsterClick uses WebSockets which completely does away with polling
and replaces it with totally event driven communications.

The latency is so low that if one pressed the [hold](hold.md) button on
a hard phone, the hold lamp on the phone and the indicator in a web
application appear to come on at the same instant.

Message overhead is also far lower. In HTTP polling requests (often several a minute)
there can be 100's or even 1000's of bytes in message overhead, even when there is no
data to process.

With Web Sockets , overhead is at worst about 15 bytes and can be as little as 3 bytes.

In between events , 100's of WebSocket clients together would not use as much bandwidth and
other resources as even a single polling client. That saves CPU,Memory ,Bandwidth and CASH!!!

To tell the truth , in between events , AsterClick uses ZERO BANDWIDTH, while other
solutions like AJAM continue to pound our servers and networks with pointless and cumbersome
HTTP traffic.

Actually an AsterClick server does not even require a web server at all.

Some of the coming features being considered are made possible only with web sockets
which can allow binary data to be vectored from within JavaScript which makes
for some interesting possibilities as relates to both audio and video.

One project some folks are working on involves "click to call" web clerks ,where shared browser
navigation, along with live telephone assistance make possible the type of customer
interactions previously only available in a brick and mortar store.

This Google code public repository is coming online in conjunction
with the release of AsterClick version ([R15](https://code.google.com/p/asterclick/source/detail?r=15)).

The code currently stored here is a pre-release upload
being used to sort out various automation tasks
for the AsterClick documentation site and other aspects.

This page will be updated with great fanfare at the official release point.