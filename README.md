This demo uses ECC608 to store a parent AES Key.
Both the host MCU and ECC608 also know a shared IO Protection Key.
Since encryption/decryption through the ECC608 results in data in the clear on the bus,
it is more secure to perform the encryption/decryption in the host MCU.
The key concept here is that the host MCU can seed the ECC608 KDF function to deterministically
generate an AES key.  This key is returned encrypted back to the host MCU and the host MCU can than
recover the AES key and begin to use it.  Any host MCU can recover the AES key as long as the ECC608
contains the parent symmetric key and the host MCU & ECC608 shared a common IO protection key. 
This is useful if you have multiple nodes on a network that need to derive a shared symmetric key.


Versions used
------------------------
MPLABX v6.20
SAMD21_DFP v3.6.144
XC32 v4.45

Load manifest file for MCC Harmony versions.