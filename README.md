# barcode2mysql
Take data from my Barcode Scan Tool (Trohestar NS-3309 N4), via USB to PS/2 interface, through a PS/2 Keyboard to serial module (Adafruit 1136), and manage a MySQL DB on my own server, over a WiFi network connection (Adafruit 3060). Geared for an Arduino MEGA2560 (CORE board, and dev board).


This is a quick and dirty utility i made, and yes i know the downfalls of using the String object. i didn't really care to do it properly with C strings, as i just needed a quick and dirty utility.

in short, i plan on having some barcodes to change modes, and set the active location. then the rest is handled by just scanning a barcode with the scanner, in any of the various modes it has. it can scan and send the barcodes instantly, through a RF link (Scan Mode), or collect a list of barcodes, and export them over the RF link in the order they were scanned in (Collection Mode), or collect a list of unique barcodes, with quantity values, to export as a list of barcodes, with their quantity (number of times each barcode was scanned), delimited by a TAB key (Inventory Mode).

The reason for this project, is that i have a large number of electronic components, and would like to be able to inventory them, and know what i have, and how many. i should add an inventory counting mode so i can make sure the database info is up to date. may just have that on the web interface though...
