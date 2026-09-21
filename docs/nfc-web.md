<p align="right"><a href="nfc-web.zh_CN.md">简体中文</a> · <strong>English</strong></p>

# NFC usage instructions (2.4.8)

The configuration page contains a static Android phone guide, with no URL
input, transfer QR generator, Web NFC write/read controls or NFC JavaScript.
The NTAG213 is independent of the MCU: saving badge profiles over USB or
the hotspot does not write NFC. One tag URL applies to all five profiles.

Enable phone NFC, open an NFC writing app such as NXP TagWriter, add a URL
record, enter the desired website and hold the phone against the badge until
writing completes. Writing replaces the previous tag content; leave the tag
writable for future changes. Exit the app, move the phone away, then tap
again and follow the phone's prompt to verify the destination.

The displayed URL is an example, not a readback or a claim of successful tag
programming. Internet destinations need phone connectivity; the hotspot
configuration URL requires joining the badge hotspot first. Changing NFC
content needs no firmware update. Version 2.4.8 updates the embedded hotspot
help page only; it does not program the NFC chip.
