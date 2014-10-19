BMPHIDE
=======

BMP steganography using LSB thechnique


USAGE
=======
usage: bmphide in_image method [data_to_hide]
         in_image: image required to hide/extract
         method: --hide,--extract
         data_to_hide: optional if --hide is setted

HIDE
=======
bmphide Lenna.bmp --hide "This is oculted data"
  
*************************  BMPHIDE  *************************
*************************************************************

********************* Initialising  *************************

------------------------ Summary ----------------------------

[!] Num pixels: 262144
[!] Heigth: 512
[!] Width: 512
[!] Length of hidded data: 28

-------------------------------------------------------------

------------------------ Results ----------------------------

[+] Data: This is oculted data hidded correctly



Extract
=======

bmphide Lenna.bmp --extract

*************************  BMPHIDE  *************************
*************************************************************

********************* Initialising  *************************

------------------------ Summary ----------------------------

[!] Num pixels: 262144
[!] Heigth: 512
[!] Width: 512
[!] Length of hidded data: 20

-------------------------------------------------------------

------------------------ Results ----------------------------

[+] Data extracted correctly: This is oculted data



Enjoy it.
